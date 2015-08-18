#include "options/maxthreatlength.h"
#include "stipulation/proxy.h"
#include "solving/machinery/slack_length.h"
#include "solving/pipe.h"
#include "solving/testing_pipe.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_played.h"
#include "stipulation/battle_play/branch.h"
#include "solving/machinery/solve.h"
#include "solving/check.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
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
  SLICE_NEXT2(result) = threat_start;

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
void maxthreatlength_guard_solve(slice_index si)
{
  stip_length_type const n_max = slack_length+2*max_len_threat;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,
                                 solve_nr_remaining>=n_max
                                 && !is_in_check(SLICE_STARTER(si))
                                 && n_max<testing_pipe_solve_delegate(si,n_max));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

static void insert_maxthreatlength_guard(slice_index si,
                                         stip_structure_traversal *st)
{
  stip_length_type const length = SLICE_U(si).branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length>=2*max_len_threat+slack_length)
  {
    boolean * const inserted = st->param;
    slice_index const threat_start = branch_find_slice(STMaxThreatLengthStart,
                                                       si,
                                                       st->context);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const dummy = alloc_pipe(STDummyMove);
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

  if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_pipe(STMaxThreatLengthStart);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor maxthreatlength_guards_inserters[] =
{
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
boolean solving_insert_maxthreatlength_guards(slice_index si)
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
  st.activity = stip_traversal_activity_testing;
  stip_traverse_structure(SLICE_TESTER(si),&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
