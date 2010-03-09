#include "stipulation/battle_play/attack_play.h"
#include "pybrafrk.h"
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
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/variation.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean attack_are_threats_refuted_in_n(table threats,
                                        stip_length_type len_threat,
                                        slice_index si,
                                        stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",threats);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STRefutationsCollector:
      result = refutations_collector_are_threats_refuted_in_n(threats,
                                                              len_threat,
                                                              si,
                                                              n);
      break;

    case STVariationWriter:
      result = variation_writer_are_threats_refuted_in_n(threats,
                                                         len_threat,
                                                         si,
                                                         n);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_are_threats_refuted_in_n(threats,
                                                                  len_threat,
                                                                  si,
                                                                  n);
      break;

    case STNoShortVariations:
      result = no_short_variations_are_threats_refuted_in_n(threats,
                                                            len_threat,
                                                            si,
                                                            n);
      break;

    case STAttackMove:
      result = attack_move_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STAttackHashed:
      result = attack_hashed_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STDirectDefense:
      result = direct_defense_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STSelfDefense:
      result = self_defense_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_are_threats_refuted_in_n(threats,
                                                               len_threat,
                                                               si,
                                                               n);
      break;

    case STSelfCheckGuardAttackerFilter:
      result = selfcheck_guard_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STKeepMatingGuardAttackerFilter:
      result = keepmating_guard_are_threats_refuted_in_n(threats,
                                                         len_threat,
                                                         si,
                                                         n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STLeafDirect:
      assert(len_threat==slack_length_battle);
      assert(n==slack_length_battle+1);
      result = leaf_d_are_threats_refuted(threats,si);
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
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type attack_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_has_solution_in_n(si,n,n_min);
      break;

    case STRefutationsCollector:
      result = refutations_collector_has_solution_in_n(si,n,n_min);
      break;

    case STVariationWriter:
      result = variation_writer_has_solution_in_n(si,n,n_min);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_has_solution_in_n(si,n,n_min);
      break;

    case STNoShortVariations:
      result = no_short_variations_has_solution_in_n(si,n,n_min);
      break;

    case STAttackMove:
      result = attack_move_has_solution_in_n(si,n,n_min);
      break;

    case STAttackHashed:
      result = attack_hashed_has_solution_in_n(si,n,n_min);
      break;

    case STSeriesMove:
    case STSeriesHashed:
    case STSeriesFork:
    {
      stip_length_type const n_ser = n-slack_length_battle+slack_length_series;
      result = series_has_solution_in_n(si,n_ser) ? n : n+2;
      break;
    }

    case STDirectDefense:
      result = direct_defense_direct_has_solution_in_n(si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_direct_has_solution_in_n(si,n,n_min);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_has_solution_in_n(si,n,n_min);
      break;

    case STSelfCheckGuardAttackerFilter:
      result = selfcheck_guard_direct_has_solution_in_n(si,n,n_min);
      break;

    case STKeepMatingGuardAttackerFilter:
      result = keepmating_guard_direct_has_solution_in_n(si,n,n_min);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_has_solution_in_n(si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_battle+1);
      if (leaf_d_has_solution(si)==has_solution)
        result = n;
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
has_solution_type attack_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min;
  stip_length_type const parity = length%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length+min_length>slack_length_battle+length)
    n_min = length-(length-min_length);
  else
    n_min = slack_length_battle-parity;

  result = (attack_has_solution_in_n(si,length,n_min)<=length
            ? has_solution
            : has_no_solution);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type attack_solve_threats_in_n(table threats,
                                           slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STRefutationsCollector:
      result = refutations_collector_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STVariationWriter:
      result = variation_writer_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STNoShortVariations:
      result = no_short_variations_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STAttackMove:
      result = attack_move_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STAttackHashed:
      result = attack_hashed_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STDirectDefense:
      result = direct_defense_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_direct_solve_threats_in_n(threats,si,
                                                                n,n_min);
      break;

    case STSelfCheckGuardAttackerFilter:
      result = selfcheck_guard_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STKeepMatingGuardAttackerFilter:
      result = keepmating_guard_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_battle+1);
      leaf_d_solve_threats(threats,si);
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

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void attack_solve_threats(table threats, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STAttackHashed:
    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const parity = (length-slack_length_battle)%2;
      stip_length_type const n_min = slack_length_battle+2-parity;
      attack_hashed_solve_threats_in_n(threats,si,length,n_min);
      break;
    }

    case STLeafDirect:
      leaf_d_solve_threats(threats,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean attack_are_threats_refuted(table threats, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STAttackHashed:
    {
      stip_length_type const length = slices[si].u.branch.length;
      result = attack_are_threats_refuted_in_n(threats,slack_length_battle,
                                               si,length);
      break;
    }

    case STLeafDirect:
      result = leaf_d_are_threats_refuted(threats,si);
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
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_battle)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type attack_solve_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_solve_in_n(si,n,n_min);
      break;

    case STRefutationsCollector:
      result = refutations_collector_solve_in_n(si,n,n_min);
      break;

    case STVariationWriter:
      result = variation_writer_solve_in_n(si,n,n_min);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_solve_in_n(si,n,n_min);
      break;

    case STNoShortVariations:
      result = no_short_variations_solve_in_n(si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_battle+1);
      assert(n_min==slack_length_battle+1);
      result = leaf_d_solve(si) ? n : n+2;
      break;

    case STAttackMove:
      result = attack_move_solve_in_n(si,n,n_min);
      break;

    case STSeriesMove:
    case STSeriesHashed:
    case STSeriesFork:
    {
      stip_length_type const n_ser = n-slack_length_battle+slack_length_series;
      result = series_solve_in_n(si,n_ser);
      break;
    }

    case STAttackHashed:
      result = attack_hashed_solve_in_n(si,n,n_min);
      break;

    case STDirectDefense:
      result = direct_defense_solve_in_n(si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_solve_in_n(si,n,n_min);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_solve_in_n(si,n,n_min);
      break;

    case STSelfCheckGuardAttackerFilter:
      result = selfcheck_guard_solve_in_n(si,n,n_min);
      break;

    case STDegenerateTree:
      result = attack_solve_in_n(slices[si].u.pipe.next,n,n_min);
      break;

    case STKeepMatingGuardAttackerFilter:
      result = keepmating_guard_direct_solve_in_n(si,n,n_min);
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

/* Solve a slice - adapter for direct slices
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean attack_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STLeafDirect:
    {
      stip_length_type const length = slack_length_battle+1;
      stip_length_type const min_length = slack_length_battle+1;
      result = attack_solve_in_n(si,length,min_length)<=length;
      break;
    }

    case STSelfDefense:
      result = self_defense_solve(si);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_solve(si);
      break;

    case STVariationWriter:
      result = variation_writer_solve(si);
      break;

    default:
    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      result = attack_solve_in_n(si,length,min_length)<=length;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found and written
 */
boolean attack_root_solve_in_n(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_root_solve(si);
      break;

    case STAttackRoot:
      result = attack_root_root_solve(si);
      break;

    case STReflexRootSolvableFilter:
      result = reflex_attacker_filter_root_solve(si);
      break;

    case STAttackHashed:
      result = attack_root_solve(slices[si].u.pipe.next);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_solve(si);
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
boolean attack_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = attack_root_solve_in_n(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
