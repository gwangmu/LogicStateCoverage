/*
 * lscov - measurement daemon 
 * --------------------------
 * 
 * Collect logic states produced by an instrumented binary, and keep observed
 * logic states with a bloom filter. Periodically tally the elements.
 *
 * Supposed to start before (and run parallely with) a fuzzer.
 */

#include <math.h>
#include <sys/mman.h>
#include "stuff.h"

// TODO: param: tallying period (default: 10 mins).
// TODO: param: bloom filter size (default: 64MB).
// TODO: param: output path (default: lscov.out).
// TODO: param: error bound (default: 95%, disabled: 0%).

u32   tallying_period = 600;          // Tallying period (in seconds) 
u32   bloom_filter_size = 0x4000000;  // Bloom filter size (in bytes)
u8    num_hashes = 4;                 // Number of hashes
u8*   out_path = "lscov.out";         // Output path
u8    error_percent = 95;             // Error bound (in percent)

// TODO: limiting caching? other core?
u8*         hit_counts;           // (SHM) Branch hit counts
recstat_t*  rec_stat;             // (SHM) Branch hit count recording status
u8*         bloom_filter;         // Bloom filter itself
u32         start_time;           // Measurement start time (in unix time)
u32         next_tallying_time;   // Next tallying time (in unix time)

// TODO: SIGINT handler; finalize daemon.

inline int is_recording_finished() {
  return (*rec_stat == RECSTAT_FIN);
}

u8 flp2(u8 x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

void bucket_hit_counts(u8* lstate) {
  /* The bucket numbers are one-hot-encoded. In other words, Bucket 3 is encoded
   * as 0x04 (0b00000100). Hit count 0 goes to Bucket 0. Anything else will go
   * to Bucket [log_2(hit_count)] + 1, which is the previous power-of-2 integer
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

    /* Caveat: we don't need to nuke 'lstate' because it will be overwritten. */
  }
}

void nuke_hit_counts() {
  memset(hit_counts, 0, LSTATE_SIZE);
  *rec_stat = RECSTAT_RDY;
}

u32 get_hash_index(const u8 *key, u32 seed) {
  /* MurmurHash implementation by Joseph Werle.
   * (https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c)
   * Copyright (c) 2014-2022 joseph werle <joseph.werle@gmail.com> */

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
  }

  h ^= len;

  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h % len;
}

int set_filter_bit_by_index(u32 idx) {
  u32 byte_idx = idx >> 3;
  u8 bit_idx = idx & 0x07;

  if (byte_idx >= bloom_filter_size)
    FATAL("bogus 'byte_idx' for a bloom filter (byte_idx: %lu, size: %lu)",
        byte_idx, bloom_filter_size);

  bloom_filter[byte_idx] |= (1 << bit_idx);
}

u32 tally_1s_in_filter() {
  u32 count = 0;

  for (int i = 0; i < bloom_filter_size; i++) {
    u8 _byte = bloom_filter[i];
    while (_byte) {
      count += (_byte & 0x01);
      _byte >>= 1;
    }
  }

  return count;
}

// TODO: replacing 'log' with a faster one?
u32 calculate_cardinality(u32 num_1s) {
  static int has_divisor = 0;
  static double divisor;

  if (!has_divisor) {
    divisor = num_hashes * log(1.0 - 1.0/(bloom_filter_size << 3));
    has_divisor = 1;
  }

  double dividend = log(1.0 - (double)num_1s/(bloom_filter_size << 3));
  return (u32)(dividend / divisor);
}

void update_tallying_time() {
  /* As we want to avoid the timing error from accumulating as the measurement
   * goes on, we calculat the next tallying time based on the start time. This
   * allows the tallying times always separated by the period. */

  if (!next_tallying_time) 
    next_tallying_time = start_time;

  u32 prev_iter_num = (next_tallying_time - start_time) / tallying_period;
  next_tallying_time = start_time + (prev_iter_num + 1) * tallying_period;
}

int main(int argc, char** argv) {
  // TODO: create SHM.
  // TODO: initialize SHM (status, zeroize).

  // TODO: create a bloom filter.
  // TODO: initialize the bloom filter (zeroize).

  // TODO: set LSREC_RDY.
  // TODO: call "loop". 
}

void loop() {
  // TODO: set the next tallying time.
  // TODO: wait for LSREC_FIN (or tallying time).

  // TODO: if LSREC_FIN
  // TODO:   bucketize the hit counts.
  // TODO:   zeroize the logic state.
  // TODO:   set LSREC_RDY.
  
  // TODO:   hash the logic state.
  // TODO:   update the bloom filter.

  // TODO: if tallying time
  // TODO:   set the next tallying time (+start).
  // TODO:   tally 1s in the bloom filter.
  // TODO:   calculate the cardinality.
  // TODO:   calculate the error bound.
  // TODO:   append new lscov+err to the output.
}
