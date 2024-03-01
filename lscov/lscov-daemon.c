/*
 * lscov - measurement daemon 
 * --------------------------
 * 
 * Collect logic states produced by an instrumented binary, and keep observed
 * logic states with a bloom filter. Periodically tally the elements.
 *
 * Supposed to start before (and run parallely with) a fuzzer.
 */

#define USE_COLOR     // Yes, please.

#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "stuff.h"
#include "emoji.h"

/* Parameters */

time_t      tallying_period = 10;      // Tallying period (in seconds) 
u32         bfilter_size = 0x4000000;  // Bloom filter size, in bytes
u8          num_hashes = 4;            // Number of hashes
const char* out_path = "lscov.csv";    // Output path
u8          error_percent = 0;         // Error bound (0: disabled)

/* State variables */

#define HCOUNT_CAP (2)

s32         shm_hcounts_1_id;     // (SHM) ID for 'hit_counts' (1)
s32         shm_hcounts_2_id;     // (SHM) ID for 'hit_counts' (2)
u8*         hit_counts_1;         // (SHM) Branch hit counts (1)
u8*         hit_counts_2;         // (SHM) Branch hit counts (2)
u8*         hit_counts;           // (SHM) Branch hit counts (current)
sem_t*      sema_rd;              // (sema) RT to daemon - "que update"
sem_t*      sema_dr;              // (sema) daemon to RT - "que execution"
struct timespec loop_timeout;     // 'sema_rd' semaphore timeout

u8*         bfilter;              // Bloom filter itself
u32         bfilter_size_bits;    // Bloom filter size, in bits
time_t      start_time;           // Measurement start time (in unix time)
time_t      next_tallying_time;   // Next tallying time (in unix time)


// FIXME: bfilter --> limiting caching? other core?

void out_init() {
  FILE *fout = fopen(out_path, "w");

  fprintf(fout, "Time,Coverage");
  if (error_percent > 0)
    fprintf(fout, ",(Lower),(Upper)\n");
  else
    fprintf(fout, "\n");

  fclose(fout);
}

void out_append(u32 time, u32 cov, u32 lower_err, u32 upper_err) {
  FILE *fout = fopen(out_path, "a");

  fprintf(fout, "%u,%u", time, cov);
  if (error_percent > 0)
    fprintf(fout, ",%u,%u\n", lower_err, upper_err);
  else
    fprintf(fout, "\n");

  fclose(fout);
}


static inline int hcounts_wait_until_ready() {
  return sem_timedwait(sema_rd, &loop_timeout);
}

void hcounts_bucket_to_lstate(u8* lstate) {
  /* The bucket numbers are one-hot-encoded. For example, Bucket 3 is encoded
   * as 0x04 (0b00000100). Hit counts 0 goes to Bucket 0. Anything else will go
   * to Bucket [log_2(hit_counts)] + 1, which is the previous power-of-2 integer
   * interpreted as a one-hot-encoded bit vector. */

  for (int i = 0; i < LSTATE_SIZE; i++) {
    /* Previous-power-of-2 implementation by Henry S. Warren Jr.
     * (https://stackoverflow.com/questions/2679815/previous-power-of-2)
     * In "Hacker's Delight." */

    u8 x = hit_counts[i];
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    lstate[i] = x - (x >> 1);
  }
}

void hcounts_nuke() {
  memset(hit_counts, 0, LSTATE_SIZE);
  if (hit_counts == hit_counts_1)
    hit_counts = hit_counts_2;
  else if (hit_counts == hit_counts_2) 
    hit_counts = hit_counts_1;
  else
    FATAL("bogus 'hit_counts'");
  sem_post(sema_dr);
}

void hcounts_stop() {
  if (shm_hcounts_1_id)
    shmctl(shm_hcounts_1_id, IPC_RMID, NULL);

  if (shm_hcounts_2_id)
    shmctl(shm_hcounts_2_id, IPC_RMID, NULL);

  if (sema_rd) {
    sem_close(sema_rd);
    sem_unlink(LSCOV_SEMA_RD_NAME);
  }
  
  if (sema_dr) {
    sem_close(sema_dr);
    sem_unlink(LSCOV_SEMA_DR_NAME);
  }
}

void hcounts_init() {
  atexit(hcounts_stop);

  /* Initialize SHM. */
  shm_hcounts_1_id = shmget(LSCOV_SHM_HCOUNT_1_KEY, LSTATE_SIZE, 
      IPC_CREAT | IPC_EXCL | 0600);
  if (shm_hcounts_1_id < 0) 
    PFATAL("shmget() for hit_counts failed");

  hit_counts_1 = (u8 *)shmat(shm_hcounts_1_id, NULL, 0);
  if (hit_counts_1 == (void *)-1) 
    PFATAL("shmat() for hit_counts failed");

  shm_hcounts_2_id = shmget(LSCOV_SHM_HCOUNT_2_KEY, LSTATE_SIZE, 
      IPC_CREAT | IPC_EXCL | 0600);
  if (shm_hcounts_2_id < 0) 
    PFATAL("shmget() for hit_counts failed");

  hit_counts_2 = (u8 *)shmat(shm_hcounts_2_id, NULL, 0);
  if (hit_counts_2 == (void *)-1) 
    PFATAL("shmat() for hit_counts failed");

  /* Initialize semaphores. */
  sema_rd = sem_open(LSCOV_SEMA_RD_NAME, O_CREAT, 0644, 0);
  if (sema_rd == (void *)-1) 
    PFATAL("sem_open() for sema_rd failed");

  sema_dr = sem_open(LSCOV_SEMA_DR_NAME, O_CREAT, 0644, HCOUNT_CAP);
  if (sema_dr == (void *)-1) 
    PFATAL("sem_open() for sema_dr failed");

  /* Initialize 'hit_counts'. */
  hit_counts = hit_counts_1;
  memset(hit_counts, 0, LSTATE_SIZE);
}


u32 bfilter_get_hash_index(const u8 *lstate, u32 seed) {
  /* MurmurHash implementation by Joseph Werle.
   * (https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c)
   * Copyright (c) 2014-2022 joseph werle <joseph.werle@gmail.com> */

  const u8 *key = lstate;
	const u32 len = LSTATE_SIZE;
  
  u32 c1 = 0xcc9e2d51;
  u32 c2 = 0x1b873593;
  u32 r1 = 15;
  u32 r2 = 13;
  u32 m = 5;
  u32 n = 0xe6546b64;
  u32 h = 0;
  u32 k = 0;
  u8 *d = (u8 *) key; // 32 bit extract from `key'
  const u32 *chunks = NULL;
  const u8 *tail = NULL; // tail - last 8 bytes
  int i = 0;
  int l = len / 4; // chunk length

  h = seed;

  chunks = (const u32 *) (d + l * 4); // body
  tail = (const u8 *) (d + l * 4); // last 8 byte chunk of `key'

  // for each 4 byte chunk of `key'
  for (i = -l; i != 0; ++i) {
    // next 4 byte chunk of `key'
    k = chunks[i];

    // encode next 4 byte chunk of `key'
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;

    // append to hash
    h ^= k;
    h = (h << r2) | (h >> (32 - r2));
    h = h * m + n;
  }

  k = 0;

  // remainder
  switch (len & 3) { // `len % 4'
    case 3: k ^= (tail[2] << 16);
    case 2: k ^= (tail[1] << 8);

    case 1:
      k ^= tail[0];
      k *= c1;
      k = (k << r1) | (k >> (32 - r1));
      k *= c2;
      h ^= k;
    case 0:;
  }

  h ^= len;

  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h % bfilter_size_bits;
}

void bfilter_set_1_by_index(u32 idx) {
  u32 byte_idx = idx >> 3;
  u8 bit_idx = idx & 0x07;

  if (byte_idx >= bfilter_size)
    FATAL("bogus 'byte_idx' for a bloom filter (byte_idx: %d, size: %u)",
        byte_idx, bfilter_size);

  bfilter[byte_idx] |= (1 << bit_idx);
}

u32 bfilter_get_num_1s() {
  /* Tally 1s in the filter. */
  u32 num_1s = 0;

  for (int i = 0; i < bfilter_size; i++) {
    u8 _byte = bfilter[i];
    while (_byte) {
      num_1s += (_byte & 0x01);
      _byte >>= 1;
    }
  }

  return num_1s;
}

u32 bfilter_calc_cardinality(u32 num_1s) {
  /* Estimate cardinality. */
  static int has_divisor = 0;
  static double divisor;

  if (!has_divisor) {
    divisor = num_hashes * log(1.0 - 1.0/bfilter_size_bits);
    has_divisor = 1;
  }

  // FIXME: replacing 'log' with a faster one?
  double dividend = log(1.0 - (double)num_1s/bfilter_size_bits);
  u32 cov = (u32)(dividend / divisor);

  return cov;
}

void bfilter_init() {
  bfilter_size_bits = (bfilter_size << 3);

  /* Allocate memory for a bloom filter. MAP_ANONYMOUS will automatically
   * zeroize the filter. */
  bfilter = mmap(0, bfilter_size, PROT_READ | PROT_WRITE, 
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (bfilter == MAP_FAILED)
    PFATAL("bloom filter allocation failed.");
}


static inline int tally_is_next_time() {
  return (next_tallying_time < time(NULL));
}

time_t tally_update_next_time() {
  /* As we want to avoid the timing error from accumulating as the measurement
   * goes on, we calculat the next tallying time based on the start time. This
   * allows the tallying times always separated by the period. */
  time_t prev_next_time = next_tallying_time;

  if (!next_tallying_time) 
    next_tallying_time = start_time;

  u32 prev_iter_num = (next_tallying_time - start_time) / tallying_period;
  next_tallying_time = start_time + (prev_iter_num + 1) * tallying_period;

  /* Wait at most until the next tallying time. */
  loop_timeout.tv_sec = next_tallying_time;

  return prev_next_time;
}

void tally_init() {
  start_time = time(NULL);
  tally_update_next_time();
}


#define PRINT_STAT
void lscov_init() {
#ifdef PRINT_STAT
  setlocale(LC_NUMERIC, "en_US.UTF-8");
#endif
}

void lscov_report(int use_cur_time) {
  if (!start_time)
    return;

#ifdef PRINT_STAT
  static u32 prev_cov = 0;
#endif

  time_t prev_next_time = tally_update_next_time();
  if (use_cur_time)
    prev_next_time = time(NULL);

#ifdef DEBUG_CARDINALITY_TIME
  /* Overhead measuring */
  struct timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
#endif

  u32 prev_time = prev_next_time - start_time;
  u32 num_1s = bfilter_get_num_1s();
  u32 cov = bfilter_calc_cardinality(num_1s); 
  // TODO: calculate error bounds.

#ifdef DEBUG_CARDINALITY_TIME
  /* Overhead measuring */
  clock_gettime(CLOCK_MONOTONIC, &t2);
  ACTF("Cardinality calculation time: %.5f", 
      ((double)t2.tv_sec + 1.0e-9 * t2.tv_nsec) - 
      ((double)t1.tv_sec + 1.0e-9 * t1.tv_nsec)); 
#endif

  out_append(prev_time, cov, 0, 0);
  OKF("Recorded new coverage. (time: %u, cov: %'u)", prev_time, cov);
  
#ifdef PRINT_STAT
  SAYF("    density: %3.2f%%, rate: (imm) %'u ls/sec, (all) %'u ls/sec\n",
    (float)num_1s / bfilter_size_bits * 100,
    (u32)((cov - prev_cov) / tallying_period), 
    (u32)(cov / prev_time)); 
  prev_cov = cov;
#endif
}

void lscov_wait() {
  /* Wait for the instrumented binary to report that it started.
   * We just busy-wait here because nobody is using computation resource in a
   * meaningful way at this point (and it's cheap for the instrumented binary
   * to do every execution) */
  while (*hit_counts != 0xff)
    continue;
}

/* Debug */
void sem_print_val(const char *when) {
  int _sema_rd_val;
  int _sema_dr_val;
  sem_getvalue(sema_rd, &_sema_rd_val);
  sem_getvalue(sema_dr, &_sema_dr_val);
  SAYF("[%s] rd:%d, dr:%d\n", when, _sema_rd_val, _sema_dr_val);
}

void lscov_loop() {
  while (1) {
    int sem_rd_ret = hcounts_wait_until_ready(); 

    /* Update the filter. */
    if (!sem_rd_ret) {
#ifdef DEBUG_RECEIVE_HCOUNT
      ACTF("Received new hit counts.");
#endif

      /* Bucketize the hit counts, making a logic state. */
      static u8* lstate;
      if (!lstate)
        lstate = mmap(0, LSTATE_SIZE, PROT_READ | PROT_WRITE, 
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

      hcounts_bucket_to_lstate(lstate);
      hcounts_nuke();

      /* Set the hash indices of the logic state to 1 in the filter. */
      for (int h = 0; h < num_hashes; h++) {
        u32 hidx = bfilter_get_hash_index(lstate, h);
        bfilter_set_1_by_index(hidx);
      } 
    }

    /* Report the coverage. */
    if (tally_is_next_time()) 
      lscov_report(0);
  }
}

void lscov_stop(int sig) {
  ACTF("Terminating lscov...");
  lscov_report(1);
  OKF("Good luck! %s", random_emoji());

  exit(0);
}


void sig_init() {
  /* Install cleanup handler. */
  struct sigaction sa;
  sa.sa_handler = lscov_stop;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}


int main(int argc, char** argv) {
  SAYF(cCYA "lscov-daemon v" VERSION cRST " by Gwangmu Lee <iss300@gmail.com>\n");
  
  /* Initialization */
  ACTF("Initializating...");
  lscov_init();
  sig_init();
  out_init();
  hcounts_init();
  bfilter_init();

  /* Wait until when a fuzzer starts. */
  ACTF("Waiting for a fuzzer...");
  lscov_wait();
  OKF("Fuzzer started.");
  
  /* Looping... */
  ACTF("Recording... (out: %s)", out_path);
  tally_init();
  lscov_loop();
}
