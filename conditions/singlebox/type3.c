#include "conditions/singlebox/type3.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move_player.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Determine whether the move just played is legal according to Singlebox Type 3
 * @return true iff the move is legal
 */
static boolean is_last_move_illegal(void)
{
  boolean result = false;

  if (singlebox_type1_illegal())
    result = true;
  else if ((trait[nbply]==White && singlebox_illegal_latent_white_pawn())
           || (trait[nbply]==Black && singlebox_illegal_latent_black_pawn()))
    result = true;

  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type3(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSingleBoxType3LegalityTester);
  stip_instrument_moves(si,STSingleBoxType3PawnPromoter);

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
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type3_pawn_promoter_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[coup_id].singlebox_type3_promotion_what!=vide)
    move_effect_journal_do_piece_change(move_effect_reason_singlebox_promotion,
                                        move_generation_stack[coup_id].singlebox_type3_promotion_where,
                                        move_generation_stack[coup_id].singlebox_type3_promotion_what);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type3_legality_tester_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
