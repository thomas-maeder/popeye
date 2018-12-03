#if !defined(POSITION_PIECE_EXCHANGE_H)
#define POSITION_PIECE_EXCHANGE_H

/* This modules deals with exchanges of places of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add exchanging two pieces to the current move of the current ply
 * @param reason reason for exchanging the two pieces
 * @param from position of primary piece
 * @param to position of partner piece
 */
void move_effect_journal_do_piece_exchange(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Initalise the module */
void position_piece_exchange_initialise(void);

#endif
