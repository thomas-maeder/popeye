#include "conditions/singlebox/type3.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/pieces.h"
#include "pieces/walks/classification.h"
#include "position/effects/walk_change.h"
#include "position/effects/null_move.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static struct
{
    square where;
    piece_walk_type what;
} promotion[toppile+1];

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

  solving_instrument_move_generation(si,nr_sides,STSingleBoxType3TMovesForPieceGenerator);

  move_effect_journal_register_pre_capture_effect();

  stip_instrument_observation_validation(si,nr_sides,STSingleBoxType3EnforceObserverWalk);
  stip_instrument_check_validation(si,nr_sides,STSingleBoxType3EnforceObserverWalk);

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
void singlebox_type3_pawn_promoter_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (promotion[move_generation_stack[curr].id].what==Empty)
    move_effect_journal_do_null_effect();
  else
    move_effect_journal_do_walk_change(move_effect_reason_singlebox_promotion,
                                        promotion[move_generation_stack[curr].id].where,
                                        promotion[move_generation_stack[curr].id].what);

  pipe_solve_delegate(si);

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
void singlebox_type3_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,is_last_move_illegal());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure to behave correctly while detecting observations by latent pawns
* @param si identifies tester slice
* @return true iff observation is valid
*/
boolean singleboxtype3_enforce_observer_walk(slice_index si)
{
  boolean result;
  Side const side_attacking = trait[nbply];
  square const sq_dep = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  SquareFlags const flag = side_attacking==White ? WhPromSq : BlPromSq;
  piece_walk_type const pprom = observing_walk[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_pawn(get_walk_of_piece_on_square(sq_dep))
      && TSTFLAG(sq_spec(sq_dep),flag))
  {
    if (being_solved.number_of_pieces[side_attacking][pprom]<game_array.number_of_pieces[side_attacking][pprom])
    {
      piece_walk_type const promotee = get_walk_of_piece_on_square(sq_dep);

      --being_solved.number_of_pieces[side_attacking][promotee];
      replace_walk(sq_dep,pprom);
      ++being_solved.number_of_pieces[side_attacking][pprom];

      result = pipe_validate_observation_recursive_delegate(si);

      --being_solved.number_of_pieces[side_attacking][pprom];
      replace_walk(sq_dep,promotee);
      ++being_solved.number_of_pieces[side_attacking][promotee];
    }
    else
      result = false;
  }
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static square find_next_latent_pawn(square sq, Side side)
{
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  while (sq!=square_h8)
  {
    if (sq==square_h1)
      sq = square_a8;
    else
      sq += dir_right;

    {
      piece_walk_type const walk_promotee = get_walk_of_piece_on_square(sq);
      if (is_pawn(walk_promotee)
          && TSTFLAG(being_solved.spec[sq],side)
          && (is_forwardpawn(walk_promotee)
              ? ForwardPromSq(side,sq)
              : ReversePromSq(side,sq)))
      {
        result = sq;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void singleboxtype3_generate_moves_for_piece(slice_index si)
{
  Side const side = trait[nbply];
  unsigned int nr_latent_promotions = 0;
  square where;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (where = find_next_latent_pawn(square_a1-dir_right,side);
       where!=initsquare;
       where = find_next_latent_pawn(where,side))
  {
    Side promoting_side;
    pieces_pawns_promotion_sequence_type sequence;
    singlebox_type2_initialise_singlebox_promotion_sequence(where,&promoting_side,&sequence);
    assert(promoting_side==side);
    while (sequence.promotee!=Empty)
    {
      numecoup curr_id = current_move_id[nbply];
      piece_walk_type const pi_departing = get_walk_of_piece_on_square(where);
      ++nr_latent_promotions;
      replace_walk(where,sequence.promotee);
      if (where==curr_generation->departure)
        pipe_move_generation_different_walk_delegate(si,sequence.promotee);
      else
        pipe_move_generation_delegate(si);

      for (; curr_id<current_move_id[nbply]; ++curr_id)
      {
        promotion[curr_id].where = where;
        promotion[curr_id].what = sequence.promotee;
      }
      replace_walk(where,pi_departing);
      singlebox_type2_continue_singlebox_promotion_sequence(promoting_side,&sequence);
    }
  }

  if (nr_latent_promotions==0)
  {
    numecoup curr_id = current_move_id[nbply];

    pipe_move_generation_delegate(si);

    for (; curr_id<current_move_id[nbply]; ++curr_id)
    {
      promotion[curr_id].where = initsquare;
      promotion[curr_id].what = Empty;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
