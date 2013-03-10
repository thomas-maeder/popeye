#include "options/maxflightsquares.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>
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
  Side const fleeing = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(save_rbn==initsquare); /* is there already a check going on? */
  number_flights_left = max_nr_flights+1;
  save_rbn = king_square[fleeing];
  result = solve(slices[si].next2,length_unspecified)==next_move_has_solution;
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
stip_length_type maxflight_guard_solve(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].next1;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>slack_length+3 && are_there_too_many_flights(si))
    result = n+2;
  else
    result = solve(next,n);

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
stip_length_type flightsquares_counter_solve(slice_index si,
                                              stip_length_type n)
{
  unsigned int result = n+2;
  Side const fleeing = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (save_rbn!=king_square[fleeing])
  {
    assert(number_flights_left>0);
    --number_flights_left;
    if (number_flights_left==0)
      result = n;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  if (slices[si].u.branch.length>slack_length)
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
void stip_insert_maxflight_guards(slice_index si)
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
