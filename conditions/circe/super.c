#include "conditions/circe/super.h"
#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "conditions/circe/rebirth_handler.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static void advance_rebirth_square(void)
{
  square next_rebirth_square = current_circe_rebirth_square[nbply]+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (e[next_rebirth_square]!=vide && next_rebirth_square<=square_h8)
    ++next_rebirth_square;

  if (next_rebirth_square>square_h8)
    current_circe_rebirth_square[nbply] = initsquare;
  else
  {
    current_circe_rebirth_square[nbply] = next_rebirth_square;
    lock_post_move_iterations();
  }

  TraceSquare(current_circe_rebirth_square[nbply]);TraceText("\n");

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
stip_length_type supercirce_rebirth_handler_attack(slice_index si,
                                                   stip_length_type n)
{
  square const pi_captured = pprise[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    current_circe_rebirth_square[nbply] = square_a1-1;

    result = attack(slices[si].next1,n);

    if (CondFlag[supercirce] || isapril[abs(pi_captured)])
    {
      if (!post_move_iteration_locked[nbply])
        advance_rebirth_square();
    }
  }
  else
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    circe_do_rebirth(sq_rebirth,pi_captured,pprispec[nbply]);
    result = attack(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);

    if (!post_move_iteration_locked[nbply])
      advance_rebirth_square();
  }

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
stip_length_type supercirce_rebirth_handler_defend(slice_index si,
                                                   stip_length_type n)
{
  square const pi_captured = pprise[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    current_circe_rebirth_square[nbply] = square_a1-1;

    result = attack(slices[si].next1,n);

    if (CondFlag[supercirce] || isapril[abs(pi_captured)])
    {
      if (!post_move_iteration_locked[nbply])
        advance_rebirth_square();
    }
  }
  else
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    circe_do_rebirth(sq_rebirth,pi_captured,pprispec[nbply]);
    result = attack(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);

    if (!post_move_iteration_locked[nbply])
      advance_rebirth_square();
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STSuperCirceRebirthHandler);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_supercirce_rebirth_handlers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STIsardamDefenderFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
