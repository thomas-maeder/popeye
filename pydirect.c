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
#include "pythreat.h"
#include "pynontrv.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return true iff the attacker has reached a deadend (e.g. by
 *         immobilising the defender in a non-stalemate stipulation)
 */
boolean direct_defender_root_defend(table refutations, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_defend(refutations,si);
      break;

    case STDirectAttack:
      result = direct_attack_root_defend(refutations,si);
      break;

    case STSelfAttack:
      result = self_attack_root_defend(refutations,si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_defend(refutations,si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_defend(refutations,si);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_root_defend(refutations,si);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_root_defend(refutations,si);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_defend(refutations,si);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_root_defend(refutations,si);
      break;

    case STRestartGuard:
      result = restart_guard_root_defend(refutations,si);
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

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_are_threats_refuted_in_n(table threats,
                                        stip_length_type len_threat,
                                        slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = branch_d_are_threats_refuted_in_n(threats,
                                                 len_threat,
                                                 si,
                                                 n,
                                                 curr_max_nr_nontrivial);
      break;

    case STDirectHashed:
      result = direct_hashed_are_threats_refuted_in_n(threats,
                                                      len_threat,
                                                      si,
                                                      n,
                                                      curr_max_nr_nontrivial);
      break;

    case STDirectDefense:
      result = direct_defense_are_threats_refuted_in_n(threats,
                                                       len_threat,
                                                       si,
                                                       n,
                                                       curr_max_nr_nontrivial);
      break;

    case STSelfDefense:
      result = self_defense_are_threats_refuted_in_n(threats,
                                                     len_threat,
                                                     si,
                                                     n,
                                                     curr_max_nr_nontrivial);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_are_threats_refuted_in_n(threats,
                                                        len_threat,
                                                        si,
                                                        n,
                                                        curr_max_nr_nontrivial);
      break;

    case STReflexGuard:
      result = reflex_guard_are_threats_refuted_in_n(threats,
                                                     len_threat,
                                                     si,
                                                     n,
                                                     curr_max_nr_nontrivial);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_are_threats_refuted_in_n(threats,
                                                         len_threat,
                                                         si,
                                                         n,
                                                         curr_max_nr_nontrivial);
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
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type direct_has_solution_in_n(slice_index si,
                                           stip_length_type n,
                                           unsigned int curr_max_nr_nontrivial)
{
  has_solution_type result = has_no_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = branch_d_has_solution_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STDirectHashed:
      result = direct_hashed_has_solution_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_has_solution_in_n(si,
                                                        n,
                                                        curr_max_nr_nontrivial);
      break;

    case STDirectDefense:
      result = direct_defense_direct_has_solution_in_n(si,
                                                       n,
                                                       curr_max_nr_nontrivial);
      break;

    case STSelfDefense:
      result = self_defense_direct_has_solution_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_has_solution_in_n(si,
                                                     n,
                                                     curr_max_nr_nontrivial);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_has_solution_in_n(si,
                                                         n,
                                                         curr_max_nr_nontrivial);
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

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void direct_solve_continuations_in_n(table continuations,
                                     slice_index si,
                                     stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      branch_d_solve_continuations_in_n(continuations,si,n);
      break;

    case STDirectHashed:
      direct_hashed_solve_continuations_in_n(continuations,si,n);
      break;

    case STDirectDefense:
      direct_defense_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STSelfDefense:
      self_defense_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STReflexGuard:
      reflex_guard_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STKeepMatingGuard:
      keepmating_guard_direct_solve_continuations_in_n(continuations,si,n);
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
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type direct_solve_threats_in_n(table threats,
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
    case STBranchDirect:
      result = branch_d_solve_threats_in_n(threats,si,n);
      break;

    case STDirectHashed:
      result = direct_hashed_solve_threats_in_n(threats,si,n);
      break;

    case STDirectDefense:
      result = direct_defense_direct_solve_threats_in_n(threats,si,n);
      break;

    case STSelfDefense:
      result = self_defense_direct_solve_threats_in_n(threats,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_solve_threats_in_n(threats,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_solve_threats_in_n(threats,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_solve_threats_in_n(threats,si,n);
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
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean direct_defender_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    unsigned int curr_max_nr_nontrivial)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STDirectAttack:
      result = direct_attack_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STSelfAttack:
      result = self_attack_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STReflexGuard:
      result = reflex_guard_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend_in_n(si,
                                                   n,
                                                   curr_max_nr_nontrivial);
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

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean direct_defender_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STDirectAttack:
      result = direct_attack_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STSelfAttack:
      result = self_attack_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STReflexGuard:
      result = reflex_guard_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,
                                                     n,
                                                     curr_max_nr_nontrivial);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend_in_n(si,
                                                       n,
                                                       curr_max_nr_nontrivial);
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

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
boolean direct_defender_solve_postkey_in_n(slice_index si, stip_length_type n)
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
      result = branch_d_defender_solve_postkey_in_n(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_solve_postkey_in_n(si,n);
      break;

    case STDirectAttack:
      result = direct_attack_solve_postkey_in_n(si,n);
      break;

    case STSelfAttack:
      result = self_attack_solve_postkey_in_n(si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_solve_postkey_in_n(si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_solve_postkey_in_n(si,n);
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
