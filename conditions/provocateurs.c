#include "conditions/provocateurs.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Determine whether a capturer is provoked
 * @param n identifies the move
 * @return true iff the capturer is provoked
 */
static boolean is_piece_provoked_on(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = is_square_observed_general(advers(trait[nbply]),
                       move_generation_stack[n].departure,
                       EVALUATE(observer));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Provocation Chess
 * @return true iff the observation is valid
 */
boolean provocateurs_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_piece_provoked_on(CURRMOVE_OF_PLY(nbply)))
    result = pipe_validate_observation_recursive_delegate(si);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void provocateurs_remove_unobserved_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_piece_provoked_on);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STProvocateursRemoveUnobservedCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Provokation Chess
 * @param si identifies the root slice of the stipulation
 */
void provocateurs_initialise_solving(slice_index si)
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

  stip_instrument_observation_validation(si,nr_sides,STProvocateursRemoveUnobservedCaptures);
  stip_instrument_check_validation(si,nr_sides,STProvocateursRemoveUnobservedCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
