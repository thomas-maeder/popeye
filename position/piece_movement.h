#if !defined(POSITION_PIECE_MOVEMENT_H)
#define POSITION_PIECE_MOVEMENT_H

/* This modules deals with movements of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 */
void move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Determine the departure square of a moveplayed
 * Assumes that the move has a single moving piece (i.e. is not a castling).
 * @param ply identifies the ply where the move is being or was played
 * @return the departure square; initsquare if the last move didn't have a movement
 */
square move_effect_journal_get_departure_square(ply ply);

/* Initalise the module */
void position_piece_movement_initialise(void);

#endif
