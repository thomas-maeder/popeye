#include "pieces/attributes/chameleon.h"
#include "pieces/walks/walks.h"
#include "conditions/anticirce/promotion.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include <assert.h>

boolean promotion_of_moving_into_chameleon[maxply+1];
boolean promotion_of_circe_reborn_into_chameleon[maxply+1];

char ChameleonSequence[256];

static boolean promotion_of_anticirce_reborn_into_chameleon[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id_moving[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_circe_reborn[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_anticirce_reborn[maxply+1];

static boolean find_promotion(square sq_arrival)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  boolean result = false;

  for (curr = base+move_effect_journal_index_offset_other_effects; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_change
        && move_effect_journal[curr].u.piece_change.on==sq_arrival)
    {
      result = true;
      break;
    }

  return result;
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
stip_length_type chameleon_promote_moving_into_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_moving[nbply])
    promotion_of_moving_into_chameleon[nbply] = false;

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (promotion_of_moving_into_chameleon[nbply])
    {
      Flags changed = spec[pos];
      SETFLAG(changed,Chameleon);
      move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                          pos,changed);

      result = solve(slices[si].next1,n);
    }
    else
    {
      result = solve(slices[si].next1,n);

      if (!TSTFLAG(spec[pos],Chameleon)
          && find_promotion(pos)
          && !post_move_iteration_locked[nbply])
      {
        promotion_of_moving_into_chameleon[nbply] = true;
        lock_post_move_iterations();
      }
    }
  }

  prev_post_move_iteration_id_moving[nbply] = post_move_iteration_id[nbply];

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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_promote_circe_reborn_into_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();
    if (rebirth==move_effect_journal_base[nbply+1])
      result = solve(slices[si].next1,n);
    else
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;

      if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_circe_reborn[nbply])
        promotion_of_circe_reborn_into_chameleon[nbply] = false;

      if (promotion_of_circe_reborn_into_chameleon[nbply])
      {
        Flags changed = spec[sq_rebirth];
        SETFLAG(changed,Chameleon);
        move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                            sq_rebirth,changed);

        result = solve(slices[si].next1,n);
      }
      else
      {
        result = solve(slices[si].next1,n);

        if (reborn_pawn_promotion_state[nbply].promotee!=Empty
            && !post_move_iteration_locked[nbply])
        {
          promotion_of_circe_reborn_into_chameleon[nbply] = true;
          lock_post_move_iterations();
        }
      }

      prev_post_move_iteration_id_circe_reborn[nbply] = post_move_iteration_id[nbply];
    }
  }

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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_promote_anticirce_reborn_into_solve(slice_index si,
                                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = anticirce_find_current_rebirth();
    if (rebirth==move_effect_journal_base[nbply+1])
      result = solve(slices[si].next1,n);
    else
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;

      if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_anticirce_reborn[nbply])
        promotion_of_anticirce_reborn_into_chameleon[nbply] = false;

      if (promotion_of_anticirce_reborn_into_chameleon[nbply])
      {
        Flags changed = spec[sq_rebirth];
        SETFLAG(changed,Chameleon);
        move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                            sq_rebirth,changed);

        result = solve(slices[si].next1,n);
      }
      else
      {
        result = solve(slices[si].next1,n);

        if (current_promotion_of_reborn_moving[nbply].promotee!=Empty
            && !post_move_iteration_locked[nbply])
        {
          promotion_of_anticirce_reborn_into_chameleon[nbply] = true;
          lock_post_move_iterations();
        }
      }

      prev_post_move_iteration_id_anticirce_reborn[nbply] = post_move_iteration_id[nbply];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static PieNam champiece(PieNam walk_arriving)
{
  PieNam walk_chameleonised = walk_arriving;

  if (walk_arriving==standard_walks[Queen])
    walk_chameleonised = standard_walks[Knight];
  else if (walk_arriving==standard_walks[Knight])
    walk_chameleonised = standard_walks[Bishop];
  else if (walk_arriving==standard_walks[Bishop])
    walk_chameleonised = standard_walks[Rook];
  else if (walk_arriving==standard_walks[Rook])
    walk_chameleonised = standard_walks[Queen];

  return walk_chameleonised;
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
stip_length_type chameleon_arriving_adjuster_solve(slice_index si,
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
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (TSTFLAG(movingspec,Chameleon))
      move_effect_journal_do_piece_change(move_effect_reason_chameleon_movement,
                                          pos,
                                          champiece(get_walk_of_piece_on_square(pos)));
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery for solving problems with some
 * chameleon pieces
 * @param si identifies root slice of stipulation
 */
void chameleon_initialse_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonArrivingAdjuster);
  stip_instrument_moves(si,STPromoteMovingIntoChameleon);

  if (anycirprom)
    stip_instrument_moves(si,STPromoteCirceRebornIntoChameleon);

  if (anyanticirprom)
    stip_instrument_moves(si,STPromoteAnticirceRebornIntoChameleon);

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
stip_length_type chameleon_chess_arriving_adjuster_solve(slice_index si,
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
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    move_effect_journal_do_piece_change(move_effect_reason_chameleon_movement,
                                        pos,
                                        champiece(get_walk_of_piece_on_square(pos)));
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery for solving problems with the condition
 * Chameleon Chess
 * @param si identifies root slice of stipulation
 */
void chameleon_chess_initialse_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonChessArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
