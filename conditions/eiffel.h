#if !defined(CONDITIONS_EIFFEL_H)
#define CONDITIONS_EIFFEL_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Eiffel chess
 */

/* Can a piece on a particular square can move according to Eiffel chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean eiffel_can_piece_move(square sq);

/* Inialise solving in Eiffel Chess
 */
void eiffel_initialise_solving(void);

#endif
