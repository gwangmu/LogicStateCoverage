/*
 * lscov - instrumentation runtime 
 * -------------------------------
 * 
 * Runtime components responsible for setup and status update.
 * Referred to AFL. (https://github.com/google/AFL)
 * See "llvm_mode/afl-llvm-rt.o.c" for the reference implementation.
 */

#include <sys/types.h>
#include <sys/shm.h>
#include "stuff.h"

/* Constructor/destructor priority. Using some arbitrarily low priority. */

#define CONST_PRIO 255 

/* Globals for instrumentation */

u8             __lscov_area_initial[LSTATE_SIZE + sizeof(lsrec_stat_t)];
u8*            __lscov_area_ptr = __lscov_area_initial + sizeof(lsrec_stat_t);
lsrec_stat_t*  __lscov_recstat_ptr = __lscov_area_initial; 

__thread u32   __lscov_prev_loc;


/* Initialization (per execution) */

__attribute__((constructor(CONST_PRIO))) 
void __lscov_init(void) {
  char *id_str = getenv(SHM_ENV_VAR);

  /* Same as AFL. If we're running with logic state coverage measurement, attach
     to the appropriate region. SHM_ENV_VAR should be set in the measurement
     daemon, of course. */

  if (id_str) {
    u32 shm_id = atoi(id_str);
    __lscov_area_ptr = shmat(shm_id, NULL, 0);

    if (__lscov_area_ptr == (void *)-1) {
      WARNF("Shared memory not attached. Did you start the daemon?");
      exit(1);
    }

    /* Adjust pointers */
    __lscov_recstat_ptr = __lscov_area_ptr;
    __lscov_area_ptr += sizeof(lsrec_stat_t);

    /* If the destructor was not called in the last execution (e.g., due to a
       crash), mark it finished and let the daemon do its job. This hack will
       make the daemon ignore the very last execution if it was a crash, but
       it's just only *one* execution. */
    if (*__lscov_recstat_ptr == LSREC_REC)
      *__lscov_recstat_ptr = LSREC_FIN;

    /* Wait until SHM is ready */
    while (*__lscov_recstat_ptr != LSREC_RDY);
  }

  /* Start recording */
  *__lscov_recstat_ptr = LSREC_REC;
}

/* Finalization (per execution) */

__attribute__((destructor(CONST_PRIO))) 
void __lscov_fin(void) {
  /* Mark the end of recording, if there was no crash. */
  *__lscov_recstat_ptr = LSREC_FIN;
}
