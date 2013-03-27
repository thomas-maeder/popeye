#if !defined(CONDITIONS_MADRASI_H)
#define CONDITIONS_MADRASI_H

#include "utilities/boolean.h"
#include "py.h"

/* Implementation of condition Madrasi
 */

boolean madrasi_is_observed(square sq);

/* Can a piece on a particular square can move according to Madrasi?
 * @param sq position of piece
 * @return true iff the piece can move according to Madrasi
 */
boolean madrasi_can_piece_move(square sq);

/* Inialise solving in Madrasi
 */
void madrasi_initialise_solving(void);

#endif
