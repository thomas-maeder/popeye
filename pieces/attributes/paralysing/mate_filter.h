#if !defined(PIECES_PARALYSING_MATE_FILTER_H)
#define PIECES_PARALYSING_MATE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STPiecesParalysingMateFilter - tests
 * additional conditions on some goals imposed by the presence of paralysing
 * pieces:
 * - the mated side must have >=1 move that only fails because of self-check
 */

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_slice(goal_applies_to_starter_or_adversary starter_or_adversary);

/* Allocate a STPiecesParalysingMateFilterTester slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_tester_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_solve(slice_index si);

#endif
