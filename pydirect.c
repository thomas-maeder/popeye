#include "pydirect.h"
#include "pybradd.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyreflxg.h"
#include "pymovenb.h"
#include "pykeepmt.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "pyleafd.h"
#include "optimisations/maxtime/root_defender_filter.h"
#include "optimisations/maxtime/defender_filter.h"
#include "optimisations/maxsolutions/root_defender_filter.h"
#include "trace.h"

#include <assert.h>

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean direct_defender_root_defend(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_defend(si);
      break;

    case STSelfAttack:
      result = self_attack_root_defend(si);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_root_defend(si);
      break;

    case STSelfCheckGuardRootDefenderFilter:
      result = selfcheck_guard_root_defend(si);
      break;

    case STKeepMatingGuardRootDefenderFilter:
      result = keepmating_guard_root_defend(si);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_root_defend(si);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_defend(si);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_root_defend(si);
      break;

    case STRestartGuardRootDefenderFilter:
      result = restart_guard_root_defend(si);
      break;

    case STMaxTimeRootDefenderFilter:
      result = maxtime_root_defender_filter_defend(si);
      break;

    case STMaxSolutionsRootDefenderFilter:
      result = maxsolutions_root_defender_filter_defend(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean direct_defender_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_defend_in_n(si,n);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_defend_in_n(si,n);
      break;

    case STSelfAttack:
      result = self_attack_defend_in_n(si,n);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_defend_in_n(si,n);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_defend_in_n(si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend_in_n(si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend_in_n(si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend_in_n(si,n);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_defend_in_n(si,n);
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

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int direct_defender_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             unsigned int max_result)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_result);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot: /* case needed for nontrivial */
    case STBranchDirectDefender:
      result = branch_d_defender_can_defend_in_n(si,n,max_result);
      break;

    case STSelfAttack:
      result = self_attack_can_defend_in_n(si,n,max_result);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_can_defend_in_n(si,n,max_result);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_can_defend_in_n(si,n,max_result);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_can_defend_in_n(si,n,max_result);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend_in_n(si,n,max_result);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,n,max_result);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend_in_n(si,n,max_result);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_can_defend_in_n(si,n,max_result);
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
