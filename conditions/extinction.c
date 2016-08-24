#include "conditions/extinction.h"
#include "pieces/pieces.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/pipe.h"
#include "position/position.h"
#include "debugging/trace.h"

/* test for extinction rather than king capture */
static void substitute_extinction_tester(slice_index si, stip_structure_traversal*st)
{
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STExtinctionExtinctedTester));
}

/* test all attacked pieces, not just the king */
static void substitute_all_pieces_observation_tester(slice_index si, stip_structure_traversal*st)
{
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STExtinctionAllPieceObservationTester));
}

/* Initialise the solving machinery with Extinction Chess
 * @param si identifies root slice of stipulation
 */
void extinction_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we have to actually play potentially extincting moves to find out whether
   * they really are in conditions such as Circe
   */
  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STGoalKingCaptureReachedTester,
                                             &substitute_extinction_tester);
    stip_structure_traversal_override_single(&st,
                                             STKingSquareObservationTester,
                                             &substitute_all_pieces_observation_tester);
    stip_traverse_structure(si,&st);
  }

  check_even_if_no_king();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean exctinction_all_piece_observation_tester_is_in_check(slice_index si,
                                                             Side side_attacked)
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_attacked);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    if (TSTFLAG(being_solved.spec[*bnp],side_attacked))
    {
      replace_observation_target(*bnp);
      if (is_square_observed(EVALUATE(check)))
      {
        result = true;
        break;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static piece_walk_type find_capturee(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base + move_effect_journal_index_offset_capture;

  if (move_effect_journal[capture].type==move_effect_piece_removal)
    return move_effect_journal[capture].u.piece_removal.walk;
  else
    return Empty;
}

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
void extinction_extincted_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = SLICE_STARTER(si);
    piece_walk_type const capturee = find_capturee();

    TraceWalk(capturee);
    TraceEnumerator(Side,side_in_check);
    TraceEOL();

    pipe_this_move_doesnt_solve_if(si,
                                   capturee==Empty
                                   || being_solved.number_of_pieces[side_in_check][capturee]!=0);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
