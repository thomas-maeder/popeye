#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "pyslice.h"
#include "pydirect.h"

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int leaf_forced_count_refutations(slice_index leaf,
                                           unsigned int max_result);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_forced_defend(slice_index leaf);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
boolean leaf_forced_has_solution(slice_index leaf);

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_forced_has_non_starter_solved(slice_index leaf);

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf);

/* Determine and write the solution of a leaf forced slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_root_solve(slice_index leaf);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_forced_root_defend(slice_index leaf,
                                unsigned int max_number_refutations);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean leaf_forced_detect_starter(slice_index si, slice_traversal *st);

#endif
