#include "conditions/ghost_chess.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "conditions/haunted_chess.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <string.h>

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void ghost_chess_ghost_rememberer_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    move_effect_journal_do_remember_ghost();
    SETFLAG(underworld[nr_ghosts-1].flags,Uncapturable);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_ghost_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  haunted_chess_initialise_move_doers();

  stip_instrument_moves(si,STGhostChessGhostRememberer);
  stip_instrument_moves(si,STHauntedChessGhostSummoner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
