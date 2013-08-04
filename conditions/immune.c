#include "conditions/immune.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

square (*immunrenai)(PieNam, Flags, square, square, square, Side);

boolean immune_is_rex_inclusive;

static boolean is_capturee_not_immune(numecoup n)
{
  Side const side_capturing = trait[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_departure = move_generation_stack[n].departure;
    square const sq_arrival = move_generation_stack[n].arrival;
    square const sq_capture = move_generation_stack[n].capture;
    square const sq_rebirth = (*immunrenai)(get_walk_of_piece_on_square(sq_capture),spec[sq_capture],
                                            sq_capture,
                                            sq_departure,
                                            sq_arrival,
                                            side_capturing);
    result = sq_rebirth==sq_departure || is_square_empty(sq_rebirth);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Immune Chess
 * @return true iff the observation is valid
 */
boolean immune_validate_observation(slice_index si)
{
  boolean result;
  Side const side_observing = trait[nbply];
  square const sq_capture = move_generation_stack[current_move[nbply]-1].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (immune_is_rex_inclusive
      || sq_capture!=king_square[advers(side_observing)])
    result = is_capturee_not_immune(current_move[nbply]-1);
  else
    result = true;

  if (result)
    result = validate_observation_recursive(slices[si].next1);

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
stip_length_type immune_remove_captures_of_immune_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(&is_capturee_not_immune);

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
    slice_index const prototype = alloc_pipe(STImmuneRemoveCapturesOfImmune);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Immune Chess
 * @param si identifies the root slice of the stipulation
 */
void immune_initialise_solving(slice_index si)
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

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationImmune);
  stip_instrument_check_validation(si,nr_sides,STValidatingObservationImmune);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
