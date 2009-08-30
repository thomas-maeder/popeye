#include "pydirect.h"
#include "pybrad.h"
#include "pybradd.h"
#include "pybrafrk.h"
#include "pyleaff.h"
#include "pyleafh.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyreflxg.h"
#include "pymovenb.h"
#include "pykeepmt.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>


#define ENUMERATION_TYPENAME quantity_of_refutations_type
#define ENUMERATORS                             \
  ENUMERATOR(attacker_has_solved_next_slice),   \
    ENUMERATOR(found_no_refutation),            \
    ENUMERATOR(found_refutations),              \
    ENUMERATOR(attacker_has_reached_deadend)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

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
                                        int curr_max_nr_nontrivial)
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
                                           int curr_max_nr_nontrivial)
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


/* Find refutations after a move of the attacking side at a nested level.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker
 *         attacker_has_solved_next_slice if the attacking move has solved the branch
 *         found_refutations if there is a refutation
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_defender_find_refutations_in_n(slice_index si,
                                      stip_length_type n,
                                      int curr_max_nr_nontrivial)
{
  quantity_of_refutations_type result = found_refutations;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_find_refutations_in_n(si,
                                                       n,
                                                       curr_max_nr_nontrivial);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_find_refutations_in_n(si,
                                                     n,
                                                     curr_max_nr_nontrivial);
      break;

    case STDirectAttack:
      result = direct_attack_find_refutations_in_n(si,
                                                   n,
                                                   curr_max_nr_nontrivial);
      break;

    case STSelfAttack:
      result = self_attack_find_refutations_in_n(si,
                                                 n,
                                                 curr_max_nr_nontrivial);
      break;

    case STReflexGuard:
      result = reflex_guard_find_refutations_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_find_refutations_in_n(si,
                                                      n,
                                                      curr_max_nr_nontrivial);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
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

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_defender_root_solve_postkey(table refutations, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_solve_postkey(refutations,si);
      break;

    case STLeafHelp:
      /* we don't output refutations of r#1
       */
      result = leaf_h_root_solve_postkey(si);
      break;

    case STLeafForced:
      result = leaf_forced_root_solve_postkey(refutations,si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_solve_postkey(refutations,si);
      break;

    case STDirectAttack:
      result = direct_attack_root_solve_postkey(refutations,si);
      break;

    case STSelfAttack:
      result = self_attack_root_solve_postkey(refutations,si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_solve_postkey(refutations,si);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_root_solve_postkey(refutations,si);
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

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_defender_root_find_refutations(table refutations,
                                      slice_index si)
{
  quantity_of_refutations_type result = attacker_has_reached_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_find_refutations(refutations,si);
      break;

    case STLeafDirect:
      /* no need to fill refutations; we don't output refutations of #1
       */
      result = leaf_d_root_find_refutations(si);
      break;

    case STLeafHelp:
      /* no need to fill refutations; we don't output refutations of r#1
       */
      result = leaf_h_root_find_refutations(si);
      break;

    case STLeafForced:
      result = leaf_forced_root_find_refutations(refutations,si);
      break;

    case STDirectAttack:
      result = direct_attack_root_find_refutations(refutations,si);
      break;

    case STSelfAttack:
      result = self_attack_root_find_refutations(refutations,si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_find_refutations(refutations,si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_find_refutations(refutations,si);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_root_find_refutations(refutations,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
