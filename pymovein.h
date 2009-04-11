#if !defined(PYMOVEIN_H)
#define PYMOVEIN_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with move inverter slice
 */

/* Allocate a move inverter slice.
 * @param next next slice
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(slice_index next);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean move_inverter_must_starter_resign(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index move_inverter_root_make_setplay_slice(slice_index si);

/* Solve a move inverter at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
move_inverter_detect_starter(slice_index si,
                             boolean same_side_as_root);

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side move_inverter_get_starter(slice_index si);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void move_inverter_impose_starter(slice_index si, Side side);

#endif
