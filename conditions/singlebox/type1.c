#include "conditions/singlebox/type1.h"
#include "pieces/pieces.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

ConditionNumberedVariantType SingleBoxType;

static boolean singlebox_officer_out_of_box(void)
{
  boolean result = false;
  piece_walk_type orthodox_walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (orthodox_walk = King; orthodox_walk<=Bishop; ++orthodox_walk)
  {
    piece_walk_type const standard_walk = standard_walks[orthodox_walk];
    if (being_solved.number_of_pieces[White][standard_walk]>game_array.number_of_pieces[White][standard_walk]
        || being_solved.number_of_pieces[Black][standard_walk]>game_array.number_of_pieces[Black][standard_walk])
    {
      TraceWalk(standard_walk);
      TraceValue("%u",being_solved.number_of_pieces[White][standard_walk]);
      TraceValue("%u",game_array.number_of_pieces[White][standard_walk]);
      TraceValue("%u",being_solved.number_of_pieces[Black][standard_walk]);
      TraceValue("%u",game_array.number_of_pieces[Black][standard_walk]);
      TraceEOL();
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean singlebox_pawn_out_of_box(void)
{
  boolean const result = (being_solved.number_of_pieces[White][Pawn]>game_array.number_of_pieces[White][Pawn]
                          || being_solved.number_of_pieces[Black][Pawn]>game_array.number_of_pieces[Black][Pawn]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the move just played is legal according to Singlebox Type 1
 * @return true iff the move is legal
 */
boolean singlebox_type1_illegal(void)
{
  return singlebox_officer_out_of_box() || singlebox_pawn_out_of_box();
}

/* Initialise solving in Singlebox Type 1
 * @param si identifies root slice of stipulation
 */
void singlebox_type1_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSingleBoxType1LegalityTester);

  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void singlebox_type1_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,singlebox_type1_illegal());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
