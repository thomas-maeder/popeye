#if !defined(POSITION_PIECE_CREATION_H)
#define POSITION_PIECE_CREATION_H

/* This modules deals with re-additions of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add an newly created piece to the current move of the current ply
 * @param reason reason for creating the piece
 * @param on where to insert the piece
 * @param created nature of created piece
 * @param createdspec specs of created piece
 * @param for which side is the (potentially neutral) piece created
 */
void move_effect_journal_do_piece_creation(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type created,
                                           Flags createdspec,
                                           Side for_side);

/* Initalise the module */
void position_piece_creation_initialise(void);

#endif
