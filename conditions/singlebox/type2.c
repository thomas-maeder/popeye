#include "conditions/singlebox/type2.h"
#include "pieces/walks/classification.h"
#include "pieces/pieces.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "conditions/singlebox/type1.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

singlebox_type2_latent_pawn_promotion_type singlebox_type2_latent_pawn_promotions[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id_selection[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_promotion[maxply+1];

square next_latent_pawn(square s, Side c)
{
  numvec const delta= c==White ?+dir_left :+dir_right;
  int  i;

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
    if (TSTFLAG(being_solved.spec[s],c) && get_walk_of_piece_on_square(s)==Pawn)
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
    piece_walk_type p;
    for (p = Queen; p<=Bishop; ++p)
      if (being_solved.number_of_pieces[White][p]<game_array.number_of_pieces[White][p])
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
    piece_walk_type p;
    for (p = Queen; p<=Bishop; ++p)
      if (being_solved.number_of_pieces[Black][p]<game_array.number_of_pieces[Black][p])
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

  stip_instrument_moves(si,STSingleboxType2LatentPawnSelector);
  stip_instrument_moves(si,STSingleboxType2LatentPawnPromoter);
  stip_instrument_moves(si,STSingleBoxType2LegalityTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_latent_pawn_selection(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply);
  TraceFunctionParamListEnd();

  singlebox_type2_latent_pawn_promotions[nbply].where = next_latent_pawn(initsquare,adv);
  TraceSquare(singlebox_type2_latent_pawn_promotions[nbply].where);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_latent_pawn_selection(Side trait_ply)
{
  Side const adv = advers(trait_ply);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply);
  TraceFunctionParamListEnd();

  singlebox_type2_latent_pawn_promotions[nbply].where = next_latent_pawn(singlebox_type2_latent_pawn_promotions[nbply].where,adv);
  TraceSquare(singlebox_type2_latent_pawn_promotions[nbply].where);
  TraceEOL();

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
void singlebox_type2_latent_pawn_selector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(prev_post_move_iteration_id_selection[nbply]<=post_move_iteration_id[nbply]);
  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_selection[nbply])
    init_latent_pawn_selection(SLICE_STARTER(si));

  pipe_solve_delegate(si);

  if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
  {
    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_selection(SLICE_STARTER(si));

      if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_selection[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the sequence of promotions of a latent pawn
 * @param sq_prom potential promotion square
 * @param side address of side; *side will be assigned the side of which sq_prom
 *             is a promotion square
 * @param sequence address of structure holding the promotion sequence
 * @pre square sq_prom is occupied by a promotable pawn
 * @note the sequence only contains the promotees legal according to type 2
 */
void singlebox_type2_initialise_singlebox_promotion_sequence(square sq_prom,
                                                             Side *side,
                                                             pieces_pawns_promotion_sequence_type *sequence)
{
  *side = (is_forwardpawn(get_walk_of_piece_on_square(sq_prom)) ? ForwardPromSq(White,sq_prom) : ReversePromSq(White,sq_prom)) ? White : Black;
  pieces_pawns_start_promotee_sequence(sq_prom,*side,sequence);
  while (sequence->promotee!=Empty)
    if (sequence->promotee!=Pawn
        && being_solved.number_of_pieces[*side][sequence->promotee]<game_array.number_of_pieces[*side][sequence->promotee])
      break;
    else
      pieces_pawns_continue_promotee_sequence(sequence);
}

/* Continue a promotion sequence intialised by
 * singlebox_type2_initialise_singlebox_promotion_sequence()
 * @param side side of the promotion square assigned during the initialisation
 * @param sequence address of structure holding the promotion sequence
 */
void singlebox_type2_continue_singlebox_promotion_sequence(Side side,
                                                           pieces_pawns_promotion_sequence_type *sequence)
{
  pieces_pawns_continue_promotee_sequence(sequence);
  while (sequence->promotee!=Empty)
    if (sequence->promotee!=Pawn
        && being_solved.number_of_pieces[side][sequence->promotee]<game_array.number_of_pieces[side][sequence->promotee])
      break;
    else
      pieces_pawns_continue_promotee_sequence(sequence);
}

static void init_latent_pawn_promotion(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (singlebox_type2_latent_pawn_promotions[nbply].where!=initsquare)
  {
    singlebox_type2_initialise_singlebox_promotion_sequence(singlebox_type2_latent_pawn_promotions[nbply].where,
                                                            &singlebox_type2_latent_pawn_promotions[nbply].side,
                                                            &singlebox_type2_latent_pawn_promotions[nbply].promotion);
    if (singlebox_type2_latent_pawn_promotions[nbply].promotion.promotee==Empty)
      singlebox_type2_latent_pawn_promotions[nbply].where = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_latent_pawn_promotion(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  singlebox_type2_continue_singlebox_promotion_sequence(singlebox_type2_latent_pawn_promotions[nbply].side,
                                                        &singlebox_type2_latent_pawn_promotions[nbply].promotion);
  TraceWalk(singlebox_type2_latent_pawn_promotions[nbply].promotion.promotee);
  TraceEOL();

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
void singlebox_type2_latent_pawn_promoter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(prev_post_move_iteration_id_promotion[nbply]<=post_move_iteration_id[nbply]);
  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_promotion[nbply])
    init_latent_pawn_promotion();

  if (singlebox_type2_latent_pawn_promotions[nbply].promotion.promotee==Empty)
    pipe_solve_delegate(si);
  else
  {
    move_effect_journal_do_walk_change(move_effect_reason_singlebox_promotion,
                                        singlebox_type2_latent_pawn_promotions[nbply].where,
                                        singlebox_type2_latent_pawn_promotions[nbply].promotion.promotee);
    pipe_solve_delegate(si);

    if (!post_move_iteration_locked[nbply])
    {
      advance_latent_pawn_promotion();
      if (singlebox_type2_latent_pawn_promotions[nbply].promotion.promotee!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_promotion[nbply] = post_move_iteration_id[nbply];

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
void singlebox_type2_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,is_last_move_illegal());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
