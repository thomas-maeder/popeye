#include "stipulation/goals/countermate/filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCounterMateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_countermate_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCounterMateFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
countermate_attacker_filter_has_solution_in_n(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n_max_unsolvable<slack_length_battle+1
      && (goal_checker_mate(advers(starter))==goal_reached))
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = attack_has_solution_in_n(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
countermate_attacker_filter_solve_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n_max_unsolvable<slack_length_battle+1
      && (goal_checker_mate(advers(starter))==goal_reached))
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = attack_solve_in_n(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

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
countermate_defender_filter_can_defend_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n_max_unsolvable<slack_length_battle+1
      && (goal_checker_mate(advers(starter))==goal_reached))
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = defense_can_defend_in_n(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

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
countermate_defender_filter_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n_max_unsolvable<slack_length_battle+1
      && (goal_checker_mate(advers(starter))==goal_reached))
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = defense_defend_in_n(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type countermate_help_filter_solve_in_n(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (goal_checker_mate(advers(starter))==goal_reached)
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = help_solve_in_n(slices[si].u.pipe.next,slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type countermate_help_filter_has_solution_in_n(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (goal_checker_mate(advers(starter))==goal_reached)
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = help_has_solution_in_n(slices[si].u.pipe.next,
                                  slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type countermate_series_filter_solve_in_n(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_series+1);

  if (goal_checker_mate(advers(starter))==goal_reached)
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = series_solve_in_n(slices[si].u.pipe.next,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type
countermate_series_filter_has_solution_in_n(slice_index si,
                                            stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (goal_checker_mate(advers(starter))==goal_reached)
    SETFLAG(goal_preprequisites_met[nbply],goal_countermate);
  result = series_has_solution_in_n(slices[si].u.pipe.next,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_countermate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
