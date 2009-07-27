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
 * Tests do not rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff starter must resign
 */
boolean move_inverter_must_starter_resign(slice_index si);

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean move_inverter_must_starter_resign_hashed(slice_index si);

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

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void move_inverter_root_solve_in_n(slice_index si, stip_length_type n);

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

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean move_inverter_impose_starter(slice_index si, slice_traversal *st);

#endif
