#if !defined(PYBRADD_H)
#define PYBRADD_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"
#include "pydirect.h"
#include "pytable.h"

/* This module provides functionality dealing with the defending side
 * in STBranchDirect stipulation slices.
 */

/* Allocate a STBranchDirectDefender defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_slice(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index next,
                                          slice_index towards_goal);

/* Allocate a STDirectDefenderRoot defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_root_slice(stip_length_type length,
                                               stip_length_type min_length,
                                               slice_index next,
                                               slice_index towards_goal);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_defender_is_goal_reached(Side just_moved, slice_index si);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean branch_d_defender_defend_in_n(slice_index si,
                                      stip_length_type n,
                                      unsigned int curr_max_nr_nontrivial);

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean branch_d_defender_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          unsigned int curr_max_nr_nontrivial);

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_solve_postkey_in_n(slice_index si, stip_length_type n);

/* Find solutions in next slice
 * @param si slice index
 * @return true iff >=1 solution has been found
 */
boolean branch_d_defender_solve_next(slice_index si);

/* Solve at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_root_solve(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return true iff the attacker has reached a deadend (e.g. by
 *         immobilising the defender in a non-stalemate stipulation)
 */
boolean branch_d_defender_root_defend(table refutations, slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
branch_d_defender_root_detect_starter(slice_index si, boolean same_side_as_root);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index branch_d_defender_root_reduce_to_postkey_play(slice_index si);

#endif
