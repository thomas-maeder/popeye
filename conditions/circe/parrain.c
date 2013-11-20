#include "conditions/circe/parrain.h"
#include "conditions/conditions.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static int move_vector(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply]<=top);

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      square const from = move_effect_journal[curr].u.piece_movement.from;
      square const to = move_effect_journal[curr].u.piece_movement.to;
      result += to-from;
    }

  if (CondFlag[contraparrain])
    result = -result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_parrain_determine_rebirth_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
  move_effect_journal_index_type const parent_capture = parent_base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (parent_capture>=move_effect_journal_base[parent+1] /* threat! */
      || move_effect_journal[parent_capture].type==move_effect_no_piece_removal)
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = initsquare;
    result = solve(slices[si].next1,n);
  }
  else
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = move_effect_journal[parent_capture].u.piece_removal.from+move_vector();
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk = move_effect_journal[parent_capture].u.piece_removal.removed;
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_spec = move_effect_journal[parent_capture].u.piece_removal.removedspec;
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_rebirth_no_choice;

    result = solve(slices[si].next1,n);
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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STCirceParrainDetermineRebirth),
        alloc_pipe(STCircePlaceReborn),
        alloc_pipe(STBeforePawnPromotion),
        alloc_pipe(STPawnPromoter),
        alloc_pipe(STLandingAfterPawnPromotion)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe Parrain
 * @param si identifies root slice of stipulation
 */
void circe_parrain_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
