#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "pyleaf.h"
#include "pyleafd.h"
#include "pyleaff.h"
#include "pyleafh.h"
#include "pybrafrk.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pymovein.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "pykeepmt.h"
#include "pypipe.h"
#include "optimisations/maxsolutions/root_solvable_filter.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"

#include <assert.h>
#include <stdlib.h>


#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(opponent_self_check),              \
    ENUMERATOR(has_no_solution),                \
    ENUMERATOR(has_solution)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_solve(si);
      break;

    case STLeafHelp:
      result = leaf_h_solve(si);
      break;

    case STQuodlibet:
      result = quodlibet_solve(si);
      break;

    case STAttackMove:
    case STAttackHashed:
    case STSelfDefense:
    case STSelfCheckGuardAttackerFilter:
    case STRefutingVariationWriter:
    case STReflexAttackerFilter:
    case STDegenerateTree:
    case STVariationWriter:
    case STLeafDirect:
      result = attack_solve(si);
      break;

    case STSelfCheckGuardDefenderFilter:
    case STContinuationWriter:
    case STDefenseMove:
      if (defense_can_defend(si))
        result = has_no_solution;
      else
      {
        boolean const defend_result = defense_defend(si);
        assert(!defend_result);
        result = has_solution;
      }
      break;

    case STHelpMove:
    case STHelpFork:
    case STHelpHashed:
    case STSelfCheckGuardHelpFilter:
    case STStopOnShortSolutionsHelpFilter:
      result = help_solve(si) ? has_solution : has_no_solution;
      break;

    case STSelfCheckGuardSeriesFilter:
    case STSeriesMove:
    case STSeriesFork:
    case STSeriesHashed:
    case STStopOnShortSolutionsSeriesFilter:
      result = series_solve(si) ? has_solution : has_no_solution;
      break;

    case STReciprocal:
      result = reci_solve(si);
      break;

    case STNot:
      result = not_solve(si);
      break;

    case STMoveInverterSolvableFilter:
      result = move_inverter_solve(si);
      break;

    case STSelfCheckGuardSolvableFilter:
      result = selfcheck_guard_solve(si);
      break;

    default:
      assert(0);
      result = has_no_solution;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_root_solve(si);
      break;

    case STReciprocal:
      result = reci_root_solve(si);
      break;

    case STNot:
      result = not_root_solve(si);
      break;

    case STAttackRoot:
    case STLeafDirect:
    case STAttackHashed:
    case STMaxThreatLength:
    case STReflexAttackerFilter:
    case STSelfCheckGuardAttackerFilter:
      result = attack_root_solve(si);
      break;

    case STSelfCheckGuardDefenderFilter:
    case STPostKeyPlaySolutionWriter:
    case STReflexDefenderFilter:
    case STContinuationWriter:
    case STThreatWriter:
      result = defense_root_solve(si);
      break;

    case STHelpRoot:
    case STReflexHelpFilter:
      result = help_root_solve(si);
      break;

    case STSeriesRoot:
    case STReflexSeriesFilter:
      result = series_root_solve(si);
      break;

    case STMoveInverterRootSolvableFilter:
      result = move_inverter_root_solve(si);
      break;

    case STSelfCheckGuardRootSolvableFilter:
      result = selfcheck_guard_root_solve(si);
      break;

    case STMaxSolutionsRootSolvableFilter:
      result = maxsolutions_root_solvable_filter_root_solve(si);
      break;

    case STStopOnShortSolutionsRootSolvableFilter:
      result = stoponshortsolutions_root_solvable_filter_root_solve(si);
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

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_solution(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_solution(si);
      break;

    case STLeafForced:
      result = leaf_forced_has_solution(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_solution(si);
      break;

    case STReciprocal:
      result = reci_has_solution(si);
      break;

    case STNot:
      result = not_has_solution(si);
      break;

    case STAttackRoot:
    case STAttackMove:
    case STSelfDefense:
    case STKeepMatingGuardAttackerFilter:
    case STKeepMatingGuardDefenderFilter:
    case STKeepMatingGuardHelpFilter:
    case STKeepMatingGuardSeriesFilter:
    case STDegenerateTree:
    case STAttackHashed:
    case STVariationWriter:
      result = attack_has_solution(si);
      break;

    case STContinuationWriter:
    case STDefenseMove:
    case STSelfCheckGuardDefenderFilter:
      result = defense_can_defend(si) ? has_no_solution : has_solution;
      break;

    case STHelpRoot:
    case STHelpMove:
    case STSelfCheckGuardHelpFilter:
      result = help_has_solution(si);
      break;

    case STHelpHashed:
      result = slice_has_solution(slices[si].u.pipe.next);
      break;

    case STMoveInverterSolvableFilter:
      result = move_inverter_has_solution(si);
      break;

    case STSeriesMove:
    case STSeriesHashed:
    case STReflexSeriesFilter:
      result = series_has_solution(si);

    case STHelpFork:
    case STSeriesFork:
      result = branch_fork_has_solution(si);
      break;

    case STSelfCheckGuardRootSolvableFilter:
    case STSelfCheckGuardSolvableFilter:
      result = selfcheck_guard_has_solution(si);
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
