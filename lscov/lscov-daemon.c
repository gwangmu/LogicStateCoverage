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

#include <assert.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "stuff.h"
#include "emoji.h"

/* Parameters */

time_t      tallying_period = 10;      // Tallying period (in seconds) 
u32         bfilter_size = 0x4000000;  // Bloom filter size, in bytes
u8          num_hashes = 4;            // Number of hashes
const char* out_path = "lscov.csv";    // Output path
u8          error_percent = 0;         // Error bound (0: disabled)

/* State variables */

s32         shm_hcount_id;        // (SHM) ID for 'hit_count'
u8*         hit_counts;           // (SHM) Branch hit counts
sem_t*      sema_rd;              // (sema) RT to daemon - "que update"
sem_t*      sema_dr;              // (sema) daemon to RT - "que execution"
struct timespec loop_timeout;     // 'sema_rd' semaphore timeout

u8*         bfilter;              // Bloom filter itself
u32         bfilter_size_bits;    // Bloom filter size, in bits
time_t      start_time;           // Measurement start time (in unix time)
time_t      next_tallying_time;   // Next tallying time (in unix time)

u32         exec_count;
u32         exec_count_in_period;
u8          stop_soon;


void out_init() {
  FILE *fout = fopen(out_path, "w");

  fprintf(fout, "Time,Coverage");
  if (error_percent > 0)
    fprintf(fout, ",(Lower),(Upper)");
  fprintf(fout, ",Density,RateS(ins),RateE(per),RateS(avg),RateE(avg)\n");

  fclose(fout);
}

void out_append(u32 time, u32 cov, u32 lower_err, u32 upper_err, float density,
    u32 rate_ins, float rate_per, u32 rate_avg, float rate_per_avg) {
  FILE *fout = fopen(out_path, "a");

  fprintf(fout, "%u,%u", time, cov);
  if (error_percent > 0)
    fprintf(fout, ",%u,%u", lower_err, upper_err);
  fprintf(fout, ",%u,%u,%3.2f,%u,%3.2f,%u,%3.2f\n", time, cov, density,
      rate_ins, rate_per, rate_avg, rate_per_avg);

  fclose(fout);
}


static inline int hcount_wait_until_ready() {
  int _sema_rd_val;
  sem_getvalue(sema_rd, &_sema_rd_val);
  if (_sema_rd_val > 1) {
    assert(0 && "_sema_rd_val > 1");
    FATAL("_sema_rd_val: %d", _sema_rd_val);
  }
  //return sem_timedwait(sema_rd, &loop_timeout);
  return sem_wait(sema_rd);
}

/* Bucketing excerpted from AFL. It was much faster than my implementation,
 * unsurprisingly */

#ifdef LSCOV_BUCKET
#ifdef BUCKET_1
static const u8 count_bucket_lookup8[256] = {
  [0]           = 0,
  [1 ... 255]   = 1,
};
#elif defined BUCKET_LOG2_LOG3p2
static const u8 count_bucket_lookup8[256] = {
  [0]           = 0,    // No hit
  [1 ... 8]     = 1,    // Hit
  [9 ... 255]   = 2,    // Repetition
};
#elif defined BUCKET_LOG2_LOG4p1_p1
static const u8 count_bucket_lookup8[256] = {
  [0]           = 0,    // No hit
  [1 ... 3]     = 1,    // Hit
  [4 ... 63]    = 2,    // Revisit
  [64 ... 255]  = 4,    // Repetition
};
#else // BUCKET_LOG2
static const u8 count_bucket_lookup8[256] = {
  [0]           = 0,    // No hit
  [1]           = 1,    // Hit
  [2 ... 3]     = 2,    
  [4 ... 7]     = 4,    
  [8 ... 15]    = 8,    
  [16 ... 31]   = 16,    
  [32 ... 63]   = 32,    
  [64 ... 127]  = 64,   // Repetition
  [128 ... 255] = 128,
};
#endif

static u16 count_bucket_lookup16[65536];
#endif

static inline void hcount_bucket_to_lstate(u8* lstate) {
#ifdef LSCOV_BUCKET
  u32 i = LSTATE_SIZE >> 3;
  u64 *mem = (u64 *)hit_counts;
  u64 *dest = (u64 *)lstate;

  while (i--) {
    /* Optimize for sparse bitmaps. */

    if (unlikely(*mem)) {
      u16* mem16 = (u16*)mem;
      u16* dest16 = (u16*)dest;
      dest16[0] = count_bucket_lookup16[mem16[0]];
      dest16[1] = count_bucket_lookup16[mem16[1]];
      dest16[2] = count_bucket_lookup16[mem16[2]];
      dest16[3] = count_bucket_lookup16[mem16[3]];
    } else {
      *dest = 0;
    }

    mem++;
    dest++;
  }
#else
  /* It feels wasteful as 7/8 of the bits are 0, but it may be better than
   * compacting bits as it requires two additional operations (i.e., load and
   * shift) during execution. I need to check which is better soon tho. */ 

  memcpy(lstate, hit_counts, LSTATE_SIZE);
#endif
}

void hcount_mark_read() {
  int _sema_dr_val;
  sem_getvalue(sema_dr, &_sema_dr_val);
  if (!_sema_dr_val) 
    sem_post(sema_dr);
}

void hcount_stop() {
  if (shm_hcount_id)
    shmctl(shm_hcount_id, IPC_RMID, NULL);

  if (sema_rd) {
    sem_close(sema_rd);
    sem_unlink(LSCOV_SEMA_RD_NAME);
  }
  
  if (sema_dr) {
    sem_close(sema_dr);
    sem_unlink(LSCOV_SEMA_DR_NAME);
  }
}

void hcount_init() {
  atexit(hcount_stop);

  /* Initialize SHM. */
  shm_hcount_id = shmget(LSCOV_SHM_HCOUNT_KEY, LSTATE_SIZE, 
      IPC_CREAT | IPC_EXCL | 0600);
  if (shm_hcount_id < 0) 
    PFATAL("shmget() for hit_count failed");

  hit_counts = (u8 *)shmat(shm_hcount_id, NULL, 0);
  if (hit_counts == (void *)-1) 
    PFATAL("shmat() for hit_count failed");

  /* Initialize semaphores. */
  sem_unlink(LSCOV_SEMA_RD_NAME);
  sema_rd = sem_open(LSCOV_SEMA_RD_NAME, O_CREAT | O_EXCL, 0644, 0);
  if (sema_rd == (void *)-1) 
    PFATAL("sem_open() for sema_rd failed");

  sem_unlink(LSCOV_SEMA_DR_NAME);
  sema_dr = sem_open(LSCOV_SEMA_DR_NAME, O_CREAT | O_EXCL, 0644, 1);
  if (sema_dr == (void *)-1) 
    PFATAL("sem_open() for sema_dr failed");

  /* Initialize 'hit_counts'. */
  memset(hit_counts, 0, LSTATE_SIZE);

#ifdef LSCOV_BUCKET
  /* Initialize 'count_bucket_lookup16' */
  for (u32 b1 = 0; b1 < 256; b1++) 
    for (u32 b2 = 0; b2 < 256; b2++)
      count_bucket_lookup16[(b1 << 8) + b2] = 
        (count_bucket_lookup8[b1] << 8) |
        count_bucket_lookup8[b2];
#endif
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
  // FIXME: bfilter --> limiting caching? other core?

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

  u64 *bfilter64 = (u64 *)bfilter;
  u32 rem_size = bfilter_size >> 3;
  while (rem_size--) {
    num_1s += __builtin_popcountll(*bfilter64);
    bfilter64++;
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


void lscov_init() {
  setlocale(LC_NUMERIC, "en_US.UTF-8");
}

void* lscov_report(void * _unused) {
  static u32 prev_cov = 0;

  if (!start_time)
    return NULL;

  time_t prev_next_time = tally_update_next_time();
  if (stop_soon)
    prev_next_time = time(NULL);

  u32 prev_time = prev_next_time - start_time;
  u32 num_1s = bfilter_get_num_1s();
  u32 cov = bfilter_calc_cardinality(num_1s); 
  // TODO: calculate error bounds.
  
  float density = (float)num_1s / bfilter_size_bits * 100;
  u32 rate_ins = (u32)((cov - prev_cov) / tallying_period);
  float rate_per = !exec_count_in_period ? 
    (float)(cov - prev_cov) / exec_count_in_period * 100 : 0;
  u32 rate_avg = (u32)(cov / prev_time); 
  float rate_per_avg = !exec_count ? 
    (float)cov / exec_count * 100 : 0;

#ifdef PRINT_STAT
  SAYF("    density: %3.2f%%, rate: (ins) %'u ls/sec [%3.2f%%], (avg) %'u ls/sec [%3.2f%%]\n",
      density, rate_ins, rate_per, rate_avg, rate_per_avg);
#endif

  out_append(prev_time, cov, 0, 0, density, rate_ins, rate_per, rate_avg, rate_per_avg);
  OKF("Recorded new coverage. (time: %u, cov: %'u)", prev_time, cov);
      
  exec_count_in_period = 0;
  prev_cov = cov;

  return NULL;
}

void lscov_wait() {
  /* Wait for the instrumented binary to report that it started.
   * We just busy-wait here because nobody is using computation resource in a
   * meaningful way at this point (and it's cheap for the instrumented binary
   * to do every execution) */
  while (*hit_counts != 0x80)
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
/* Debug */

void lscov_loop() {
  while (1) {
    int sem_rd_ret = hcount_wait_until_ready(); 

    /* Update the filter. */
    if (!sem_rd_ret) {
      exec_count++;
      exec_count_in_period++;

      /* Bucketize the hit counts, making a logic state. */
      static u8* lstate;
      if (!lstate)
        lstate = mmap(0, LSTATE_SIZE, PROT_READ | PROT_WRITE, 
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

      hcount_bucket_to_lstate(lstate);
      hcount_mark_read();

      /* Set the hash indices of the logic state to 1 in the filter. */
      for (int h = 0; h < num_hashes; h++) {
        u32 hidx = bfilter_get_hash_index(lstate, h);
        bfilter_set_1_by_index(hidx);
      } 
    }

    /* Report the coverage. */
    if (tally_is_next_time()) {
      /* Tallying the bloom filter while (potentially) updating it may seem
       * inaccurate, but I believe the cardinality calculation only yields a
       * cardinality between the "previous" and "next" true value. So we're
       * actually not losing anything by doing this. */

      pthread_t _pt_dummy;
      pthread_create(&_pt_dummy, NULL, &lscov_report, NULL);
    }
  }
}

void lscov_stop(int sig) {
  ACTF("Terminating lscov...");
  stop_soon = 1;
  lscov_report(NULL);
  OKF("Good luck! %s", random_emoji());

  exit(0);
}


void arg_parse(int argc, char** argv) {
  /* GNU getopt() example:
   * https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html */

  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "o:")) != -1) {
    switch (c) {
    case 'o':
      out_path = optarg;
      ACTF("Output path: %s", out_path);
      break;
    case '?':
      WARNF("Ignoring -%c...", optopt);
      break;
    default:
      abort();
    }
  }

  /* In case lscov requires non-option arguments in the future... */
  // for (index = optind; index < argc; index++)
  //   printf ("Non-option argument %s\n", argv[index]);

  return;
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

  /* Argument parsing */
  arg_parse(argc, argv);
  
  /* Initialization */
  ACTF("Initializating...");
  lscov_init();
  sig_init();
  out_init();
  hcount_init();
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
