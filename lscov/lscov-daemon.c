/*
 * lscov - measurement daemon 
 * --------------------------
 * 
 * Collect logic states produced by an instrumented binary, and keep observed
 * logic states with a bloom filter. Periodically tally the elements.
 *
 * Supposed to start before (and run parallely with) a fuzzer.
 */

// TODO: param: tallying period (default: 10 mins).
// TODO: param: bloom filter size (default: 64MB).
// TODO: param: output path (default: lscov.out).
// TODO: param: error bound (default: 95%, disabled: 0%).

#include <sys/mman.h>
#include "stuff.h"

// TODO: limiting caching? other core?
u8* bloom_filter;

// TODO: Murmur3 hashing.

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
