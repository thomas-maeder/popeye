#include "conditions/circe/kamikaze.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/capture_fork.h"
#include "solving/move_effect_journal.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

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
stip_length_type circe_kamikaze_capture_fork_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    if (TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Kamikaze))
      result = anticirce_capture_fork_solve(si,n);
    else
    {
      anticirce_current_rebirth_square[nbply] = initsquare;
      result = solve(slices[si].next2,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void replace(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  slices[si].type = STCirceKamikazeCaptureFork;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the Circe instrumentation of the solving machinery with
 * Kamikaze+Circe
 * @param si identifies root slice of stipulation
 */
void circe_kamikaze_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,STAnticirceDetermineRevelantPiece);
  stip_instrument_moves(si,STAnticirceDetermineRebirthSquare);
  stip_insert_anticirce_capture_forks(si);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STAnticirceCaptureFork,&replace);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
