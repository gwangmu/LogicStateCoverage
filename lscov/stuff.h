/*
 * lscov - good stuff 
 * ------------------
 *
 * Constants, debug macros, and stuff.
 * Debug macros blatantly stolen from AFL. (https://github.com/google/AFL)
 * See "debug.h" in AFL for the original implementation.
 */

/* We're living in 2024 and still recommended to implement header guards
   with 'ifndef...endif'. I'll just use 'pragma once' because I have little
   regard to the de-jure standard (and it's no big code base anyway). */
#pragma once

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Version string */

#define VERSION "0.01"

/* Standard integer types */

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

/* Memory barrier */

#define MEM_BARRIER() \
  __asm__ volatile("" ::: "memory")

/* Environment variable used to pass SHM ID to the called program. */

#define SHM_ENV_VAR "__LSCOV_SHM_ID"

/* Logic state size: simply following MAP_SIZE in AFL. */

#define LSTATE_SIZE_POW2 16
#define LSTATE_SIZE      (1 << LSTATE_SIZE_POW2)

/*******************
 * Terminal colors *
 *******************/

#ifdef USE_COLOR

#  define cBLK "\x1b[0;30m"
#  define cRED "\x1b[0;31m"
#  define cGRN "\x1b[0;32m"
#  define cBRN "\x1b[0;33m"
#  define cBLU "\x1b[0;34m"
#  define cMGN "\x1b[0;35m"
#  define cCYA "\x1b[0;36m"
#  define cLGR "\x1b[0;37m"
#  define cGRA "\x1b[1;90m"
#  define cLRD "\x1b[1;91m"
#  define cLGN "\x1b[1;92m"
#  define cYEL "\x1b[1;93m"
#  define cLBL "\x1b[1;94m"
#  define cPIN "\x1b[1;95m"
#  define cLCY "\x1b[1;96m"
#  define cBRI "\x1b[1;97m"
#  define cRST "\x1b[0m"

#  define bgBLK "\x1b[40m"
#  define bgRED "\x1b[41m"
#  define bgGRN "\x1b[42m"
#  define bgBRN "\x1b[43m"
#  define bgBLU "\x1b[44m"
#  define bgMGN "\x1b[45m"
#  define bgCYA "\x1b[46m"
#  define bgLGR "\x1b[47m"
#  define bgGRA "\x1b[100m"
#  define bgLRD "\x1b[101m"
#  define bgLGN "\x1b[102m"
#  define bgYEL "\x1b[103m"
#  define bgLBL "\x1b[104m"
#  define bgPIN "\x1b[105m"
#  define bgLCY "\x1b[106m"
#  define bgBRI "\x1b[107m"

#else

#  define cBLK ""
#  define cRED ""
#  define cGRN ""
#  define cBRN ""
#  define cBLU ""
#  define cMGN ""
#  define cCYA ""
#  define cLGR ""
#  define cGRA ""
#  define cLRD ""
#  define cLGN ""
#  define cYEL ""
#  define cLBL ""
#  define cPIN ""
#  define cLCY ""
#  define cBRI ""
#  define cRST ""

#  define bgBLK ""
#  define bgRED ""
#  define bgGRN ""
#  define bgBRN ""
#  define bgBLU ""
#  define bgMGN ""
#  define bgCYA ""
#  define bgLGR ""
#  define bgGRA ""
#  define bgLRD ""
#  define bgLGN ""
#  define bgYEL ""
#  define bgLBL ""
#  define bgPIN ""
#  define bgLCY ""
#  define bgBRI ""

#endif /* ^USE_COLOR */

/*************************
 * Box drawing sequences *
 *************************/

#ifdef FANCY_BOXES

#  define SET_G1   "\x1b)0"       /* Set G1 for box drawing    */
#  define RESET_G1 "\x1b)B"       /* Reset G1 to ASCII         */
#  define bSTART   "\x0e"         /* Enter G1 drawing mode     */
#  define bSTOP    "\x0f"         /* Leave G1 drawing mode     */
#  define bH       "q"            /* Horizontal line           */
#  define bV       "x"            /* Vertical line             */
#  define bLT      "l"            /* Left top corner           */
#  define bRT      "k"            /* Right top corner          */
#  define bLB      "m"            /* Left bottom corner        */
#  define bRB      "j"            /* Right bottom corner       */
#  define bX       "n"            /* Cross                     */
#  define bVR      "t"            /* Vertical, branch right    */
#  define bVL      "u"            /* Vertical, branch left     */
#  define bHT      "v"            /* Horizontal, branch top    */
#  define bHB      "w"            /* Horizontal, branch bottom */

#else

#  define SET_G1   ""
#  define RESET_G1 ""
#  define bSTART   ""
#  define bSTOP    ""
#  define bH       "-"
#  define bV       "|"
#  define bLT      "+"
#  define bRT      "+"
#  define bLB      "+"
#  define bRB      "+"
#  define bX       "+"
#  define bVR      "+"
#  define bVL      "+"
#  define bHT      "+"
#  define bHB      "+"

#endif /* ^FANCY_BOXES */

/***********************
 * Misc terminal codes *
 ***********************/

#define TERM_HOME     "\x1b[H"
#define TERM_CLEAR    TERM_HOME "\x1b[2J"
#define cEOL          "\x1b[0K"
#define CURSOR_HIDE   "\x1b[?25l"
#define CURSOR_SHOW   "\x1b[?25h"

/************************
 * Debug & error macros *
 ************************/

/* Just print stuff to the appropriate stream. */

#ifdef MESSAGES_TO_STDOUT
#  define SAYF(x...)    printf(x)
#else 
#  define SAYF(x...)    fprintf(stderr, x)
#endif /* ^MESSAGES_TO_STDOUT */

/* Show a prefixed warning. */

#define WARNF(x...) do { \
    SAYF(cYEL "[!] " cBRI "WARNING: " cRST x); \
    SAYF(cRST "\n"); \
  } while (0)

/* Show a prefixed "doing something" message. */

#define ACTF(x...) do { \
    SAYF(cLBL "[*] " cRST x); \
    SAYF(cRST "\n"); \
  } while (0)

/* Show a prefixed "success" message. */

#define OKF(x...) do { \
    SAYF(cLGN "[+] " cRST x); \
    SAYF(cRST "\n"); \
  } while (0)

/* Show a prefixed fatal error message (not used in afl). */

#define BADF(x...) do { \
    SAYF(cLRD "\n[-] " cRST x); \
    SAYF(cRST "\n"); \
  } while (0)

/* Die with a verbose non-OS fatal error message. */

#define FATAL(x...) do { \
    SAYF(bSTOP RESET_G1 CURSOR_SHOW cRST cLRD "\n[-] PROGRAM ABORT : " \
         cBRI x); \
    SAYF(cLRD "\n         Location : " cRST "%s(), %s:%u\n\n", \
         __FUNCTION__, __FILE__, __LINE__); \
    exit(1); \
  } while (0)

/* Die by calling abort() to provide a core dump. */

#define ABORT(x...) do { \
    SAYF(bSTOP RESET_G1 CURSOR_SHOW cRST cLRD "\n[-] PROGRAM ABORT : " \
         cBRI x); \
    SAYF(cLRD "\n    Stop location : " cRST "%s(), %s:%u\n\n", \
         __FUNCTION__, __FILE__, __LINE__); \
    abort(); \
  } while (0)

/* Die while also including the output of perror(). */

#define PFATAL(x...) do { \
    fflush(stdout); \
    SAYF(bSTOP RESET_G1 CURSOR_SHOW cRST cLRD "\n[-]  SYSTEM ERROR : " \
         cBRI x); \
    SAYF(cLRD "\n    Stop location : " cRST "%s(), %s:%u\n", \
         __FUNCTION__, __FILE__, __LINE__); \
    SAYF(cLRD "       OS message : " cRST "%s\n", strerror(errno)); \
    exit(1); \
  } while (0)

/* Die with FAULT() or PFAULT() depending on the value of res (used to
   interpret different failure modes for read(), write(), etc). */

#define RPFATAL(res, x...) do { \
    if (res < 0) PFATAL(x); else FATAL(x); \
  } while (0)

/* Random number */

#define RANDOM(x) (random() % (x))

/* Logic state recording status */

typedef uint8_t recstat_t;

#define RECSTAT_RDY ((recstat_t)0)   // Ready for recording; SHM cleared
#define RECSTAT_REC ((recstat_t)1)   // Now recording
#define RECSTAT_FIN ((recstat_t)2)   // Recording finished
