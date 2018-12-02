#if !defined(POSITION_PIECE_READDITION_H)
#define POSITION_KING_SQUARE_H

/* This modules deals with re-additions of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Readd an already existing piece to the current move of the current ply
 * @param reason reason for adding the piece
 * @param on where to insert the piece
 * @param added nature of added piece
 * @param addedspec specs of added piece
 * @param for_side for which side is the (potientally neutral) piece re-added
 */
void move_effect_journal_do_piece_readdition(move_effect_reason_type reason,
                                             square on,
                                             piece_walk_type added,
                                             Flags addedspec,
                                             Side for_side);

/* Initalise the module */
void position_piece_readdition_initialise(void);

#endif
