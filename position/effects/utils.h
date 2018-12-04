#if !defined(POSITION_EFFECTS_UTILS_H)
#define POSITION_EFFECTS_UTILS_H

/* This modules deals with changes of pieces' flags for various reasons
 */

#include "position/board.h"
#include "position/pieceid.h"
#include "solving/ply.h"

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param ply ply in which the move was played
 * @param followed_id id of the piece to be followed
 * @param pos position of the piece after the inital capture removal and piece movement have taken place
 * @return the position of the piece with the "other" effect applied
 *         initsquare if the piece is not on the board after the "other" effects
 */
square move_effect_journal_follow_piece_through_other_effects(ply ply,
                                                              PieceIdType followed_id,
                                                              square pos);

#endif
