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
#include "stipulation/help_play/play.h"
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
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type defense_root_defend(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min,
                                     unsigned int max_nr_refutations)
{
  stip_length_type result = n+4;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STContinuationWriter:
      result = continuation_writer_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STRefutationsWriter:
      result = refutations_writer_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_root_defend(si,
                                                  n,n_min,
                                                  max_nr_refutations);
      break;

    case STThreatWriter:
      result = threat_writer_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STDefenseMove:
      result = defense_move_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_root_defend(si,
                                                  n,n_min,
                                                  max_nr_refutations);
      break;

    case STSelfCheckGuardRootDefenderFilter:
      result = selfcheck_guard_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STKeepMatingGuardRootDefenderFilter:
      result = keepmating_guard_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_defend(si,
                                                 n,n_min,
                                                 max_nr_refutations);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_root_defend(si,
                                                   n,n_min,
                                                   max_nr_refutations);
      break;

    case STRestartGuardRootDefenderFilter:
      result = restart_guard_root_defend(si,n,n_min,max_nr_refutations);
      break;

    case STMaxTimeRootDefenderFilter:
      result = maxtime_root_defender_filter_defend(si,
                                                   n,n_min,
                                                   max_nr_refutations);
      break;

    case STMaxSolutionsRootDefenderFilter:
      result = maxsolutions_root_defender_filter_defend(si,
                                                        n,n_min,
                                                        max_nr_refutations);
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
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean defense_defend_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_min)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STContinuationWriter:
      result = continuation_writer_defend_in_n(si,n,n_min);
      break;

    case STThreatWriter:
      result = threat_writer_defend_in_n(si,n,n_min);
      break;

    case STDefenseMove:
      result = defense_move_defend_in_n(si,n,n_min);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_defend_in_n(si,n,n_min);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_defend_in_n(si,n,n_min);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_defend_in_n(si,n,n_min);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend_in_n(si,n,n_min);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend_in_n(si,n,n_min);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend_in_n(si,n,n_min);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_defend_in_n(si,n,n_min);
      break;

    case STHelpMove:
    {
      stip_length_type const nhelp = n-slack_length_battle+slack_length_help;
      result = !help_solve_in_n(si,nhelp);
      break;
    }

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
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=max_nr_refutations refutations found
           n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type defense_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int max_nr_refutations)
{
  stip_length_type result = n+4;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STRefutationsWriter:
      result = refutations_writer_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STContinuationWriter:
      result = continuation_writer_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STThreatWriter:
      result = threat_writer_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STDefenseMove:
      result = defense_move_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend_in_n(si,n,max_nr_refutations);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_can_defend_in_n(si,n,max_nr_refutations);
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

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @return true iff the defender can defend
 */
boolean defense_defend(slice_index si)
{
  boolean result = true;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;
  stip_length_type nr_moves_needed;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = defense_can_defend_in_n(si,
                                            length,
                                            max_nr_allowed_refutations);
  if (nr_moves_needed<slack_length_battle)
    result = true;
  else if (nr_moves_needed<=length)
  {
    result = false;

    {
      boolean defend_result;
      if (nr_moves_needed>slack_length_battle && min_length<=slack_length_battle)
        min_length += 2;
      defend_result = defense_defend_in_n(si,length,min_length);
      assert(!defend_result);
    }
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @return true iff there is >=1 refutation
 */
boolean defense_can_defend(slice_index si)
{
  boolean result = true;
  stip_length_type const n = slices[si].u.branch.length;
  stip_length_type nr_moves_needed;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = defense_can_defend_in_n(si,n,max_nr_allowed_refutations);
  result = nr_moves_needed<slack_length_battle || nr_moves_needed>n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
