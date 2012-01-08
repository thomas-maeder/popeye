#include "pyflight.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "trace.h"

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
  result = slice_has_solution(slices[si].u.fork.fork)==has_solution;
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
    slice_index const counter = alloc_help_branch(slack_length_help+1,
                                                  slack_length_help+1);
    slice_index const prototype = alloc_pipe(STFlightsquaresCounter);
    help_branch_insert_slices(counter,&prototype,1);
    result = alloc_branch_fork(STMaxFlightsquares,counter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxflight_guard_defend(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>slack_length_battle+3 && are_there_too_many_flights(si))
    result = n+4;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxflight_guard_can_defend(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>slack_length_battle+3 && are_there_too_many_flights(si))
    result = n+4;
  else
    result = can_defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type flightsquares_counter_can_help(slice_index si,
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

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_maxflight_guard_slice();
    battle_branch_insert_slices(si,&prototype,1);
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
                                           STReadyForAttack,
                                           &maxflight_guard_inserter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
