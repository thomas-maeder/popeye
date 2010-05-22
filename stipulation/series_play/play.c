#include "stipulation/series_play/play.h"
#include "stipulation/help_play/play.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovein.h"
#include "pymovenb.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/parry_fork.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/shortcut.h"
#include "optimisations/intelligent/series_filter.h"
#include "optimisations/maxtime/series_filter.h"
#include "optimisations/maxsolutions/series_filter.h"
#include "optimisations/stoponshortsolutions/series_filter.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>


/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STSeriesShortcut:
      result = series_shortcut_solve_in_n(si,n);
      break;

    case STSeriesMove:
      result = series_move_solve_in_n(si,n);
      break;

    case STHelpMove:
    {
      stip_length_type const n_help = n-slack_length_series+slack_length_help;
      result = help_solve_in_n(si,n_help)==n_help ? n : n+1;
      break;
    }

    case STContinuationWriter:
    {
      stip_length_type const n_battle = (n+slack_length_battle
                                         -slack_length_series);
      stip_length_type const n_min = battle_branch_calc_n_min(si,n_battle);
      result = defense_defend_in_n(si,n_battle,n_min)<=n_battle ? n : n+1;
      break;
    }

    case STParryFork:
      result = parry_fork_solve_in_n(si,n);
      break;

    case STSeriesFork:
      result = series_fork_solve_in_n(si,n);
      break;

    case STSeriesHashed:
      result = hashed_series_solve_in_n(si,n);
      break;

    case STMoveInverterSeriesFilter:
      result = move_inverter_series_solve_in_n(si,n);
      break;

    case STReflexSeriesFilter:
      result = reflex_series_filter_solve_in_n(si,n);
      break;

    case STKeepMatingGuardSeriesFilter:
      result = keepmating_guard_series_solve_in_n(si,n);
      break;

    case STIntelligentSeriesFilter:
      result = intelligent_series_filter_solve_in_n(si,n);
      break;

    case STGoalReachableGuardSeriesFilter:
      result = goalreachable_guard_series_solve_in_n(si,n);
      break;

    case STSelfCheckGuardSeriesFilter:
      result = selfcheck_guard_series_solve_in_n(si,n);
      break;

    case STRestartGuardSeriesFilter:
      result = restart_guard_series_solve_in_n(si,n);
      break;

    case STMaxTimeSeriesFilter:
      result = maxtime_series_filter_solve_in_n(si,n);
      break;

    case STMaxSolutionsSeriesFilter:
      result = maxsolutions_series_filter_solve_in_n(si,n);
      break;

    case STStopOnShortSolutionsSeriesFilter:
      result = stoponshortsolutions_series_filter_solve_in_n(si,n);
      break;

    default:
      assert(0);
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
boolean series_root_solve(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STSeriesRoot:
      result = series_root_root_solve(si);
      break;

    case STReflexSeriesFilter:
      result = reflex_series_filter_root_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_series);

  while (len<=full_length)
    if (series_solve_in_n(si,len)==len)
    {
      result = true;
      break;
    }
    else
      ++len;

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
stip_length_type series_has_solution_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STSeriesMove:
      result = series_move_has_solution_in_n(si,n);
      break;

    case STSeriesShortcut:
      result = series_shortcut_has_solution_in_n(si,n);
      break;

    case STContinuationWriter:
    {
      unsigned int const max_nr_allowed_refutations = 0;
      stip_length_type const n_battle = (n+slack_length_battle
                                         -slack_length_series);
      stip_length_type const n_min = battle_branch_calc_n_min(si,n_battle);
      stip_length_type const
          nr_moves_needed = defense_can_defend_in_n(si,
                                                    n_battle,n_min,
                                                    max_nr_allowed_refutations);
      if (nr_moves_needed>n_battle || nr_moves_needed<n_min)
        result = n+1;
      else
        result = nr_moves_needed+slack_length_series-slack_length_battle;
      break;
    }

    case STParryFork:
      result = parry_fork_has_solution_in_n(si,n);
      break;

    case STSeriesFork:
      result = series_fork_has_solution_in_n(si,n);
      break;

    case STSeriesHashed:
      result = hashed_series_has_solution_in_n(si,n);
      break;

    case STReflexSeriesFilter:
      result = reflex_series_filter_has_solution_in_n(si,n);
      break;

    case STKeepMatingGuardSeriesFilter:
      result = keepmating_guard_series_has_solution_in_n(si,n);
      break;

    case STGoalReachableGuardSeriesFilter:
      result = goalreachable_guard_series_has_solution_in_n(si,n);
      break;

    case STSelfCheckGuardSeriesFilter:
      result = selfcheck_guard_series_has_solution_in_n(si,n);
      break;

    case STMoveInverterSeriesFilter:
      result = move_inverter_series_has_solution_in_n(si,n);
      break;

    case STMaxTimeSeriesFilter:
      result = maxtime_series_filter_has_solution_in_n(si,n);
      break;

    case STMaxSolutionsSeriesFilter:
      result = maxsolutions_series_filter_has_solution_in_n(si,n);
      break;

    case STStopOnShortSolutionsSeriesFilter:
      result = stoponshortsolutions_series_filter_has_solution_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length)
    if (series_has_solution_in_n(si,len)==len)
    {
      result = has_solution;
      break;
    }
    else
      ++len;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
