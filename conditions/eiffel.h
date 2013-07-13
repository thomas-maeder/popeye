#if !defined(CONDITIONS_EIFFEL_H)
#define CONDITIONS_EIFFEL_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Eiffel chess
 */

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void eiffel_generate_moves_for_piece(slice_index si,
                                     square sq_departure,
                                     PieNam p);

/* Inialise the solving machinery with Eiffel Chess
 * @param si identifies root slice of solving machinery
 */
void eiffel_initialise_solving(slice_index si);

#endif
