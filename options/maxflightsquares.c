#include "options/maxflightsquares.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/conditional_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "position/position.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

static unsigned int max_nr_flights;

/* Reset the max flights setting to off
 */
void reset_max_flights(void)
{
  max_nr_flights = INT_MAX;
}

/* Read the requested max flight setting from a text token entered by
 * the user
 * @param textToken text token from which to read
 * @return true iff max flight setting was successfully read
 */
boolean read_max_flights(const char *textToken)
{
  boolean result;
  char *end;
  unsigned long const requested_max_nr_flights = strtoul(textToken,&end,10);

  if (textToken!=end && requested_max_nr_flights<=nr_squares_on_board)
  {
    max_nr_flights = (unsigned int)requested_max_nr_flights;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Retrieve the current max flights setting
 * @return current max flights setting
 *         UINT_MAX if max flights option is not active
 */
unsigned int get_max_flights(void)
{
  return max_nr_flights;
}

/* **************** Private helpers ***************
 */

static unsigned int number_flights_left;
static square save_rbn;

/* Determine whether the defending side has more flights than allowed
 * by the user.
 * @return true iff the defending side has too many flights.
 */
static boolean are_there_too_many_flights(slice_index si)
{
  boolean result;
  Side const fleeing = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(save_rbn==initsquare); /* is there already a check going on? */
  number_flights_left = max_nr_flights+1;
  save_rbn = being_solved.king_square[fleeing];

  result = conditional_pipe_solve_delegate(si)==previous_move_has_solved;

  save_rbn = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Initialisation ***************
 */

/* Initialise a STMaxFlightsquares slice
 * @return identifier of allocated slice
 */
static slice_index alloc_maxflight_guard_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const counter = alloc_help_branch(slack_length+1,
                                                  slack_length+1);
    slice_index const prototype = alloc_pipe(STFlightsquaresCounter);
    help_branch_insert_slices(counter,&prototype,1);
    result = alloc_conditional_pipe(STMaxFlightsquares,counter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

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
void maxflight_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,
                                 solve_nr_remaining>next_move_has_no_solution
                                 && are_there_too_many_flights(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean are_flights_exhausted(slice_index si)
{
  Side const fleeing = advers(SLICE_STARTER(si));
  Flags const mask = BIT(fleeing)|BIT(Royal);

  if (!TSTFULLFLAGMASK(being_solved.spec[save_rbn],mask))
  {
    assert(number_flights_left>0);
    --number_flights_left;
    if (number_flights_left==0)
      return true;
  }

  return false;
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
void flightsquares_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_solves_exactly_if(are_flights_exhausted(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STMaxFlightsquares slice before each defender slice
 * @param si identifier defender slice
 * @param st address of structure representing the traversal
 */
static void maxflight_guard_inserter(slice_index si,stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (SLICE_U(si).branch.length>slack_length)
  {
    slice_index const prototype = alloc_maxflight_guard_slice();
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STMaxFlightsquares slices
 * @param si identifies slice where to start
 */
void solving_insert_maxflight_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STReadyForDefense,
                                           &maxflight_guard_inserter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
