#include "conditions/singlebox/type2.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "conditions/singlebox/type1.h"
#include "debugging/trace.h"

#include <assert.h>

singlebox_type2_latent_pawn_promotion_type singlebox_type2_latent_pawn_promotions[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id_selection[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_promotion[maxply+1];

square next_latent_pawn(square s, Side c)
{
  piece pawn;
  int  i, delta;

  pawn=  c==White ? pb : pn;
  delta= c==White ?+dir_left :+dir_right;

  if (s==initsquare)
  {
    i = 0;
    s = c==White ? square_h8 : square_a1;
  }
  else
  {
    i = c==White ? square_h8-s+1 : s-square_a1+1;
    s += delta;
  }

  for (; i<8; ++i, s+=delta)
    if (e[s]==pawn)
      return s;

  return initsquare;
}

/* Does the current position contain an illegal latent white pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_white_pawn(void)
{
  boolean result = false;
  square const next_latent_white = next_latent_pawn(initsquare,White);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (next_latent_white!=initsquare)
  {
    piece p;
    for (p = db; p<=fb; ++p)
      if (nbpiece[p]<nr_piece(game_array)[p])
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

/* Does the current position contain an illegal latent black pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_black_pawn(void)
{
  boolean result = false;
  square const next_latent_black = next_latent_pawn(initsquare,Black);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (next_latent_black!=initsquare)
  {
    piece p;
    for (p = dn; p>=fn; --p)
      if (nbpiece[p]<nr_piece(game_array)[p])
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

static boolean is_last_move_illegal(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (singlebox_type1_illegal())
    result = true;
  else if (singlebox_illegal_latent_white_pawn()
           || singlebox_illegal_latent_black_pawn())
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type2(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves_no_replay(si,STSingleboxType2LatentPawnSelector);
  stip_instrument_moves_no_replay(si,STSingleboxType2LatentPawnPromoter);
  stip_instrument_moves_no_replay(si,STSingleBoxType2LegalityTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_latent_pawn_selection(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  singlebox_type2_latent_pawn_promotions[nbply].where = next_latent_pawn(initsquare,adv);
  TraceSquare(singlebox_type2_latent_pawn_promotions[nbply].where);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_latent_pawn_selection(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  singlebox_type2_latent_pawn_promotions[nbply].where = next_latent_pawn(singlebox_type2_latent_pawn_promotions[nbply].where,adv);
  TraceSquare(singlebox_type2_latent_pawn_promotions[nbply].where);TraceText("\n");

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
stip_length_type singlebox_type2_latent_pawn_selector_attack(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_selection[nbply])
    init_latent_pawn_selection(slices[si].starter);

  result = attack(next,n);

  if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
  {
    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_selection(slices[si].starter);

      if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_selection[nbply] = post_move_iteration_id[nbply];

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
stip_length_type singlebox_type2_latent_pawn_selector_defend(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_selection[nbply])
    init_latent_pawn_selection(slices[si].starter);

  result = defend(next,n);

  if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
  {
    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_selection(slices[si].starter);
      if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_selection[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void init_latent_pawn_promotion(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
  {
    singlebox_type2_latent_pawn_promotions[nbply].what = next_singlebox_prom(Empty,adv);
    TracePiece(singlebox_type2_latent_pawn_promotions[nbply].what);TraceText("\n");

    if (singlebox_type2_latent_pawn_promotions[nbply].what==Empty)
      singlebox_type2_latent_pawn_promotions[nbply].where = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_latent_pawn_promotion(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  singlebox_type2_latent_pawn_promotions[nbply].what = next_singlebox_prom(singlebox_type2_latent_pawn_promotions[nbply].what,adv);
  TracePiece(singlebox_type2_latent_pawn_promotions[nbply].what);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_promotee(Side trait_ply)
{
  piece const promotee = (trait_ply==Black
                          ? singlebox_type2_latent_pawn_promotions[nbply].what
                          : -singlebox_type2_latent_pawn_promotions[nbply].what);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                      singlebox_type2_latent_pawn_promotions[nbply].where,
                                      promotee);

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
stip_length_type singlebox_type2_latent_pawn_promoter_attack(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_promotion[nbply])
    init_latent_pawn_promotion(slices[si].starter);

  if (singlebox_type2_latent_pawn_promotions[nbply].what==Empty)
    result = attack(next,n);
  else
  {
    place_promotee(slices[si].starter);
    result = attack(next,n);

    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_promotion(slices[si].starter);

      if (singlebox_type2_latent_pawn_promotions[nbply].what!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_promotion[nbply] = post_move_iteration_id[nbply];

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
stip_length_type singlebox_type2_latent_pawn_promoter_defend(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_promotion[nbply])
    init_latent_pawn_promotion(slices[si].starter);

  if (singlebox_type2_latent_pawn_promotions[nbply].what==Empty)
    result = defend(next,n);
  else
  {
    place_promotee(slices[si].starter);
    result = defend(next,n);

    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_promotion(slices[si].starter);

      if (singlebox_type2_latent_pawn_promotions[nbply].what!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_promotion[nbply] = post_move_iteration_id[nbply];

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
stip_length_type singlebox_type2_legality_tester_attack(slice_index si,
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
stip_length_type singlebox_type2_legality_tester_defend(slice_index si,
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
