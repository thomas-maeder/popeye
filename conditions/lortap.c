#include "conditions/lortap.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

/* Validate an observation according to Lortap
 * @return true iff the observation is valid
 */
boolean lortap_validate_observation(slice_index si)
{
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  boolean result;
  boolean is_observer_supported;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]-1].capture = sq_observer;
  is_observer_supported = is_square_observed(&validate_observer);
  finply();

  result = (!is_observer_supported
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_not_supported_capture(numecoup n,
                                        square sq_departure,
                                        square sq_arrival,
                                        square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_capture))
    result = true;
  else
  {
    siblingply(trait[nbply]);
    current_move[nbply] = current_move[nbply-1]+1;
    move_generation_stack[current_move[nbply]-1].capture = sq_departure;
    move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;
    result = !is_square_observed(&validate_observer);
    finply();
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
stip_length_type lortap_remove_supported_captures_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_not_supported_capture);

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
    slice_index const prototype = alloc_pipe(STLortapRemoveSupportedCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Lortap
 * @param si identifies the root slice of the stipulation
 */
void lortap_initialise_solving(slice_index si)
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

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationLortap);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
