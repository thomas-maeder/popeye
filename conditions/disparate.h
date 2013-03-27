#if !defined(CONDITIONS_DISPARATE_H)
#define CONDITIONS_DISPARATE_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Disparate chess
 */

/* Can a piece on a particular square can move according to Disparate chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean disparate_can_piece_move(square sq);

/* Inialise solving in Disparate chess
 */
void disparate_initialise_solving(void);

#endif
