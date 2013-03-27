#if !defined(PIECES_PARALYSING_H)
#define PIECES_PARALYSING_H

#include "stipulation/slice.h"


/* Determine whether a piece is paralysed
 * @param s position of piece
 * @return true iff the piece on square s is paralysed
 */
boolean is_piece_paralysed_on(square s);

/* Determine whether a side is "suffocated by paralysis", i.e. would the side
 * have moves (possibly exposing the side to self check) if no piece were
 * paralysing.
 * @param side side for which to determine whether it is suffocated
 * @return true iff side is suffocated by paralysis
 */
boolean suffocated_by_paralysis(Side side);

/* Initialise solving with paralysing pieces
 * @param si root of branch to be instrumented
 */
void paralysing_initialise_solving(slice_index si);

#endif
