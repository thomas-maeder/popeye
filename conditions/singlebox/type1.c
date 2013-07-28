#include "conditions/singlebox/type1.h"
#include "pydata.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "solving/observation.h"
#include "solving/single_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

static boolean singlebox_officer_out_of_box(void)
{
  boolean result = false;
  PieNam orthodox_walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (orthodox_walk = King; orthodox_walk<=Bishop; ++orthodox_walk)
  {
    PieNam const standard_walk = standard_walks[orthodox_walk];
    if (number_of_pieces[White][standard_walk]>game_array.number_of_pieces[White][standard_walk]
        || number_of_pieces[Black][standard_walk]>game_array.number_of_pieces[Black][standard_walk])
    {
      TracePiece(standard_walk);
      TraceValue("%u",number_of_pieces[White][standard_walk]);
      TraceValue("%u",game_array.number_of_pieces[White][standard_walk]);
      TraceValue("%u",number_of_pieces[Black][standard_walk]);
      TraceValue("%u\n",game_array.number_of_pieces[Black][standard_walk]);
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
  boolean const result = (number_of_pieces[White][Pawn]>game_array.number_of_pieces[White][Pawn]
                          || number_of_pieces[Black][Pawn]>game_array.number_of_pieces[Black][Pawn]);

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
stip_length_type singlebox_type1_legality_tester_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (singlebox_type1_illegal())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
