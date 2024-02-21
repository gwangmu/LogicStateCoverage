/*
 * lscov - some configurations 
 * ---------------------------
 *
 * Constants and stuff.
 */

#ifndef _HAVE_CONFIG_H
#define _HAVE_CONFIG_H

/* Version string */

#define VERSION "0.01"

/* Logic state size: simply following MAP_SIZE in AFL. */

#define LSTATE_SIZE_POW2 16
#define LSTATE_SIZE      (1 << LSTATE_SIZE_POW2)

#endif /* ! _HAVE_CONFIG_H */
