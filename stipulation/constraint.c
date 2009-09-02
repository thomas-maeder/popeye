#include "pyreflxg.h"
#include "pydirect.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Initialise a STReflexGuard slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_be_avoided prototype of slice that must not be solvable
 */
static void init_reflex_guard_slice(slice_index si,
                                    stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index to_be_avoided,
                                    slice_index to_be_avoided_advers)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_be_avoided);
  TraceFunctionParamListEnd();

  slices[si].type = STReflexGuard; 
  slices[si].starter = no_side; 

  slices[si].u.pipe.u.reflex_guard.length = length;
  slices[si].u.pipe.u.reflex_guard.min_length = min_length;
  slices[si].u.pipe.u.reflex_guard.avoided = to_be_avoided;
  slices[si].u.pipe.u.reflex_guard.avoided_advers = to_be_avoided_advers;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
reflex_guard_direct_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      int curr_max_nr_nontrivial)
{
  has_solution_type result = has_no_solution;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = defender_self_check;
      break;

    case has_solution:
      result = has_no_solution;
      break;

    case has_no_solution:
      result = direct_has_solution_in_n(slices[si].u.pipe.next,
                                        n,
                                        curr_max_nr_nontrivial);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void reflex_guard_direct_solve_continuations_in_n(table continuations,
                                                  slice_index si,
                                                  stip_length_type n)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* TODO this switch is only necessary if
   * reflex_guard_direct_solve_continuations_in_n() is used for
   * finding threats or if we are solving postkey (otherwise
   * has_solution would make the defense just played a refutation).
   * Should we use separate operations for finding threats and regular
   * continuations?
   */
  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      /* must already have been dealt with in an earlier slice */
      assert(0);
      break;

    case has_solution:
      /* no continuations to be found because of reflex obligations;
       * cf. issue 2843251 */
      break;

    case has_no_solution:
      direct_solve_continuations_in_n(continuations,next,n);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean reflex_guard_are_threats_refuted_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n,
                                              int curr_max_nr_nontrivial)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(slice_has_solution(avoided)==has_no_solution);
  result = direct_are_threats_refuted_in_n(threats,
                                           len_threat,
                                           next,
                                           n,
                                           curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defender can successfully defend
 */
boolean reflex_guard_root_defend(slice_index si)
{
  boolean result = true;
  stip_length_type const length = slices[si].u.pipe.u.reflex_guard.length;
  stip_length_type const
      min_length = slices[si].u.pipe.u.reflex_guard.min_length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length==slack_length_direct)
    switch (slice_has_solution(avoided))
    {
      case defender_self_check:
        break;

      case has_solution:
        result = false;
        slice_solve_postkey(avoided);
        write_end_of_solution();
        break;

      case has_no_solution:
        if (length>slack_length_direct)
          result = direct_defender_root_defend(next);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_root_defend(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean reflex_guard_defend_in_n(slice_index si,
                                 stip_length_type n,
                                 int curr_max_nr_nontrivial)
{
  boolean result = true;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const
      min_length = slices[si].u.pipe.u.reflex_guard.min_length;
  stip_length_type const max_n_for_avoided = (length-min_length
                                              +slack_length_direct);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n<=max_n_for_avoided)
    switch (slice_has_solution(avoided))
    {
      case has_solution:
        result = false;
        slice_solve_postkey(avoided);
        break;

      case has_no_solution:
        if (n>slack_length_direct)
          result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean reflex_guard_can_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     int curr_max_nr_nontrivial)
{
  boolean result = true;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const
      min_length = slices[si].u.pipe.u.reflex_guard.min_length;
  stip_length_type const max_n_for_avoided = (length-min_length
                                              +slack_length_direct);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n<=max_n_for_avoided)
    switch (slice_has_solution(avoided))
    {
      case has_solution:
        result = false;
        break;

      case has_no_solution:
        if (n>slack_length_direct)
          result = direct_defender_can_defend_in_n(next,
                                                   n,
                                                   curr_max_nr_nontrivial);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_can_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
boolean reflex_guard_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_direct)
    result = slice_solve_postkey(avoided);
  else
    result = direct_defender_solve_postkey_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (n==slack_length_help)
    result = slice_solve(avoided);
  else if (slice_has_solution(avoided)==has_solution)
    result = false;
  else 
    result = help_solve_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
   result = false;
  else
    result = help_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_guard_help_solve_continuations_in_n(table continuations,
                                                slice_index si,
                                                stip_length_type n)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    help_solve_continuations_in_n(continuations,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = false;
  else
    result = series_solve_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_series_has_solution_in_n(slice_index si,
                                              stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = false;
  else
    result = series_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_guard_series_solve_continuations_in_n(table continuations,
                                                  slice_index si,
                                                  stip_length_type n)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    series_solve_continuations_in_n(continuations,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_root_solve(slice_index si)
{
  boolean result;
  slice_index const length = slices[si].u.pipe.u.reflex_guard.length;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = defender_self_check;
      break;

    case has_solution:
      init_output(si);
      slice_write_unsolvability(avoided);
      result = has_no_solution;
      break;

    case has_no_solution:
      if (length==slack_length_direct)
        result = has_no_solution;
      else
        result = slice_root_solve(next);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_solve(slice_index si)
{
  boolean result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = defender_self_check;
      break;

    case has_solution:
      result = has_no_solution;
      break;

    case has_no_solution:
      result = slice_solve(next);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_root_solve_postkey(table refutations, slice_index si)
{
  boolean result = false;
  stip_length_type const next = slices[si].u.pipe.next;
  stip_length_type const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.pipe.u.reflex_guard.length;
  stip_length_type const
      min_length = slices[si].u.pipe.u.reflex_guard.min_length;
  stip_length_type const max_n_for_avoided = (length-min_length
                                              +slack_length_direct);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length<=max_n_for_avoided)
    result = slice_solve_postkey(avoided);

  if (!result)
    result = slice_root_solve_postkey(refutations,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index reflex_guard_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_root_make_setplay_slice(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index reflex_guard_root_reduce_to_postkey_play(slice_index si)
{
  slice_index result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const length = slices[si].u.pipe.u.reflex_guard.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (next==no_slice)
  {
    assert(avoided!=no_slice);
    result = avoided;
    dealloc_slice_index(si);
  }
  else
  {
    result = slice_root_reduce_to_postkey_play(next);

    if (result!=no_slice)
    {
      if ((length-slack_length_direct)%2==1
          && length<=slack_length_direct+2)
        dealloc_slice_index(avoided);
      dealloc_slice_index(si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reflex_guard_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

typedef struct
{
    slice_index to_be_avoided[2];
} init_param;

/* In alternate play, insert a STReflexGuard slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static boolean reflex_guards_inserter_branch(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (length==slack_length_direct+1)
  {
    /* insert an STReflexGuard slice that switches to the next branch
     */
    pipe_insert_after(si);
    init_reflex_guard_slice(slices[si].u.pipe.next,
                            length-1,min_length-1,
                            param->to_be_avoided[1-length%2],
                            param->to_be_avoided[length%2]);
  }

  pipe_insert_before(si);
  init_reflex_guard_slice(si,
                          length,min_length,
                          param->to_be_avoided[length%2],
                          param->to_be_avoided[1-length%2]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In series play, insert a STReflexGuard slice before a slice where
 * the reflex stipulation might force the side at the move to reach
 * the goal
 */
static boolean reflex_guards_inserter_series(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_reflex_guard_slice(si,
                          length,min_length,
                          param->to_be_avoided[1],
                          param->to_be_avoided[0]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Prevent STReflexGuard slice insertion from recursing into the
 * following branch
 */
static boolean reflex_guards_inserter_branch_fork(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .towards_goal! */
  traverse_slices(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static slice_operation const reflex_guards_inserters[] =
{
  &reflex_guards_inserter_branch,      /* STBranchDirect */
  &reflex_guards_inserter_branch,      /* STBranchDirectDefender */
  &reflex_guards_inserter_branch,      /* STBranchHelp */
  &reflex_guards_inserter_series,      /* STBranchSeries */
  &reflex_guards_inserter_branch_fork, /* STBranchFork */
  &slice_operation_noop,               /* STLeafDirect */
  &slice_operation_noop,               /* STLeafHelp */
  &slice_operation_noop,               /* STLeafForced */
  &slice_traverse_children,            /* STReciprocal */
  &slice_traverse_children,            /* STQuodlibet */
  &slice_traverse_children,            /* STNot */
  &slice_traverse_children,            /* STMoveInverter */
  &reflex_guards_inserter_branch,      /* STDirectRoot */
  &reflex_guards_inserter_branch,      /* STDirectDefenderRoot */
  0,                                   /* STDirectHashed */
  &reflex_guards_inserter_branch,      /* STHelpRoot */
  &slice_traverse_children,            /* STHelpAdapter */
  0,                                   /* STHelpHashed */
  &reflex_guards_inserter_series,      /* STSeriesRoot */
  &slice_traverse_children,            /* STSeriesAdapter */
  0,                                   /* STSeriesHashed */
  0,                                   /* STSelfCheckGuard */
  0,                                   /* STDirectAttack */
  0,                                   /* STDirectDefense */
  0,                                   /* STReflexGuard */
  0,                                   /* STSelfAttack */
  0,                                   /* STSelfDefense */
  0,                                   /* STRestartGuard */
  0,                                   /* STGoalReachableGuard */
  0,                                   /* STKeepMatingGuard */
  0                                    /* STMaxFlightsquares */
};

/* Instrument a branch with STReflexGuard slices for a (non-semi)
 * reflex stipulation 
 * @param si root of branch to be instrumented
 * @param tobeavoided identifies what branch needs to be guarded from
 */
void slice_insert_reflex_guards(slice_index si, slice_index tobeavoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",tobeavoided);
  TraceFunctionParamListEnd();

  param.to_be_avoided[0] = tobeavoided;

  param.to_be_avoided[1] = copy_slice(tobeavoided);
  slices[param.to_be_avoided[1]].type = STLeafDirect;

  slice_traversal_init(&st,&reflex_guards_inserters,&param);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* In alternate play, insert a STReflexGuard slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static boolean reflex_guards_inserter_branch_semi(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (length==slack_length_direct+1)
  {
    /* insert an STReflexGuard slice that switches to the next branch
     */
    pipe_insert_after(si);
    init_reflex_guard_slice(slices[si].u.pipe.next,
                            length-1,min_length-1,
                            param->to_be_avoided[1-length%2],
                            param->to_be_avoided[length%2]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const reflex_guards_inserters_semi[] =
{
  &reflex_guards_inserter_branch_semi, /* STBranchDirect */
  &reflex_guards_inserter_branch,      /* STBranchDirectDefender */
  &reflex_guards_inserter_branch,      /* STBranchHelp */
  &slice_traverse_children,            /* STBranchSeries */
  &reflex_guards_inserter_branch_fork, /* STBranchFork */
  &slice_operation_noop,               /* STLeafDirect */
  &slice_operation_noop,               /* STLeafHelp */
  &slice_operation_noop,               /* STLeafForced */
  &slice_traverse_children,            /* STReciprocal */
  &slice_traverse_children,            /* STQuodlibet */
  &slice_traverse_children,            /* STNot */
  &slice_traverse_children,            /* STMoveInverter */
  &reflex_guards_inserter_branch_semi, /* STDirectRoot */
  &reflex_guards_inserter_branch,      /* STDirectDefenderRoot */
  0,                                   /* STDirectHashed */
  &slice_traverse_children,            /* STHelpRoot */
  &slice_traverse_children,            /* STHelpAdapter */
  0,                                   /* STHelpHashed */
  &slice_traverse_children,            /* STSeriesRoot */
  &slice_traverse_children,            /* STSeriesAdapter */
  0,                                   /* STSeriesHashed */
  0,                                   /* STSelfCheckGuard */
  0,                                   /* STDirectAttack */
  0,                                   /* STDirectDefense */
  0,                                   /* STReflexGuard */
  0,                                   /* STSelfAttack */
  0,                                   /* STSelfDefense */
  0,                                   /* STRestartGuard */
  0,                                   /* STGoalReachableGuard */
  0,                                   /* STKeepMatingGuard */
  0                                    /* STMaxFlightsquares */
};

/* Instrument a branch with STReflexGuard slices for a semi-reflex
 * stipulation 
 * @param si root of branch to be instrumented
 * @param tobeavoided identifies what branch needs to be guarded from
 */
void slice_insert_reflex_guards_semi(slice_index si, slice_index tobeavoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",tobeavoided);
  TraceFunctionParamListEnd();

  param.to_be_avoided[0] = tobeavoided;
  param.to_be_avoided[1] = no_slice;

  slice_traversal_init(&st,&reflex_guards_inserters_semi,&param);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
