#include "stipulation/battle_play/defense_play.h"
#include "pydirctg.h"
#include "pyflight.h"
#include "pykeepmt.h"
#include "pymovenb.h"
#include "pynontrv.h"
#include "pyreflxg.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pythreat.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/help_play/root.h"
#include "optimisations/maxsolutions/root_defender_filter.h"
#include "optimisations/maxtime/root_defender_filter.h"
#include "optimisations/maxtime/defender_filter.h"
#include "trace.h"

#include <assert.h>

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean defense_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STPostKeyPlaySolutionWriter:
      result = postkey_solution_writer_root_solve(si);
      break;

    case STDirectDefenseRootSolvableFilter:
      result = direct_defense_root_solve(si);
      break;

    case STSelfAttack:
      result = self_attack_root_solve(si);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_root_solve(si);
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

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean defense_root_defend(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STContinuationWriter:
      result = continuation_writer_root_defend(si);
      break;

    case STTryWriter:
      result = try_writer_root_defend(si);
      break;

    case STThreatWriter:
      result = threat_writer_root_defend(si);
      break;

    case STDefenseMove:
      result = defense_move_root_defend(si);
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
boolean defense_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STContinuationWriter:
      result = continuation_writer_defend_in_n(si,n);
      break;

    case STThreatWriter:
      result = threat_writer_defend_in_n(si,n);
      break;

    case STDefenseMove:
      result = defense_move_defend_in_n(si,n);
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
unsigned int defense_can_defend_in_n(slice_index si,
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
    case STTryWriter:
      result = try_writer_can_defend_in_n(si,n,max_result);
      break;

    case STContinuationWriter:
      result = continuation_writer_can_defend_in_n(si,n,max_result);
      break;

    case STThreatWriter:
      result = threat_writer_can_defend_in_n(si,n,max_result);
      break;

    case STDefenseMove:
      result = defense_move_can_defend_in_n(si,n,max_result);
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
