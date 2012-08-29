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

singlebox_type3_promotion_type singlebox_type3_promotions[toppile+1];

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

  stip_instrument_moves_no_replay(si,STSingleBoxType3LegalityTester);
  stip_instrument_moves_no_replay(si,STSingleBoxType3PawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type3_pawn_promoter_attack(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (singlebox_type3_promotions[coup_id].what!=vide)
    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        singlebox_type3_promotions[coup_id].where,
                                        singlebox_type3_promotions[coup_id].what);

  result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type singlebox_type3_pawn_promoter_defend(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();


  if (singlebox_type3_promotions[coup_id].what!=vide)
    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        singlebox_type3_promotions[coup_id].where,
                                        singlebox_type3_promotions[coup_id].what);

  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type3_legality_tester_attack(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = n+2;
  else
    result = attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type singlebox_type3_legality_tester_defend(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = slack_length-1;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
