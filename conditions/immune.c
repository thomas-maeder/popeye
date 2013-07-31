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

static boolean is_capturee_immune(Side side_capturing,
                                  numecoup n,
                                  square sq_departure,
                                  square sq_arrival)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_capturing,"");
  TraceValue("%u",n);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  {
    square const sq_capture = move_generation_stack[n].capture;
    square const sq_rebirth = (*immunrenai)(get_walk_of_piece_on_square(sq_capture),spec[sq_capture],
                                            sq_capture,
                                            sq_departure,
                                            sq_arrival,
                                            side_capturing);
    result = sq_rebirth!=sq_departure && !is_square_empty(sq_rebirth);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Immune Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean immune_validate_observation(slice_index si,
                                    square sq_observer,
                                    square sq_landing)
{
  boolean result;
  Side const side_observing = trait[nbply];
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceFunctionParamListEnd();

  if (immune_is_rex_inclusive
      || sq_capture!=king_square[advers(side_observing)])
    result = !is_capturee_immune(side_observing,current_move[nbply],sq_observer,sq_landing);
  else
    result = true;

  if (result)
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_not_capture_of_immune(numecoup n,
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
    result = !is_capturee_immune(trait[nbply],n,sq_departure,sq_arrival);

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

  move_generator_filter_moves(&is_not_capture_of_immune);

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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
