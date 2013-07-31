#include "conditions/edgemover.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Validate the geometry of observation according to Edgemover
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean edgemover_validate_observation_geometry(slice_index si,
                                                square sq_observer,
                                                square sq_landing)
{
  boolean result;

  if (NoEdge(sq_landing))
    result = false;
  else
    result = validate_observation_geometry_recursive(slices[si].next1,
                                                     sq_observer,
                                                     sq_landing);

  return result;
}

static boolean does_not_go_to_the_edge(numecoup n,
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

  result = !NoEdge(sq_arrival);

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
stip_length_type edgemover_remove_illegal_moves_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&does_not_go_to_the_edge);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  boolean const (* const enabled)[nr_sides] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*enabled)[slices[si].starter])
  {
    slice_index const prototype = alloc_pipe(STEdgeMoverRemoveIllegalMoves);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_edgemover(slice_index si)
{
  stip_structure_traversal st;
  boolean const enabled[nr_sides] = { CondFlag[whiteedge], CondFlag[blackedge] };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_impose_starter(si,slices[si].starter);

  stip_structure_traversal_init(&st,&enabled);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  if (CondFlag[whiteedge])
    stip_instrument_observation_geometry_validation(si,
                                                    White,
                                                    STValidateObservationGeometryEdgeMover);
  if (CondFlag[blackedge])
    stip_instrument_observation_geometry_validation(si,
                                                    Black,
                                                    STValidateObservationGeometryEdgeMover);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
