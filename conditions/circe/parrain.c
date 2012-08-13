#include "conditions/circe/parrain.h"
#include "pydata.h"
#include "conditions/circe/rebirth_handler.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static square do_rebirth(void)
{
  square sq_rebirth;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (pprise[parent_ply[nbply]]==vide)
    sq_rebirth = initsquare;
  else
  {
    numecoup const coup_id = current_move[nbply];
    piece const pi_captured = pprise[parent_ply[nbply]];
    Flags spec_captured = pprispec[parent_ply[nbply]];

    if (CondFlag[parrain]) {
      sq_rebirth = (move_generation_stack[current_move[parent_ply[nbply]]].capture
                                   + move_generation_stack[coup_id].arrival
                                   - move_generation_stack[coup_id].departure);
    }
    if (CondFlag[contraparrain]) {
      sq_rebirth = (move_generation_stack[current_move[parent_ply[nbply]]].capture
                                   - move_generation_stack[coup_id].arrival
                                   + move_generation_stack[coup_id].departure);
    }

    if (e[sq_rebirth]==vide)
    {
      ren_parrain[nbply] = pi_captured;
      e[sq_rebirth] = pi_captured;
      ++nbpiece[pi_captured];
      spec[sq_rebirth] = spec_captured;
    }
    else
      sq_rebirth = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceSquare(sq_rebirth);
  TraceFunctionResultEnd();
  return sq_rebirth;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_parrain_rebirth_handler_attack(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = do_rebirth();
    if (sq_rebirth==initsquare)
      result = attack(slices[si].next1,n);
    else
    {
      current_circe_rebirth_square[nbply] = sq_rebirth;
      result = attack(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);
      current_circe_rebirth_square[nbply] = initsquare;
    }
  }

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
stip_length_type circe_parrain_rebirth_handler_defend(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = do_rebirth();
    if (sq_rebirth==initsquare)
      result = defend(slices[si].next1,n);
    else
    {
      current_circe_rebirth_square[nbply] = sq_rebirth;
      result = defend(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);
      current_circe_rebirth_square[nbply] = initsquare;
    }
  }

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
    slice_index const prototype = alloc_pipe(STCirceParrainRebirthHandler);
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

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_parrain_rebirth_handlers(slice_index si)
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
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
