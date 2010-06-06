#include "stipulation/help_play/play.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/series_play/play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/fork.h"
#include "pyleafh.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/help_filter.h"
#include "optimisations/maxtime/help_filter.h"
#include "optimisations/maxsolutions/help_filter.h"
#include "optimisations/stoponshortsolutions/help_filter.h"
#include "trace.h"

#include <assert.h>

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpShortcut:
      result = help_shortcut_solve_in_n(si,n);
      break;

    case STHelpMove:
      result = help_move_solve_in_n(si,n);
      break;

    case STSeriesFork:
    case STSeriesMove:
    case STSeriesHashed:
    case STSelfCheckGuardSeriesFilter:
    {
      stip_length_type const n_series = n-slack_length_help+slack_length_series;
      stip_length_type const length = series_solve_in_n(si,n_series);
      if (length==n_series+2)
        result = n+4;
      else if (length==n_series+1)
        result = n+2;
      else if (length==n_series)
        result = n;
      else
      {
        assert(length==n_series-1);
        result = n-2;
      }
      break;
    }

    case STThreatEnforcer:
    case STVariationWriter:
    case STSelfDefense:
    case STReflexAttackerFilter:
    case STSelfCheckGuardAttackerFilter:
    {
      stip_length_type const nbattle = n+slack_length_battle-slack_length_help;
      stip_length_type const parity = (nbattle-slack_length_battle)%2;
      stip_length_type const n_min = slack_length_battle+parity;
      stip_length_type const n_max_unsolvable = n_min-2;
      stip_length_type const
          sol_length = attack_has_solution_in_n(si,
                                                nbattle,n_min,
                                                n_max_unsolvable);
      if (sol_length<n_min)
        result = n+4;
      else if (sol_length<=nbattle)
      {
        result = n;
        attack_solve_in_n(si,nbattle,n_min,n_max_unsolvable);
      }
      else
        result = sol_length+slack_length_help-slack_length_battle;
      break;
    }

    case STReflexDefenderFilter:
    {
      stip_length_type const nbattle = (n-slack_length_help+
                                        slack_length_battle+1);
      stip_length_type const parity = (nbattle-slack_length_battle)%2;
      stip_length_type const n_min = slack_length_battle+parity;
      result = defense_defend_in_n(si,nbattle,n_min,n_min-2)<=nbattle ? n : n+2;
      break;
    }

    case STHelpFork:
      result = help_fork_solve_in_n(si,n);
      break;

    case STHelpHashed:
      result = hashed_help_solve_in_n(si,n);
      break;

    case STLeafHelp:
      assert(n==slack_length_help+1);
      switch (leaf_h_solve(si))
      {
        case has_solution:
          result = n;
          break;

        case has_no_solution:
          result = n+2;
          break;

        case opponent_self_check:
          result = n+4;
          break;
      }
      break;

    case STReflexHelpFilter:
      result = reflex_help_filter_solve_in_n(si,n);
      break;

    case STKeepMatingGuardHelpFilter:
      result = keepmating_guard_help_solve_in_n(si,n);
      break;

    case STIntelligentHelpFilter:
      result = intelligent_help_filter_solve_in_n(si,n);
      break;

    case STGoalReachableGuardHelpFilter:
      result = goalreachable_guard_help_solve_in_n(si,n);
      break;

    case STSelfCheckGuardHelpFilter:
      result = selfcheck_guard_help_solve_in_n(si,n);
      break;

    case STRestartGuardHelpFilter:
      result = restart_guard_help_solve_in_n(si,n);
      break;

    case STMaxTimeHelpFilter:
      result = maxtime_help_filter_solve_in_n(si,n);
      break;

    case STMaxSolutionsHelpFilter:
      result = maxsolutions_help_filter_solve_in_n(si,n);
      break;

    case STStopOnShortSolutionsHelpFilter:
      result = stoponshortsolutions_help_filter_solve_in_n(si,n);
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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_solve(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_help);

  while (len<=full_length)
    if (help_solve_in_n(si,len)==len)
    {
      result = has_solution;
      break;
    }
    else
      len += 2;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
stip_length_type help_has_solution_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpMove:
      result = help_move_has_solution_in_n(si,n);
      break;

    case STHelpRoot:
      result = help_root_has_solution_in_n(si,n);
      break;

    case STHelpShortcut:
      result = help_shortcut_has_solution_in_n(si,n);
      break;

    case STHelpFork:
      result = help_fork_has_solution_in_n(si,n);
      break;

    case STHelpHashed:
      result = hashed_help_has_solution_in_n(si,n);
      break;

    case STReflexHelpFilter:
      result = reflex_help_filter_has_solution_in_n(si,n);
      break;

    case STKeepMatingGuardHelpFilter:
      result = keepmating_guard_help_has_solution_in_n(si,n);
      break;

    case STGoalReachableGuardHelpFilter:
      result = goalreachable_guard_help_has_solution_in_n(si,n);
      break;

    case STSelfCheckGuardHelpFilter:
      result = selfcheck_guard_help_has_solution_in_n(si,n);
      break;

    case STMaxTimeHelpFilter:
      result = maxtime_help_filter_has_solution_in_n(si,n);
      break;

    case STMaxSolutionsHelpFilter:
      result = maxsolutions_help_filter_has_solution_in_n(si,n);
      break;

    case STStopOnShortSolutionsHelpFilter:
      result = stoponshortsolutions_help_filter_has_solution_in_n(si,n);
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
has_solution_type help_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length)
    if (help_has_solution_in_n(si,len)==len)
    {
      result = has_solution;
      break;
    }
    else
      len += 2;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
