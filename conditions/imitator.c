#include "conditions/imitator.h"
#include "pymsg.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "conditions/castling_chess.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean promotion_of_moving_into_imitator[maxply+1];
square im0;                    /* position of the 1st imitator */

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static int imitator_diff(void)
{
  numecoup const coup_id = current_move[nbply];
  square const sq_departure = move_generation_stack[coup_id].departure;
  square const sq_capture = move_generation_stack[coup_id].capture;
  square const sq_arrival = move_generation_stack[coup_id].arrival;

  if (sq_capture==queenside_castling)
    return +dir_right;
  else if (sq_capture==kingside_castling)
    return 0;
  else if (sq_capture>platzwechsel_rochade)
    return (3*sq_arrival-sq_departure-castling_partner_origin[coup_id]) / 2;
  else
    return sq_arrival-sq_departure;
}

static void move_imitators(int diff)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",diff);
  TraceFunctionParamListEnd();

  TraceValue("%u",number_of_imitators);
  for (i=0; i!=number_of_imitators; ++i)
  {
    isquare[i] += diff;
    TraceSquare(isquare[i]);
  }
  TraceText("\n");

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
stip_length_type imitator_mover_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_imitators(diff);
  result = attack(slices[si].next1,n);
  move_imitators(-diff);

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
stip_length_type imitator_mover_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_imitators(diff);
  result = defend(slices[si].next1,n);
  move_imitators(-diff);

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
stip_length_type moving_pawn_to_imitator_promoter_attack(slice_index si,
                                                         stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    promotion_of_moving_into_imitator[nbply] = has_pawn_reached_promotion_square(slices[si].starter,sq_arrival);

  if (promotion_of_moving_into_imitator[nbply])
  {
    if (number_of_imitators==maxinum)
      FtlMsg(ManyImitators);

    isquare[number_of_imitators] = sq_arrival;
    ++number_of_imitators;
    replace_arriving_piece(vide);

    result = attack(slices[si].next2,n);

    --number_of_imitators;

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_moving_into_imitator[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = attack(slices[si].next1,n);

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

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
stip_length_type moving_pawn_to_imitator_promoter_defend(slice_index si,
                                                         stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    promotion_of_moving_into_imitator[nbply] = has_pawn_reached_promotion_square(slices[si].starter,sq_arrival);

  if (promotion_of_moving_into_imitator[nbply])
  {
    if (number_of_imitators==maxinum)
      FtlMsg(ManyImitators);

    isquare[number_of_imitators] = sq_arrival;
    ++number_of_imitators;
    replace_arriving_piece(vide);

    result = defend(slices[si].next2,n);

    --number_of_imitators;

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_moving_into_imitator[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = defend(slices[si].next1,n);

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STLandingAfterMovingPawnPromoter);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_promoter(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const prototype = alloc_fork_slice(STMovingPawnToImitatorPromoter,proxy);
    assert(*landing!=no_slice);
    link_to_branch(proxy,*landing);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;
  slice_index const save_landing = *landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *landing = no_slice;
  insert_landing(si,st);
  stip_traverse_structure_children_pipe(si,st);
  insert_promoter(si,st);
  *landing = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  *landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_promoters(slice_index si)
{
  stip_structure_traversal st;
  slice_index landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&landing);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,STReplayingMoves,&instrument_move);
  stip_structure_traversal_override_single(&st,STLandingAfterMovingPawnPromoter,&remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void stip_insert_imitator(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!CondFlag[noiprom])
    insert_promoters(si);

  stip_instrument_moves(si,STImitatorMover);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
