#if !defined(CONDITION_HAUNTED_CHESS_H)
#define CONDITION_HAUNTED_CHESS_H

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "solving/move_effect_journal.h"

/* This module implements Haunted Chess.
 */

typedef struct
{
    square on;
    PieNam ghost;
    Flags flags;
} ghost_record_type;

enum
{
  ghost_capacity = 32,
  ghost_not_found = ghost_capacity
};

typedef ghost_record_type ghosts_type[ghost_capacity];

extern ghosts_type ghosts;

typedef unsigned int ghost_index_type;

extern ghost_index_type nr_ghosts;

/* Remember the ghost from the current capture
 */
void move_effect_journal_do_remember_ghost(void);
void move_effect_journal_undo_remember_ghost(move_effect_journal_index_type curr);
void move_effect_journal_redo_remember_ghost(move_effect_journal_index_type curr);

/* Forget a ghost "below" a square (typically because it is on the board now)
 */
void move_effect_journal_undo_forget_ghost(move_effect_journal_index_type curr);
void move_effect_journal_redo_forget_ghost(move_effect_journal_index_type curr);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type haunted_chess_ghost_summoner_solve(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type haunted_chess_ghost_rememberer_solve(slice_index si,
                                                       stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_haunted_chess(slice_index si);

#endif
