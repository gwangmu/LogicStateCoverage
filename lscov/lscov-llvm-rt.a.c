/*
 * lscov - instrumentation runtime 
 * -------------------------------
 * 
 * Runtime components responsible for setup and status update.
 * Referred to AFL. (https://github.com/google/AFL)
 * See "llvm_mode/afl-llvm-rt.o.c" for the reference implementation.
 */

#include <assert.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "stuff.h"

/* Constructor/destructor priority. Using some arbitrarily low priority. */

#define CONST_PRIO 255 

/* Globals for instrumentation */

u8           __lscov_area_initial[LSTATE_SIZE];
u8*          __lscov_area_ptr = __lscov_area_initial;
__thread u32 __lscov_prev_loc;

sem_t*       __lscov_sema_rd;
sem_t*       __lscov_sema_dr;


void __lscov_end_exec() {
  /* Mark that it notified this termination to the daemon. */
  *__lscov_area_ptr = 0x80;

  int _sema_rd_val;
  sem_getvalue(__lscov_sema_rd, &_sema_rd_val);
  if (_sema_rd_val) {
    SAYF("_sema_rd_val: %d\n", _sema_rd_val);
    assert(0 && "non-zero _sema_rd_val");
  }

  /* Mark the end of recording. */
  sem_post(__lscov_sema_rd);
}

void __lscov_start_exec() {
  /* Mark this hit count as unnotified yet to the daemon (0x01). */
  *__lscov_area_ptr = 0x81;

  /* Wait until SHM is ready */
  sem_wait(__lscov_sema_dr);
}

/* Finalization (per execution) */

__attribute__((destructor(CONST_PRIO))) 
void __lscov_fin(void) {
  if (__lscov_sema_rd) 
    __lscov_end_exec();
}

/* Initialization (upon starting) */

__attribute__((constructor(CONST_PRIO))) 
void __lscov_init(void) {
  s32 shm_hcount_id = shmget(LSCOV_SHM_HCOUNT_KEY, LSTATE_SIZE, 0600);

  if (shm_hcount_id >= 0) {
    __lscov_area_ptr = (u8 *)shmat(shm_hcount_id, NULL, 0);
    if (__lscov_area_ptr == (void *)-1) 
      PFATAL("shmat() for hit_count failed");

    /* Initialize semaphores. */
    __lscov_sema_rd = sem_open(LSCOV_SEMA_RD_NAME, 0, 0644, 0);
    if (__lscov_sema_rd == (void *)-1) 
      PFATAL("sem_open() for sema_rd failed");

    __lscov_sema_dr = sem_open(LSCOV_SEMA_DR_NAME, 0, 0644, 0);
    if (__lscov_sema_dr == (void *)-1) 
      PFATAL("sem_open() for sema_dr failed");

    /* Signal the daemon to start an execution. For convenience's sake, just use
     * one unlikely bit at the beginning. All logic states will have this bit,
     * so it has zero implication for the coverage. */
    *__lscov_area_ptr = 0x80;
  }
}

/* Initialization (every execution)
 *
 * Some fuzzers (well, most of them) insert their initializer as a constuctor.
 * What's worse is that they put their initializer at the least priority,
 * so the forkserver happens before any other initializers. '__lscov_main'
 * CANNOT be one of them because it should wait a semaphore ('__lscov_sema_dr')
 * every execution. Just insert a call to '__lscov_main' at the beginning of
 * 'main' and that'll defeat all initializers. M-hwa-hwa-hwa. */

void __lscov_main(void) {
  /* Similar to AFL, if we're running with logic state coverage measurement,
   * attach to the appropriate region. */

  if (__lscov_sema_rd) {
    /* If the destructor was not called in the last execution (e.g., due to a
     * crash), mark it finished and let the daemon do its job. This hack will
     * make the daemon ignore the very last execution if it was a crash, but
     * it's just only *one* execution. */

    if (*__lscov_area_ptr > 0x80) 
      __lscov_end_exec();

    /* Sanity check: should be a lock-step. */
    int _sema_dr_val;
    sem_getvalue(__lscov_sema_dr, &_sema_dr_val);
    if (_sema_dr_val > 1)
      assert(0 && "_sema_dr_val > 1");

    __lscov_start_exec();

    /* Sanity check: should have a clear '__lscov_area_ptr'. */
    u8 _test_hc = 0;
    for (int i = 1; i < (LSTATE_SIZE >> 6); i++)
      _test_hc |= __lscov_area_ptr[i << 6];
    if (_test_hc) 
      assert(0 && "tainted hit counts");
  }
}
