#include "pyreflxg.h"
#include "pydirect.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 * @return true iff slice si has been successfully traversed
 */
boolean reflex_filter_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);
  proxy_slice_resolve(&slices[si].u.reflex_guard.avoided);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface attacker_filter **************
 */

/* Allocate a STReflexAttackerFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_attacker_filter(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexAttackerFilter,
                             length,min_length,
                             proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean reflex_attacker_filter_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(next,st);

  if (length-slack_length_direct==1)
  {
    pipe_link(si,*root);
    *root = si;
  }
  else
  {
    slice_index const guard = copy_slice(si);
    pipe_link(guard,*root);
    *root = guard;

    slices[si].u.branch.length -= 2;
    if (slices[si].u.branch.min_length-slack_length_direct>2)
      slices[si].u.branch.min_length -= 2;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type reflex_attacker_filter_has_solution_in_n(slice_index si,
                                                          stip_length_type n,
                                                          stip_length_type n_min)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = 0;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = direct_has_solution_in_n(slices[si].u.pipe.next,n,n_min);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void
reflex_attacker_filter_direct_solve_continuations_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
reflex_attacker_filter_direct_solve_threats_in_n(table threats,
                                                 slice_index si,
                                                 stip_length_type n,
                                                 stip_length_type n_min)
{
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case has_solution:
      /* no threats to be found because of reflex obligations;
       * cf. issue 2843251 */
      result = n+2;
      break;

    case has_no_solution:
    {
      slice_index const next = slices[si].u.pipe.next;
      result = direct_solve_threats_in_n(threats,next,n,n_min);
      break;
    }

    case defender_self_check:
      /* must already have been dealt with in an earlier slice */
    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean
reflex_attacker_filter_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(slice_has_solution(slices[si].u.reflex_guard.avoided)
         ==has_no_solution);
  result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve slice that is to be avoided
 * @param avoided slice to be avoided
 * @return true iff >=1 solution was found
 */
static boolean solve_avoided(slice_index avoided)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",avoided);
  TraceFunctionParamListEnd();

  output_start_unsolvability_mode();
  result = slice_solve(avoided);
  output_end_unsolvability_mode();

  if (result)
    write_end_of_solution();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_attacker_filter_root_solve(slice_index si)
{
  boolean result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_avoided(avoided))
    result = false;
  else
    result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type reflex_attacker_filter_solve_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = 0;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = direct_solve_in_n(next,n,n_min);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean reflex_attacker_filter_make_setplay_slice(slice_index si,
                                                  struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  prod->sibling = si;
  traverse_slices(slices[si].u.pipe.next,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean reflex_attacker_filter_reduce_to_postkey_play(slice_index si,
                                                      struct slice_traversal *st)
{
  boolean result;
  slice_index *postkey_slice = st->param;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(next,st);

  if (*postkey_slice!=no_slice)
  {
    stip_length_type const length = slices[si].u.branch.length;
    if (length==slack_length_direct+1)
    {
      slice_index const avoided = slices[si].u.reflex_guard.avoided;
      dealloc_slice(branch_deallocate_to_fork(avoided));
    }
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface defender_filter **********
 */

/* Allocate a STReflexDefenderFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_defender_filter(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexDefenderFilter,
                             length,min_length,
                             proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean reflex_defender_filter_insert_root(slice_index si,
                                           slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(next,st);

  if (length-slack_length_direct==0)
  {
    pipe_link(si,*root);
    *root = si;
  }
  else
  {
    slice_index const guard = copy_slice(si);
    pipe_link(guard,*root);
    *root = guard;

    slices[si].u.branch.length -= 2;
    if (slices[si].u.branch.min_length-slack_length_direct>=2)
      slices[si].u.branch.min_length -= 2;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean reflex_defender_filter_root_defend(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.reflex_guard.min_length==slack_length_direct
      && slice_solve(slices[si].u.reflex_guard.avoided))
  {
    write_end_of_solution();
    result = false;
  }
  else if (slices[si].u.reflex_guard.length>slack_length_direct)
    result = direct_defender_root_defend(slices[si].u.pipe.next);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean reflex_defender_filter_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_direct)
    result = !slice_solve(avoided);
  else
    result = direct_defender_defend_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int reflex_defender_filter_can_defend_in_n(slice_index si,
                                                    stip_length_type n,
                                                    unsigned int max_result)
{
  unsigned int result = max_result+1;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.reflex_guard.min_length;
  stip_length_type const max_n_for_avoided = (length-min_length
                                              +slack_length_direct);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<=max_n_for_avoided)
    switch (slice_has_solution(avoided))
    {
      case has_solution:
        result = 0;
        break;

      case has_no_solution:
        if (n>slack_length_direct)
          result = direct_defender_can_defend_in_n(next,n,max_result);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_defender_filter_root_solve(slice_index si)
{
  boolean result;
  slice_index const length = slices[si].u.reflex_guard.length;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_avoided(avoided) || length==slack_length_direct)
    result = false;
  else
    result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean reflex_defender_filter_make_setplay_slice(slice_index si,
                                                  struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (prod->sibling==no_slice)
  {
    /* semi-reflex stipulation */
    prod->sibling = slices[si].prev;
    assert(slices[prod->sibling].type==STDirectRoot);
  }

  if (slices[si].u.branch.length==slack_length_direct)
  {
    slice_index const proxy_to_avoided = slices[si].u.reflex_guard.avoided;
    assert(slices[proxy_to_avoided].type==STProxy);
    prod->setplay_slice = slices[proxy_to_avoided].u.pipe.next;
  }
  else
    traverse_slices(slices[si].u.pipe.next,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean reflex_defender_filter_reduce_to_postkey_play(slice_index si,
                                                      struct slice_traversal *st)
{
  boolean const result = true;
  slice_index *postkey_slice = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (next==no_slice)
  {
    assert(avoided!=no_slice);
    *postkey_slice = avoided;
    dealloc_slice(si);
  }
  else
    *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface help_filter ************
 */

/* Allocate a STReflexHelpFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_help_filter(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexHelpFilter,
                             length,min_length,
                             proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean reflex_help_filter_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);

  {
    slice_index const guard = copy_slice(si);
    pipe_link(guard,*root);
    *root = guard;
  }

  slices[si].u.reflex_guard.length -=2;
  if (slices[si].u.reflex_guard.min_length-slack_length_help>=2)
    slices[si].u.reflex_guard.min_length -= 2;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_help_filter_root_solve(slice_index si)
{
  boolean result;
  slice_index const length = slices[si].u.reflex_guard.length;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_avoided(avoided) || length==slack_length_help)
    result = false;
  else
    result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_help_filter_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
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
boolean reflex_help_filter_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_help_filter_solve_threats_in_n(table threats,
                                           slice_index si,
                                           stip_length_type n)
{
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    help_solve_threats_in_n(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface series_filter ************
 */

/* Allocate a STReflexSeriesFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_series_filter(stip_length_type length,
                                              stip_length_type min_length,
                                              slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexSeriesFilter,
                             length,min_length,
                             proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean reflex_series_filter_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);

  {
    slice_index const guard = copy_slice(si);
    pipe_link(guard,*root);
    *root = guard;

    --slices[si].u.branch.length;
    if (slices[si].u.branch.min_length>slack_length_series)
      --slices[si].u.branch.min_length;
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
boolean reflex_series_filter_root_solve(slice_index si)
{
  boolean result;
  slice_index const length = slices[si].u.reflex_guard.length;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_avoided(avoided) || length==slack_length_series)
    result = false;
  else
    result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_series_filter_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
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
boolean reflex_series_filter_has_solution_in_n(slice_index si,
                                               stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_series_filter_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n)
{
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    series_solve_threats_in_n(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reflex_filter_impose_starter(slice_index si, slice_traversal *st)
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

/* In alternate play, insert a STReflexHelpFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static boolean reflex_guards_inserter_help(slice_index si,
                                           slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const proxy_to_avoided = param->to_be_avoided[length%2];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (next==no_slice)
  {
    /* end of "chain" (i.e. stipulation too short for closed loop) -
     * append an STReflexHelpFilter slice that switches to the next branch
     */
    slice_index const proxy_to_avoided = param->to_be_avoided[0];
    slice_index const guard = alloc_reflex_help_filter(length-1,min_length-1,
                                                       proxy_to_avoided);
    pipe_link(si,guard);
  }

  {
    slice_index const prev = slices[si].prev;
    slice_index const guard = alloc_reflex_help_filter(length,min_length,
                                                       proxy_to_avoided);
    pipe_link(prev,guard);
    pipe_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In battle play, insert a STReflexAttackFilter slice before a
 * slice where the reflex stipulation might force the side at the move
 * to reach the goal
 */
static boolean reflex_guards_inserter_attack(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (next==no_slice)
  {
    /* end of "chain" (i.e. stipulation too short for closed loop) -
     * append an STReflexAttackFilter slice that switches to the next branch
     */
    slice_index const proxy_to_avoided = param->to_be_avoided[0];
    slice_index const guard = alloc_reflex_defender_filter(length-1,
                                                           min_length-1,
                                                           proxy_to_avoided);
    pipe_link(si,guard);
  }

  {
    slice_index const proxy_to_avoided = param->to_be_avoided[length%2];
    slice_index const prev = slices[si].prev;
    slice_index const guard = alloc_reflex_attacker_filter(length,min_length,
                                                           proxy_to_avoided);
    pipe_link(prev,guard);
    pipe_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In battle play, insert a STReflexDefenseFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static boolean reflex_guards_inserter_defense(slice_index si,
                                              slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (next==no_slice)
  {
    /* end of "chain" (i.e. stipulation too short for closed loop) -
     * append an STReflexAttackFilter slice that switches to the next branch
     */
    slice_index const proxy_to_avoided = param->to_be_avoided[0];
    slice_index const guard = alloc_reflex_attacker_filter(length-1,
                                                           min_length-1,
                                                           proxy_to_avoided);
    pipe_link(si,guard);
  }

  {
    slice_index const proxy_to_avoided = param->to_be_avoided[length%2];
    slice_index const prev = slices[si].prev;
    slice_index const dguard = alloc_reflex_defender_filter(length,min_length,
                                                            proxy_to_avoided);
    pipe_link(prev,dguard);
    pipe_link(dguard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In series play, insert a STReflexSeriesFilter slice before a slice where
 * the reflex stipulation might force the side at the move to reach
 * the goal
 */
static boolean reflex_guards_inserter_series(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  slice_index const proxy_to_avoided = param->to_be_avoided[1];
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index const prev = slices[si].prev;
  slice_index guard;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  guard = alloc_reflex_series_filter(length,min_length,proxy_to_avoided);
  pipe_link(prev,guard);
  pipe_link(guard,si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Prevent STReflex* slice insertion from recursing into the following
 * branch
 */
static boolean reflex_guards_inserter_branch_fork(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .avoided! */
  traverse_slices(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static slice_operation const reflex_guards_inserters[] =
{
  &slice_traverse_children,            /* STProxy */
  &reflex_guards_inserter_attack,      /* STBranchDirect */
  &reflex_guards_inserter_defense,     /* STBranchDirectDefender */
  &reflex_guards_inserter_help,        /* STBranchHelp */
  &reflex_guards_inserter_branch_fork, /* STHelpFork */
  &reflex_guards_inserter_series,      /* STBranchSeries */
  &reflex_guards_inserter_branch_fork, /* STSeriesFork */
  &slice_operation_noop,               /* STLeafDirect */
  &slice_operation_noop,               /* STLeafHelp */
  &slice_operation_noop,               /* STLeafForced */
  &slice_traverse_children,            /* STReciprocal */
  &slice_traverse_children,            /* STQuodlibet */
  &slice_traverse_children,            /* STNot */
  &slice_traverse_children,            /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,            /* STMoveInverterSolvableFilter */
  &slice_traverse_children,            /* STMoveInverterSeriesFilter */
  &slice_traverse_children,            /* STDirectRoot */
  &slice_traverse_children,            /* STDirectDefenderRoot */
  &slice_traverse_children,            /* STDirectHashed */
  &slice_traverse_children,            /* STHelpRoot */
  &slice_traverse_children,            /* STHelpShortcut */
  &slice_traverse_children,            /* STHelpHashed */
  &slice_traverse_children,            /* STSeriesRoot */
  &slice_traverse_children,            /* STSeriesShortcut */
  &slice_traverse_children,            /* STParryFork */
  &slice_traverse_children,            /* STSeriesHashed */
  &slice_traverse_children,            /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,            /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,            /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,            /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,            /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,            /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,            /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,            /* STDirectDefense */
  &slice_traverse_children,            /* STReflexHelpFilter */
  &slice_traverse_children,            /* STReflexSeriesFilter */
  &slice_traverse_children,            /* STReflexAttackerFilter */
  &slice_traverse_children,            /* STReflexDefenderFilter */
  &slice_traverse_children,            /* STSelfAttack */
  &slice_traverse_children,            /* STSelfDefense */
  &slice_traverse_children,            /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,            /* STRestartGuardHelpFilter */
  &slice_traverse_children,            /* STRestartGuardSeriesFilter */
  &slice_traverse_children,            /* STIntelligentHelpFilter */
  &slice_traverse_children,            /* STIntelligentSeriesFilter */
  &slice_traverse_children,            /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,            /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,            /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,            /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,            /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,            /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,            /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,            /* STMaxFlightsquares */
  &slice_traverse_children,            /* STDegenerateTree */
  &slice_traverse_children,            /* STMaxNrNonTrivial */
  &slice_traverse_children,            /* STMaxThreatLength */
  &slice_traverse_children,            /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,            /* STMaxTimeDefenderFilter */
  &slice_traverse_children,            /* STMaxTimeHelpFilter */
  &slice_traverse_children             /* STMaxTimeSeriesFilter */
};

/* Instrument a branch with STReflex* slices for a (non-semi)
 * reflex stipulation 
 * @param si root of branch to be instrumented
 * @param avoided identifies branch that needs to be guarded from
 */
void slice_insert_reflex_filters(slice_index si, slice_index avoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",avoided);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  if (slices[avoided].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,avoided);
    avoided = proxy;
  }

  {
    slice_index const avoided_leaf = slices[avoided].u.pipe.next;
    Goal const avoided_goal = slices[avoided_leaf].u.leaf.goal;
    slice_index const direct_avoided = alloc_leaf_slice(STLeafDirect,
                                                        avoided_goal);

    param.to_be_avoided[0] = avoided;
    param.to_be_avoided[1] = alloc_proxy_slice();
    pipe_link(param.to_be_avoided[1],direct_avoided);

    slice_traversal_init(&st,&reflex_guards_inserters,&param);
    traverse_slices(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In battle play, insert a STReflexDefenderFilter slice before a
 * defense slice
 * @param si identifies defense slice
 * @param address of structure representing the traversal
 */
static boolean reflex_guards_inserter_defense_semi(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    init_param const * const param = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const proxy_to_avoided = param->to_be_avoided[length%2];
    slice_index const guard = alloc_reflex_defender_filter(length,min_length,
                                                           proxy_to_avoided);
    pipe_link(prev,guard);
    pipe_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In battle play, insert a STReflexAttackerFilter if we are at the end of the
 * branch
 * @param si identifies slice potentially at the end of the branch
 * @param st address of structure representing the traversal
 */
static boolean reflex_guards_inserter_attack_semi(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (length==slack_length_direct+1)
  {
    /* insert an STReflexAttackerFilter slice that switches to the next branch
     */
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const proxy_to_avoided = param->to_be_avoided[1-length%2];
    slice_index const guard = alloc_reflex_defender_filter(length-1,
                                                           min_length-1,
                                                           proxy_to_avoided);
    pipe_link(si,guard);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const reflex_guards_inserters_semi[] =
{
  &slice_traverse_children,             /* STProxy */
  &reflex_guards_inserter_attack_semi,  /* STBranchDirect */
  &reflex_guards_inserter_defense_semi, /* STBranchDirectDefender */
  &reflex_guards_inserter_help,         /* STBranchHelp */
  &reflex_guards_inserter_branch_fork,  /* STHelpFork */
  &slice_traverse_children,             /* STBranchSeries */
  &reflex_guards_inserter_branch_fork,  /* STSeriesFork */
  &slice_operation_noop,                /* STLeafDirect */
  &slice_operation_noop,                /* STLeafHelp */
  &slice_operation_noop,                /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  &slice_traverse_children,             /* STNot */
  &slice_traverse_children,             /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,             /* STMoveInverterSolvableFilter */
  &slice_traverse_children,             /* STMoveInverterSeriesFilter */
  &slice_traverse_children,             /* STDirectRoot */
  &slice_traverse_children,             /* STDirectDefenderRoot */
  &slice_traverse_children,             /* STDirectHashed */
  &slice_traverse_children,             /* STHelpRoot */
  &slice_traverse_children,             /* STHelpShortcut */
  &slice_traverse_children,             /* STHelpHashed */
  &slice_traverse_children,             /* STSeriesRoot */
  &slice_traverse_children,             /* STSeriesShortcut */
  &slice_traverse_children,             /* STParryFork */
  &slice_traverse_children,             /* STSeriesHashed */
  &slice_traverse_children,             /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,             /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,             /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,             /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,             /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,             /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,             /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,             /* STDirectDefense */
  &slice_traverse_children,             /* STReflexHelpFilter */
  &slice_traverse_children,             /* STReflexSeriesFilter */
  &slice_traverse_children,             /* STReflexAttackerFilter */
  &slice_traverse_children,             /* STReflexDefenderFilter */
  &slice_traverse_children,             /* STSelfAttack */
  &slice_traverse_children,             /* STSelfDefense */
  &slice_traverse_children,             /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,             /* STRestartGuardHelpFilter */
  &slice_traverse_children,             /* STRestartGuardSeriesFilter */
  &slice_traverse_children,             /* STIntelligentHelpFilter */
  &slice_traverse_children,             /* STIntelligentSeriesFilter */
  &slice_traverse_children,             /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,             /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,             /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,             /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,             /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,             /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,             /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,             /* STMaxFlightsquares */
  &slice_traverse_children,             /* STDegenerateTree */
  &slice_traverse_children,             /* STMaxNrNonTrivial */
  &slice_traverse_children,             /* STMaxThreatLength */
  &slice_traverse_children,             /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,             /* STMaxTimeDefenderFilter */
  &slice_traverse_children,             /* STMaxTimeHelpFilter */
  &slice_traverse_children              /* STMaxTimeSeriesFilter */
};

/* Instrument a branch with STReflex* slices for a semi-reflex
 * stipulation 
 * @param si root of branch to be instrumented
 * @param avoided identifies branch that needs to be guarded from
 */
void slice_insert_reflex_filters_semi(slice_index si, slice_index avoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",avoided);
  TraceFunctionParamListEnd();

  if (slices[avoided].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,avoided);
    avoided = proxy;
  }

  param.to_be_avoided[0] = avoided;
  param.to_be_avoided[1] = no_slice;

  slice_traversal_init(&st,&reflex_guards_inserters_semi,&param);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
