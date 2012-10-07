#include "options/maxthreatlength.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solve.h"
#include "solving/solving.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type max_len_threat;

/* Reset the max threats setting to off
 */
void reset_max_threat_length(void)
{
  max_len_threat = no_stip_length;
}

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
boolean read_max_threat_length(const char *textToken)
{
  boolean result;
  char *end;
  unsigned long const requested_max_threat_length = strtoul(textToken,&end,10);

  if (textToken!=end && requested_max_threat_length<=UINT_MAX)
  {
    max_len_threat = (stip_length_type)requested_max_threat_length;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Retrieve the current max threat length setting
 * @return current max threat length setting
 *         no_stip_length if max threats option is not active
 */
stip_length_type get_max_threat_length(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",max_len_threat);
  TraceFunctionResultEnd();
  return max_len_threat;
}

/* **************** Private helpers ***************
 */

/* **************** Initialisation ***************
 */

/* Allocate a STMaxThreatLength slice
 */
static slice_index alloc_maxthreatlength_guard(slice_index threat_start)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",threat_start);
  TraceFunctionParamListEnd();

  result = alloc_testing_pipe(STMaxThreatLength);
  slices[result].next2 = threat_start;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type maxthreatlength_guard_solve(slice_index si,
                                              stip_length_type n)
{
  slice_index const next = slices[si].next1;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* determining whether the solve move has delivered check is
     expensive, so let's try to avoid it */
  if (max_len_threat==0)
  {
    if (echecc(slices[si].starter))
      result = solve(next,n);
    else
      result = n+2;
  }
  else
  {
    stip_length_type const n_max = 2*(max_len_threat-1)+slack_length+2;
    if (n>=n_max)
    {
      if (echecc(slices[si].starter))
        result = solve(next,n);
      else if (n_max<solve(slices[si].next2,n_max))
        result = n+2;
      else
        result = solve(next,n);
    }
    else
      /* remainder of play is too short for max_len_threat to apply */
      result = solve(next,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

static void insert_maxthreatlength_guard(slice_index si,
                                         stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->activity==stip_traversal_activity_testing
      && length>=2*max_len_threat+slack_length)
  {
    boolean * const inserted = st->param;
    slice_index const threat_start = branch_find_slice(STMaxThreatLengthStart,
                                                       si,
                                                       stip_traversal_context_defense);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const dummy = alloc_dummy_move_slice();
    slice_index const played = alloc_defense_played_slice();
    slice_index const prototype = alloc_maxthreatlength_guard(proxy);
    assert(threat_start!=no_slice);
    pipe_link(proxy,dummy);
    pipe_link(dummy,played);
    link_to_branch(played,threat_start);
    defense_branch_insert_slices(si,&prototype,1);

    *inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_max_threat_length_start(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->activity==stip_traversal_activity_testing
      && st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_pipe(STMaxThreatLengthStart);
    attack_branch_insert_slices(si,&prototype,1);
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor maxthreatlength_guards_inserters[] =
{
  { STDefenseAdapter,  &insert_maxthreatlength_guard   },
  { STReadyForDefense, &insert_maxthreatlength_guard   },
  { STNotEndOfBranch,  &insert_max_threat_length_start }
};

enum
{
  nr_maxthreatlength_guards_inserters =
  (sizeof maxthreatlength_guards_inserters
   / sizeof maxthreatlength_guards_inserters[0])
};

/* Instrument stipulation with STMaxThreatLength slices
 * @param si identifies slice where to start
 */
boolean stip_insert_maxthreatlength_guards(slice_index si)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,
                                    maxthreatlength_guards_inserters,
                                    nr_maxthreatlength_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
