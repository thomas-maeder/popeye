#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */

/* Allocate a quodlibet slice.
 * @param proxy1 proxy to 1st operand
 * @param proxy2 proxy to 1st operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index proxy1, slice_index proxy2);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean quodlibet_insert_root(slice_index si, slice_traversal *st);

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void quodlibet_solve_threats(table threats, slice_index si);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean quodlibet_are_threats_refuted(table threats, slice_index si);

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si);

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si);

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int quodlibet_count_refutations(slice_index si,
                                         unsigned int max_result);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean quodlibet_defend(slice_index si);

/* Solve a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve(slice_index si);

/* Solve a quodlibet slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_root_solve(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean quodlibet_root_defend(slice_index si,
                              unsigned int max_number_refutations);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean quodlibet_detect_starter(slice_index si, slice_traversal *st);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean quodlibet_impose_starter(slice_index si, slice_traversal *st);

#endif
