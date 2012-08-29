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
#include "solving/move_effect_journal.h"
#include "conditions/castling_chess.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean promotion_of_moving_into_imitator[maxply+1];
square im0;                    /* position of the 1st imitator */

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static void move_imitators(int delta)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%u",number_of_imitators);
  for (i=0; i!=number_of_imitators; ++i)
  {
    isquare[i] += delta;
    TraceSquare(isquare[i]);
  }
  TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the movement of all imitators
 * @param reason reason for moving the imitators
 * @param delta how to move the imitators (to-from)
 */
static void move_effect_journal_do_imitator_movement(move_effect_reason_type reason,
                                                     int delta)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_imitator_movement;
  top_elmt->reason = reason;
  top_elmt->u.imitator_movement.delta = delta;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  move_imitators(delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_imitator_movement(move_effect_journal_index_type curr)
{
  int const delta = move_effect_journal[curr].u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  move_imitators(-delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_imitator_movement(move_effect_journal_index_type curr)
{
  int const delta = move_effect_journal[curr].u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  move_imitators(delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the addition of an imitator to the current move of the current ply
 * @param reason reason for adding the imitator
 * @param to where to add the imitator
 */
static void move_effect_journal_do_imitator_addition(move_effect_reason_type reason,
                                                     square to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_imitator_addition;
  top_elmt->reason = reason;
  top_elmt->u.imitator_addition.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  if (number_of_imitators==maxinum)
    FtlMsg(ManyImitators);

  isquare[number_of_imitators] = to;
  ++number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_imitator_addition(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  assert(number_of_imitators>0);
  --number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_imitator_addition(move_effect_journal_index_type curr)
{
  square const to = move_effect_journal[curr].u.imitator_addition.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (number_of_imitators==maxinum)
    FtlMsg(ManyImitators);

  isquare[number_of_imitators] = to;
  ++number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int imitator_diff(void)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
      switch (move_effect_journal[curr].reason)
      {
        case move_effect_reason_moving_piece_movement:
        case move_effect_reason_castling_partner_movement:
          result += move_effect_journal[curr].u.piece_movement.to-move_effect_journal[curr].u.piece_movement.from;
          break;

        default:
          break;
      }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
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
stip_length_type imitator_mover_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_imitator_movement(move_effect_reason_movement_imitation,
                                           diff);
  jouearr[nbply] = vide;
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
stip_length_type imitator_mover_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_imitator_movement(move_effect_reason_movement_imitation,diff);
  jouearr[nbply] = vide;
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
    move_effect_journal_do_piece_removal(move_effect_reason_pawn_promotion,
                                         sq_arrival);
    move_effect_journal_do_imitator_addition(move_effect_reason_pawn_promotion,
                                             sq_arrival);

    result = attack(slices[si].next2,n);

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
    move_effect_journal_do_piece_removal(move_effect_reason_pawn_promotion,
                                         sq_arrival);
    move_effect_journal_do_imitator_addition(move_effect_reason_pawn_promotion,
                                             sq_arrival);

    result = defend(slices[si].next2,n);

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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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
