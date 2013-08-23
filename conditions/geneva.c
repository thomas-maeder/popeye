#include "conditions/geneva.h"
#include "conditions/circe/circe.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

boolean rex_geneva;

static boolean is_capture_legal(numecoup n)
{
  boolean result;
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  Side const side_capturing = trait[nbply];
  Side const side_capturee = advers(side_capturing);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (rex_geneva || sq_departure!=king_square[side_capturing])
  {
    square const sq_rebirth = rennormal(get_walk_of_piece_on_square(sq_departure),spec[sq_departure],
                                        sq_departure,sq_departure,sq_arrival,
                                        side_capturee);
    result = is_square_empty(sq_rebirth);
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Geneva Chess
 * @return true iff the observation is valid
 */
boolean geneva_validate_observation(slice_index si)
{
  return (is_capture_legal(current_move[nbply]-1)
          && validate_observation_recursive(slices[si].next1));
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
stip_length_type geneva_remove_illegal_captures_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(&is_capture_legal);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STGenevaRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Geneva Chess
 * @param si identifies the root slice of the stipulation
 */
void geneva_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observation_validation(si,nr_sides,STGenevaRemoveIllegalCaptures);
  stip_instrument_check_validation(si,nr_sides,STGenevaRemoveIllegalCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
