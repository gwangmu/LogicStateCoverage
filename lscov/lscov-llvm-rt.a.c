/*
 * lscov - instrumentation runtime 
 * -------------------------------
 * 
 * Runtime components responsible for setup and status update.
 * Referred to AFL. (https://github.com/google/AFL)
 * See "llvm_mode/afl-llvm-rt.o.c" for the reference implementation.
 */

#include <semaphore.h>
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


/* Initialization (per execution) */

__attribute__((constructor(CONST_PRIO))) 
void __lscov_init(void) {
  /* Same as AFL. If we're running with logic state coverage measurement, attach
     to the appropriate region. SHM_ENV_VAR should be set in the measurement
     daemon, of course. */

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

    /* If the destructor was not called in the last execution (e.g., due to a
       crash), mark it finished and let the daemon do its job. This hack will
       make the daemon ignore the very last execution if it was a crash, but
       it's just only *one* execution. */
    int _sema_rd_val, _sema_dr_val;
    sem_getvalue(__lscov_sema_rd, &_sema_rd_val);
    sem_getvalue(__lscov_sema_dr, &_sema_dr_val);

    if (!_sema_dr_val && !_sema_rd_val)
      sem_post(__lscov_sema_rd);

    /* Wait until SHM is ready */
    sem_wait(__lscov_sema_dr);
  }
}

/* Finalization (per execution) */

__attribute__((destructor(CONST_PRIO))) 
void __lscov_fin(void) {
  if (__lscov_sema_rd) {
    /* Mark the end of recording, if there was no crash. */
    sem_post(__lscov_sema_rd);
  }
}
