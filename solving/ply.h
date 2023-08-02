#if !defined(SOLVING_PLY_H)
#define SOLVING_PLY_H

/* This module implements plys.
 * Each ply is a stage in the solving process where moves can be generated
 * and played from one position (usually the one produced by the current move
 * in the parent ply).
 */

#include "position/side.h"

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
  maxply = 702
#endif /* _WIN32 */
#endif /* __unix */
};

enum
{
  ply_nil,
  ply_diagram_setup,
  ply_twinning,
  ply_setup_solving,
  ply_retro_move_takeback,
  ply_retro_move
};

/* the current ply */
extern ply nbply;

extern ply parent_ply[maxply+1];

extern Side trait[maxply+1];

/* reset the ply module */
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
