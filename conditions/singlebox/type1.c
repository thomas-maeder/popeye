#include "conditions/singlebox/type1.h"
#include "pydata.h"
#include "pieces/walks.h"
#include "pieces/pawns/promotion.h"
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
#include <stdlib.h>

static boolean avoid_observation_by_unpromotable_pawn(square sq_observer,
                                                      square sq_landing,
                                                      square sq_observee)
{
  boolean result;
  Side const side_observing = e[sq_observer]>vide ? White : Black;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  init_single_move_generator(sq_observer,sq_landing,sq_observee);
  result = solve(slices[temporary_hack_king_capture_legality_tester[side_observing]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean singlebox_officer_out_of_box(void)
{
  boolean result = false;
  PieNam orthodox_walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (orthodox_walk = King; orthodox_walk<=Bishop; ++orthodox_walk)
  {
    PieNam const standard_walk = standard_walks[orthodox_walk];
    piece const p_white = standard_walk;
    piece const p_black = -standard_walk;
    if (nbpiece[p_white]>nr_piece(game_array)[p_white]
        || nbpiece[p_black]>nr_piece(game_array)[p_black])
    {
      TracePiece(standard_walk);
      TraceValue("%u",nbpiece[p_white]);
      TraceValue("%u",nr_piece(game_array)[p_white]);
      TraceValue("%u",nbpiece[p_black]);
      TraceValue("%u\n",nr_piece(game_array)[p_black]);
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
  boolean const result = (nbpiece[pb]>nr_piece(game_array)[pb]
                          || nbpiece[pn]>nr_piece(game_array)[pn]);

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

  register_observation_validator(&avoid_observation_by_unpromotable_pawn);

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
