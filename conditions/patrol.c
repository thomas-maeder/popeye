#include "conditions/patrol.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

enum
{
  idle,
  seeking_supporter_for_observer
} state;

static boolean is_supported(square sq_departure)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  /* the supporter of the piece on sq_departure doesn't need a supporter */
  state = seeking_supporter_for_observer;
  move_generation_stack[current_move[nbply]].capture = sq_departure;
  result = is_square_observed(&validate_observer);
  state = idle;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Patrol Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean patrol_validate_observation(slice_index si,
                                    square sq_observer,
                                    square sq_landing)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceFunctionParamListEnd();

  if (state==idle && TSTFLAG(spec[sq_observer],Patrol))
  {
    boolean is_observer_supported;

    siblingply(trait[nbply]);
    current_move[nbply] = current_move[nbply-1]+1;
    move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
    is_observer_supported = is_supported(sq_observer);
    finply();

    if (is_observer_supported)
      result = validate_observation_recursive(slices[si].next1,
                                              sq_observer,
                                              sq_landing);
    else
      result = false;
  }
  else
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Patrol Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean is_not_patrol_or_supported_capture(square sq_observer,
                                                  square sq_landing,
                                                  square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_observer],Patrol))
  {
    siblingply(trait[nbply]);
    current_move[nbply] = current_move[nbply-1]+1;
    move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
    result = is_supported(sq_observer);
    finply();
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_not_unsupported_patrol_capture(numecoup n,
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
    result = is_not_patrol_or_supported_capture(sq_departure,sq_arrival,sq_capture);

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
stip_length_type patrol_remove_unsupported_captures_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_not_unsupported_patrol_capture);

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
    slice_index const prototype = alloc_pipe(STPatrolRemoveUnsupportedCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void patrol_initialise_solving(slice_index si)
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

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationPatrol);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to Ultra-Patrol Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean ultrapatrol_validate_observation(slice_index si,
                                         square sq_observer,
                                         square sq_landing)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
  result = is_supported(sq_observer);
  finply();

  if (result)
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void ultrapatrol_generate_moves_for_piece(slice_index si,
                                          square sq_departure,
                                          PieNam p)
{
  boolean is_mover_supported;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
  is_mover_supported = is_supported(sq_departure);
  finply();

  if (is_mover_supported)
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Ultra-Patrol Chess
 * @param si identifies root slice of solving machinery
 */
void ultrapatrol_initialise_solving(slice_index si)
{
  solving_instrument_move_generation(si,nr_sides,STUltraPatrolMovesForPieceGenerator);
  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationUltraPatrol);
}
