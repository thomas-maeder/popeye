#include "conditions/shielded_kings.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

static boolean is_not_king_captures_guarded_king(numecoup n)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[sq_observer],Royal) && TSTFLAG(being_solved.spec[sq_observee],Royal))
    result = !is_square_observed_general(advers(trait[nbply]),
                          move_generation_stack[n].capture,
                          EVALUATE(observer));
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Shielded Kings
 * @return true iff the observation is valid
 */
boolean shielded_kings_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_not_king_captures_guarded_king(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

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
void shielded_kings_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_not_king_captures_guarded_king);

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
    slice_index const prototype = alloc_pipe(STShieldedKingsRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise solving in Shielded kings
 * @param si identifies the root slice of the solving machinery
 */
void shielded_kings_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  // needed for combination with transmuting kings etc.
  stip_instrument_observation_validation(si,nr_sides,STShieldedKingsRemoveIllegalCaptures);

  stip_instrument_check_validation(si,nr_sides,STShieldedKingsRemoveIllegalCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
