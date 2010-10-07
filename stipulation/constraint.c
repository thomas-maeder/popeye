#include "pyreflxg.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STReflexRootFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_root_filter(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexRootFilter,0,0,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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


/* **************** Implementation of interface attacker_filter **************
 */

/* Allocate a STReflexAttackerFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
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

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_attacker_filter_make_root(slice_index si,
                                      stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index guard;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(avoided,st);
  guard = alloc_reflex_root_filter(state->result);

  stip_traverse_structure_pipe(si,st);
  pipe_link(guard,state->result);
  state->result = guard;

  if (slices[si].u.pipe.next==no_slice)
  {
    if (slices[si].prev!=no_slice)
      pipe_unlink(slices[si].prev);
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a reflex attacker filter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_reflex_attack_filter(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining==slices[si].u.reflex_guard.length)
    stip_traverse_moves_branch(slices[si].u.reflex_guard.avoided,st);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attacker_filter_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = attack_has_solution_in_n(next,n,n_max_unsolvable);
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

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
has_solution_type reflex_root_filter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_solve(avoided)==has_solution)
    result = slice_solve(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attacker_filter_solve_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = attack_solve_in_n(next,n,n_max_unsolvable);
      break;

    case has_no_solution:
      if (n==slack_length_battle)
        slice_solve(avoided);
      result = n+2;
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

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_root_filter_reduce_to_postkey_play(slice_index si,
                                               stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (*postkey_slice!=no_slice)
  {
    dealloc_slices(avoided);
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse the moves beyond a reflex root filter
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_reflex_root_filter(slice_index si,
                                            stip_moves_traversal *st)
{
  stip_traverse_moves_pipe(si,st);
  stip_traverse_moves(slices[si].u.reflex_guard.avoided,st);
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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
reflex_defender_filter_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.reflex_guard.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.branch_fork.length;
  stip_length_type const min_length = slices[si].u.branch_fork.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<=slack_length_battle
      && length-min_length+1>=n-slack_length_battle
      && slice_solve(avoided)==has_no_solution)
    result = slack_length_battle+1;
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
reflex_defender_filter_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.branch_fork.length;
  stip_length_type const min_length = slices[si].u.branch_fork.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<=slack_length_battle
      && length-min_length+1>=n-slack_length_battle
      && slice_has_solution(avoided)==has_no_solution)
    result = slack_length_battle+1;
  else
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void
reflex_guard_defender_filter_make_setplay_slice(slice_index si,
                                                stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.reflex_guard.length;
  stip_length_type const length_h = (length-slack_length_battle
                                     +slack_length_help);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const guard = alloc_reflex_help_filter(length_h,length_h,
                                                       avoided);
    pipe_link(guard,*result);
    *result = guard;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void reflex_defender_filter_apply_setplay(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const avoided = slices[si].u.reflex_guard.avoided;
    slice_index const length = slices[si].u.reflex_guard.length;
    stip_length_type const length_h = (length-slack_length_battle
                                       +slack_length_help);
    slice_index const min_length = slices[si].u.reflex_guard.min_length;
    stip_length_type const min_length_h = (min_length-slack_length_battle
                                           +slack_length_help);
    slice_index const filter = alloc_reflex_help_filter(length_h,min_length_h,
                                                        avoided);
    pipe_link(filter,*setplay_slice);
    *setplay_slice = filter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_defender_filter_reduce_to_postkey_play(slice_index si,
                                                   stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface help_filter ************
 */

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_help_filter_make_root(slice_index si, stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index guard;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  guard = alloc_reflex_root_filter(avoided);

  stip_traverse_structure_pipe(si,st);
  pipe_link(guard,state->result);
  state->result = guard;

  if (slices[si].u.pipe.next==no_slice)
  {
    if (slices[si].prev!=no_slice)
      pipe_unlink(slices[si].prev);
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type reflex_help_filter_solve_in_n(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = help_solve_in_n(next,n);
  else
    result = n+2;

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
stip_length_type reflex_help_filter_has_solution_in_n(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = help_has_solution_in_n(next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_series_filter_make_root(slice_index si,
                                      stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const guard = alloc_reflex_root_filter(avoided);
    pipe_link(guard,state->result);
    state->result = guard;
  }

  if (slices[si].u.pipe.next==no_slice)
  {
    if (slices[si].prev!=no_slice)
      pipe_unlink(slices[si].prev);
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a reflex attacker filter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_reflex_series_filter(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining==slack_length_series)
    stip_traverse_moves_branch(slices[si].u.reflex_guard.avoided,st);
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type reflex_series_filter_solve_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = series_solve_in_n(next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type reflex_series_filter_has_solution_in_n(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = series_has_solution_in_n(next,n);
  else
    result = n+1;

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

/* In battle play, insert a STReflexAttackFilter slice before a
 * slice where the reflex stipulation might force the side at the move
 * to reach the goal
 */
static void reflex_guards_inserter_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  init_param * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const idx = (length-slack_length_battle-1)%2;
    slice_index const proxy_to_avoided = param->to_be_avoided[idx];
    pipe_append(slices[si].prev,
                alloc_reflex_attacker_filter(length,min_length,
                                             proxy_to_avoided));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In battle play, insert a STReflexDefenseFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static void reflex_guards_inserter_defense(slice_index si,
                                           stip_structure_traversal *st)
{
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const idx = (length-slack_length_battle-1)%2;
    slice_index const proxy_to_avoided = param->to_be_avoided[idx];
    pipe_append(slices[si].prev,
                alloc_reflex_defender_filter(length,min_length,
                                             proxy_to_avoided));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prevent STReflex* slice insertion from recursing into the following
 * branch
 */
static void reflex_guards_inserter_branch_fork(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .avoided! */
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static structure_traversers_visitors reflex_guards_inserters[] =
{
  { STHelpFork,                         &reflex_guards_inserter_branch_fork },
  { STSeriesFork,                       &reflex_guards_inserter_branch_fork },
  { STGoalReachedTester,                &stip_structure_visitor_noop        },
  { STGoalMateReachedTester,            &stip_structure_visitor_noop        },
  { STGoalStalemateReachedTester,       &stip_structure_visitor_noop        },
  { STGoalDoubleStalemateReachedTester, &stip_structure_visitor_noop        },
  { STGoalTargetReachedTester,          &stip_structure_visitor_noop        },
  { STGoalCheckReachedTester,           &stip_structure_visitor_noop        },
  { STGoalCaptureReachedTester,         &stip_structure_visitor_noop        },
  { STGoalSteingewinnReachedTester,     &stip_structure_visitor_noop        },
  { STReadyForAttack,                   &reflex_guards_inserter_attack      },
  { STReadyForDefense,                  &reflex_guards_inserter_defense     }
};

enum
{
  nr_reflex_guards_inserters = (sizeof reflex_guards_inserters
                                / sizeof reflex_guards_inserters[0])
};

/* Instrument a branch with STReflex* slices for a (non-semi)
 * reflex stipulation
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided_attack identifies branch that the
 *                                attacker attempts to avoid
 * @param proxy_to_avoided_defense identifies branch that the
 *                                 defender attempts to avoid
 */
void slice_insert_reflex_filters(slice_index si,
                                 slice_index proxy_to_avoided_attack,
                                 slice_index proxy_to_avoided_defense)
{
  stip_structure_traversal st;
  init_param param = { { proxy_to_avoided_defense, proxy_to_avoided_attack } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided_attack);
  TraceFunctionParam("%u",proxy_to_avoided_defense);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_avoided_attack].type==STProxy);
  assert(slices[proxy_to_avoided_defense].type==STProxy);

  stip_structure_traversal_init(&st,&param);
  stip_structure_traversal_override(&st,
                                    reflex_guards_inserters,
                                    nr_reflex_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In alternate play, insert a STReflexHelpFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static void reflex_guards_inserter_help(slice_index si,
                                        stip_structure_traversal *st)
{
  init_param * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const idx = (length-slack_length_help)%2;
  slice_index const proxy_to_avoided = param->to_be_avoided[idx];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  /* in general, only one side is bound to deliver reflexmate */
  if (proxy_to_avoided!=no_slice)
    pipe_append(slices[si].prev,
                alloc_reflex_help_filter(length,min_length,proxy_to_avoided));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In series play, insert a STReflexSeriesFilter slice before a slice where
 * the reflex stipulation might force the side at the move to reach
 * the goal
 */
static void reflex_guards_inserter_series(slice_index si,
                                          stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->to_be_avoided[0];
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(slices[si].prev,
              alloc_reflex_series_filter(length,min_length,proxy_to_avoided));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In battle play, insert a STReflexDefenderFilter slice before a
 * defense slice
 * @param si identifies defense slice
 * @param address of structure representing the traversal
 */
static void reflex_guards_inserter_defense_semi(slice_index si,
                                                stip_structure_traversal *st)
{
  init_param const * const param = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    stip_length_type const idx = (length-slack_length_battle-1)%2;
    pipe_append(slices[si].prev,
                alloc_reflex_defender_filter(length,min_length,
                                             param->to_be_avoided[idx]));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors reflex_guards_inserters_semi[] =
{
  { STReadyForHelpMove,                 &reflex_guards_inserter_help         },
  { STHelpFork,                         &reflex_guards_inserter_branch_fork  },
  { STReadyForSeriesMove,               &reflex_guards_inserter_series       },
  { STSeriesFork,                       &reflex_guards_inserter_branch_fork  },
  { STGoalReachedTester,                &stip_structure_visitor_noop         },
  { STGoalMateReachedTester,            &stip_structure_visitor_noop         },
  { STGoalStalemateReachedTester,       &stip_structure_visitor_noop         },
  { STGoalDoubleStalemateReachedTester, &stip_structure_visitor_noop         },
  { STGoalTargetReachedTester,          &stip_structure_visitor_noop         },
  { STGoalCheckReachedTester,           &stip_structure_visitor_noop         },
  { STGoalCaptureReachedTester,         &stip_structure_visitor_noop         },
  { STGoalSteingewinnReachedTester,     &stip_structure_visitor_noop         },
  { STReadyForDefense,                  &reflex_guards_inserter_defense_semi }
};

enum
{
  nr_reflex_guards_inserters_semi = (sizeof reflex_guards_inserters_semi
                                     / sizeof reflex_guards_inserters_semi[0])
};

/* Instrument a branch with STReflex* slices for a semi-reflex
 * stipulation
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies branch that needs to be guarded from
 */
void slice_insert_reflex_filters_semi(slice_index si,
                                      slice_index proxy_to_avoided)
{
  stip_structure_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_avoided].type==STProxy);

  param.to_be_avoided[0] = proxy_to_avoided;
  param.to_be_avoided[1] = no_slice;

  stip_structure_traversal_init(&st,&param);
  stip_structure_traversal_override(&st,
                                    reflex_guards_inserters_semi,
                                    nr_reflex_guards_inserters_semi);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_reflex_filter(slice_index branch,
                                           stip_structure_traversal *st)
{
  slice_index const avoided = slices[branch].u.reflex_guard.avoided;
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(avoided,st);
}
