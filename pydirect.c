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
#include "pyint.h"
#include "trace.h"

#include <assert.h>

#define ENUMERATION_TYPENAME attack_result_type
#define ENUMERATORS \
  ENUMERATOR(attack_has_reached_deadend),       \
    ENUMERATOR(attack_refuted_full_length),    \
    ENUMERATOR(attack_solves_full_length),    \
    ENUMERATOR(attack_has_solved_next_branch)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type direct_defender_root_defend(table refutations,
                                               slice_index si)
{
  attack_result_type result = attack_has_reached_deadend;

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
  TraceEnumerator(attack_result_type,result,"");
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
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type direct_has_solution_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min)
{
  has_solution_type result = has_no_solution;

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
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type direct_solve_continuations_in_n(table continuations,
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
      result = branch_d_solve_continuations_in_n(continuations,si,n);
      break;

    case STDirectHashed:
      result = direct_hashed_solve_continuations_in_n(continuations,si,n);
      break;

    case STDirectDefense:
      result = direct_defense_direct_solve_continuations_in_n(continuations,
                                                              si,
                                                              n);
      break;

    case STSelfDefense:
      result = self_defense_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_solve_continuations_in_n(continuations,
                                                               si,
                                                               n);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_solve_continuations_in_n(continuations,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_solve_continuations_in_n(continuations,
                                                                si,
                                                                n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_solve_continuations_in_n(continuations,
                                                               si,
                                                               n);
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
stip_length_type direct_solve_threats(table threats,
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
      result = branch_d_solve_threats(threats,si,n);
      break;

    case STDirectHashed:
      result = direct_hashed_solve_threats(threats,si,n);
      break;

    case STDirectDefense:
      result = direct_defense_direct_solve_threats(threats,si,n);
      break;

    case STSelfDefense:
      result = self_defense_direct_solve_threats(threats,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_direct_solve_threats(threats,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_direct_solve_threats(threats,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_direct_solve_threats(threats,si,n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_solve_threats(threats,si,n);
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
 * @return success of key move
 */
attack_result_type direct_defender_defend_in_n(slice_index si,
                                               stip_length_type n)
{
  attack_result_type result = attack_has_reached_deadend;

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

    case STDirectAttack:
      result = direct_attack_defend_in_n(si,n);
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
  TraceEnumerator(attack_result_type,result,"");
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

    case STDirectAttack:
      result = direct_attack_can_defend_in_n(si,n,max_result);
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
stip_length_type direct_defender_solve_threats(table threats,
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
    case STDirectDefenderRoot:
    case STBranchDirectDefender:
      result = branch_d_defender_solve_threats(threats,si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defender_solve_threats(threats,si,n);
      break;

    case STDirectAttack:
      result = direct_attack_solve_threats(threats,si,n);
      break;

    case STSelfAttack:
      result = self_attack_solve_threats(threats,si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_defender_solve_threats(threats,si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_defender_solve_threats(threats,si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_solve_threats(threats,si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_solve_threats(threats,si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_solve_threats(threats,si,n);
      break;

    case STRestartGuard:
      result = restart_guard_solve_threats(threats,si,n);
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
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void direct_defender_root_solve_variations(table threats,
                                           stip_length_type len_threat,
                                           table refutations,
                                           slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      branch_d_defender_root_solve_variations(threats,len_threat,
                                              refutations,
                                              si);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_root_solve_variations(threats,len_threat,
                                            refutations,
                                            si);
      break;

    case STDirectAttack:
      direct_attack_root_solve_variations(threats,len_threat,
                                          refutations,
                                          si);
      break;

    case STSelfAttack:
      self_attack_root_solve_variations(threats,len_threat,
                                        refutations,
                                        si);
      break;

    case STReflexGuard:
      reflex_guard_root_solve_variations(threats,len_threat,
                                         refutations,
                                         si);
      break;

    case STKeepMatingGuard:
      keepmating_guard_root_solve_variations(threats,len_threat,
                                             refutations,
                                             si);
      break;

    case STMaxThreatLength:
      maxthreatlength_guard_root_solve_variations(threats,len_threat,
                                                  refutations,
                                                  si);
      break;

    case STMaxFlightsquares:
      maxflight_guard_root_solve_variations(threats,len_threat,
                                            refutations,
                                            si);
      break;

    case STMaxNrNonTrivial:
      max_nr_nontrivial_guard_root_solve_variations(threats,len_threat,
                                                    refutations,
                                                    si);
      break;

    case STRestartGuard:
      restart_guard_root_solve_variations(threats,len_threat,
                                          refutations,
                                          si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

    case STDirectAttack:
      result = direct_attack_solve_variations_in_n(threats,len_threat,si,n);
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
