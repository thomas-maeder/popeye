#if !defined(SOLVING_PLY_H)
#define SOLVING_PLY_H

/* This module implements plys.
 * Each ply is a stage in the solving process where moves can be generated
 * and played from one position (usually the one produced by the current move
 * in the parent ply).
 */

#include "position/position.h"

typedef unsigned int ply;

enum
{
#if defined(__unix)
  maxply =   1002
#else
#if defined(_WIN32)
  maxply = 2702
#elif defined(_OS2)
  maxply = 302
#else
#if defined(SIXTEEN) /* DOS 16 Bit, ... */
#if defined(MSG_IN_MEM)
  maxply = 26
#else
  maxply = 48
#endif /* MSG_IN_MEM */
#else
  maxply = 702
#endif/* SIXTEEN */
#endif /* _WIN32 */
#endif /* __unix */
};

enum
{
  nil_ply = 1
};

/* the current ply */
extern ply nbply;

extern ply parent_ply[maxply+1];

/* resset the ply module */
void ply_reset(void);

/* Copy the current ply, making the copy the new current ply */
void copyply(void);

/* Start a new ply as the child of the current ply, making the child the new
 * current ply
 * @param side the side at the move in the child ply
 */
void nextply(Side side);

/* Start a new ply as a sibling of the current ply, making the child the new
 * current ply
 * @param side the side at the move in the child ply
 */
void siblingply(Side side);

/* End the current ply, making the previous current ply the current ply again */
void finply(void);

#endif
