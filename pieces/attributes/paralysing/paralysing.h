#if !defined(PIECES_PARALYSING_H)
#define PIECES_PARALYSING_H

#include "pystip.h"

/* Determine whether a piece is paralysed
 * @param s position of piece
 * @return true iff the piece on square s is paralysed
 */
boolean paralysiert(square s);

/* Determine whether a side is "suffocated by paralysis", i.e. would the side
 * have moves (possibly exposing the side to self check) if no piece were
 * paralysing.
 * @param side side for which to determine whether it is suffocated
 * @return true iff side is suffocated by paralysis
 */
boolean suffocated_by_paralysis(Side side);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_paralysing_goal_filters(slice_index si);

#endif
