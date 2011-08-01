#if !defined(SOLVING_LEGAL_MOVE_FINDER_H)
#define SOLVING_LEGAL_MOVE_FINDER_H

#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STLegalMoveFinder stipulation slices.
 */

/* Information about the found legal move (if any) */
extern square legal_move_finder_departure;
extern square legal_move_finder_arrival;

/* Allocate a STLegalMoveFinder slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_finder_slice(void);

/* Initialise the search for a legal move */
void init_legal_move_finder(void);

/* Finalise the search for a legal move */
void fini_legal_move_finder(void);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type legal_move_finder_has_solution(slice_index si);

#endif
