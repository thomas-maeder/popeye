#include "pydirect.h"
#include "pybrad.h"
#include "pybradd.h"
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
#include "pyint.h"
#include "trace.h"

#include <assert.h>

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @param max_number_refutations maximum number of refutations to deliver
 * @return true iff the defending side can successfully defend
 */
boolean direct_defender_root_defend(table refutations,
                                    slice_index si,
                                    unsigned int max_number_refutations)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_defend(refutations,
                                             si,
                                             max_number_refutations);
      break;

    case STSelfAttack:
      result = self_attack_root_defend(refutations,
                                       si,
                                       max_number_refutations);
      break;

    case STReflexGuard:
      result = reflex_guard_root_defend(refutations,
                                        si,
                                        max_number_refutations);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_defend(refutations,
                                           si,
                                           max_number_refutations);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_root_defend(refutations,
                                            si,
                                            max_number_refutations);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_root_defend(refutations,
                                           si,
                                           max_number_refutations);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_defend(refutations,
                                                 si,
                                                 max_number_refutations);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_root_defend(refutations,
                                                   si,
                                                   max_number_refutations);
      break;

    case STRestartGuard:
      result = restart_guard_root_defend(refutations,
                                         si,
                                         max_number_refutations);
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

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_are_threats_refuted_in_n(table threats,
                                        stip_length_type len_threat,
                                        slice_index si,
                                        stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = branch_d_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STDirectHashed:
      result = direct_hashed_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STDirectDefense:
      result = direct_defense_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STSelfDefense:
      result = self_defense_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_are_threats_refuted_in_n(threats,
                                                         len_threat,
                                                         si,
                                                         n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_are_threats_refuted_in_n(threats,len_threat,si,n);
      break;

    case STLeafDirect:
      assert(len_threat==slack_length_direct);
      assert(n==slack_length_direct+1);
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
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type direct_has_solution_in_n(slice_index si,
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
    case STBranchDirect:
      result = branch_d_has_solution_in_n(si,n,n_min);
      break;

    case STDirectHashed:
      result = direct_hashed_has_solution_in_n(si,n,n_min);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_has_solution_in_n(si,n,n_min);
      break;

    case STDirectDefense:
      result = direct_defense_direct_has_solution_in_n(si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_direct_has_solution_in_n(si,n,n_min);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_has_solution_in_n(si,n,n_min);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_has_solution_in_n(si,n,n_min);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_has_solution_in_n(si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_direct+1);
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
has_solution_type direct_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type n_min;
  stip_length_type const parity = length%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length+min_length>slack_length_direct+length)
    n_min = length-(length-min_length);
  else
    n_min = slack_length_direct-parity;

  result = (direct_has_solution_in_n(si,length,n_min)<=length
            ? has_solution
            : has_no_solution);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
void direct_solve_continuations_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      branch_d_solve_continuations_in_n(si,n,n_min);
      break;

    case STDirectHashed:
      direct_hashed_solve_continuations_in_n(si,n,n_min);
      break;

    case STDirectDefense:
      direct_defense_direct_solve_continuations_in_n(si,n,n_min);
      break;

    case STSelfDefense:
      self_defense_direct_solve_continuations_in_n(si,n,n_min);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_direct_solve_continuations_in_n(si,n,n_min);
      break;

    case STReflexGuard:
      reflex_guard_direct_solve_continuations_in_n(si,n,n_min);
      break;

    case STKeepMatingGuard:
      keepmating_guard_direct_solve_continuations_in_n(si,n,n_min);
      break;

    case STDegenerateTree:
      degenerate_tree_direct_solve_continuations_in_n(si,n,n_min);
      break;

    default:
      assert(0);
      break;
  }

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
stip_length_type direct_solve_threats_in_n(table threats,
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
    case STBranchDirect:
      result = branch_d_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STDirectHashed:
      result = direct_hashed_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STDirectDefense:
      result = direct_defense_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_solve_threats_in_n(threats,si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_direct+1);
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
void direct_solve_threats(table threats, slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const parity = (length-slack_length_direct)%2;
  stip_length_type const n_min = slack_length_direct+2-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  direct_solve_threats_in_n(threats,si,length,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_are_threats_refuted(table threats,
                                   stip_length_type len_threat,
                                   slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  if (slices[si].type==STLeafDirect)
    result = leaf_d_are_threats_refuted(threats,si);
  else
  {
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    result = direct_are_threats_refuted_in_n(threats,len_threat,si,length);
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
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type direct_solve_in_n(slice_index si,
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
    case STLeafDirect:
      result = leaf_d_solve_in_n(si,n,n_min);
      break;

    case STBranchDirect:
      result = branch_d_solve_in_n(si,n,n_min);
      break;

    case STDirectHashed:
      result = direct_hashed_solve_in_n(si,n,n_min);
      break;

    case STDirectDefense:
      result = direct_defense_solve_in_n(si,n,n_min);
      break;

    case STSelfDefense:
      result = self_defense_solve_in_n(si,n,n_min);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_solve_in_n(si,n,n_min);
      break;

    case STReflexGuard:
      result = reflex_guard_solve_in_n(si,n,n_min);
      break;

    case STDegenerateTree:
      result = direct_solve_in_n(slices[si].u.pipe.next,n,n_min);
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
boolean direct_solve(slice_index si)
{
  boolean result;
  stip_length_type length;
  stip_length_type min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type==STLeafDirect)
  {
    length = slack_length_direct+1;
    min_length = slack_length_direct+1;
  }
  else
  {
    length = slices[si].u.pipe.u.branch.length;
    min_length = slices[si].u.pipe.u.branch.min_length;
  }

  result = direct_solve_in_n(si,length,min_length)<=length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found and written
 */
boolean direct_root_solve_in_n(slice_index si)
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

    case STDirectRoot:
      result = direct_root_root_solve(si);
      break;

    case STDirectDefenderRoot:
      result = branch_d_defender_root_solve(si);
      break;

    case STDirectDefense:
      result = direct_defense_root_solve(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_solve(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_solve(si);
      break;

    case STDirectHashed:
      result = direct_root_solve(slices[si].u.pipe.next);
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
boolean direct_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_root_solve_in_n(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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

    case STSelfCheckGuard:
      result = selfcheck_guard_defend_in_n(si,n);
      break;

    case STSelfAttack:
      result = self_attack_defend_in_n(si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_defend_in_n(si,n);
      break;

    case STKeepMatingGuard:
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
    case STDirectDefenderRoot:
    case STBranchDirectDefender:
      result = branch_d_defender_can_defend_in_n(si,n,max_result);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_can_defend_in_n(si,n,max_result);
      break;

    case STSelfAttack:
      result = self_attack_can_defend_in_n(si,n,max_result);
      break;

    case STReflexGuard:
      result = reflex_guard_can_defend_in_n(si,n,max_result);
      break;

    case STKeepMatingGuard:
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

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type direct_defender_solve_threats_in_n(table threats,
                                                    slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_solve_threats_in_n(threats,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defender_solve_threats_in_n(threats,si,n);
      break;

    case STSelfAttack:
      result = self_attack_solve_threats_in_n(threats,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_defender_solve_threats_in_n(threats,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_defender_solve_threats_in_n(threats,si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_solve_threats_in_n(threats,si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_solve_threats_in_n(threats,si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_solve_threats_in_n(threats,si,n);
      break;

    case STRestartGuard:
      result = restart_guard_solve_threats_in_n(threats,si,n);
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

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean direct_defender_solve_variations_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STSelfAttack:
      result = self_attack_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_solve_variations_in_n(threats,len_threat,
                                                           si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_solve_variations_in_n(threats,len_threat,si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_solve_variations_in_n(threats,len_threat,
                                                             si,n);
      break;

    case STRestartGuard:
      result = restart_guard_solve_variations_in_n(threats,len_threat,si,n);
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
