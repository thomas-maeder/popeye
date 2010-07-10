#if !defined(OPTIMISATION_INTELLIGENT_DUPLICATE_AVOIDER_H)
#define OPTIMISATION_INTELLIGENT_DUPLICATE_AVOIDER_H

#include "pyslice.h"

/* This module provides the slice type STIntelligentDuplicateAvoider
 * which avoids producing certain solutions twice in intelligent mode
 */

/* Initialise the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_init(void);

/* Cleanup the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_cleanup(void);

/* Allocate a STIntelligentDuplicateAvoider slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_duplicate_avoider_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type intelligent_duplicate_avoider_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type intelligent_duplicate_avoider_solve(slice_index si);

#endif
