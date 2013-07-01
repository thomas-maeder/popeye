#if !defined(CONDITIONS_IMITATOR_H)
#define CONDITIONS_IMITATOR_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"
#include "position/position.h"

/* This module implements imitators */

extern boolean promotion_of_moving_into_imitator[maxply+1];
extern square im0;                    /* position of the 1st imitator */
extern imarr isquare;                 /* Imitatorstandfelder */
extern unsigned int number_of_imitators;       /* aktuelle Anzahl Imitatoren */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type imitator_remove_illegal_moves_solve(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type imitator_mover_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type moving_pawn_to_imitator_promoter_solve(slice_index si,
                                                        stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_imitator(slice_index si);

void undo_imitator_movement(move_effect_journal_index_type curr);
void redo_imitator_movement(move_effect_journal_index_type curr);

void undo_imitator_addition(move_effect_journal_index_type curr);
void redo_imitator_addition(move_effect_journal_index_type curr);

#endif
