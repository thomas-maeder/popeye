#if !defined(OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H)
#define OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H

#include "pyslice.h"

/* This module provides the STOutputPlaintextMoveInversionCounter
 * slice type. Slices of this type count the move inversions occuring
 * throughout the play.
 */

/* Number of move inversions up to the current move.
 * Exposed for read-only access only
 */
extern unsigned int output_plaintext_nr_move_inversions;


/* Allocate a STOutputPlaintextMoveInversionCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_move_inversion_counter_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_move_inversion_counter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_move_inversion_counter_solve(slice_index si);

#endif
