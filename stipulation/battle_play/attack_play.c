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
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/variation.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
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
    case STSelfCheckGuardSeriesFilter:
    {
      stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
      stip_length_type const nr_moves_needed = series_has_solution_in_n(si,
                                                                        n_ser);
      if (nr_moves_needed==n_ser+2)
        result = n_min-2;
      else if (nr_moves_needed==n_ser+1)
        result = n+2;
      else
        result = n;
      break;
    }

    case STSelfDefense:
      result = self_defense_direct_has_solution_in_n(si,n,n_min);
      break;

    case STLeafDirect:
      assert(n==slack_length_battle+2);
      if (leaf_d_has_solution(si)==has_solution)
        result = n;
      break;

    case STLeafForced:
    case STQuodlibet:
      assert(n==slack_length_battle);
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = n-2;
          break;

        case has_solution:
          result = n;
          break;

        case has_no_solution:
          result = n+2;
          break;

        default:
          assert(0);
          result = n+2;
          break;
      }
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

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_has_solution_in_n(si,n,n_min);
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
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const n_min = slices[si].u.branch.min_length;
    stip_length_type const sol_length = attack_has_solution_in_n(si,
                                                                 length,n_min);
    if (sol_length<n_min)
      result = opponent_self_check;
    else if (sol_length<=length)
      result = has_solution;
    else
      result = has_no_solution;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
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
      assert(n==slack_length_battle+2);
      assert(n_min==slack_length_battle+2);
      result = leaf_d_solve(si) ? n : n+2;
      break;

    case STLeafForced:
    case STQuodlibet:
      assert(n==slack_length_battle);
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = n-2;
          break;

        case has_solution:
          result = n;
          break;

        case has_no_solution:
          result = n+2;
          break;

        default:
          assert(0);
          result = n+2;
          break;
      }
      break;

    case STAttackMove:
      result = attack_move_solve_in_n(si,n,n_min);
      break;

    case STSeriesMove:
    case STSeriesHashed:
    case STSeriesFork:
    case STSelfCheckGuardSeriesFilter:
    {
      stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
      stip_length_type nr_moves_needed = series_solve_in_n(si,n_ser);
      if (nr_moves_needed==n_ser+2)
        result = n_min-2;
      else if (nr_moves_needed==n_ser+1)
        result = n+2;
      else
        result = n_min;
      break;
    }

    case STAttackHashed:
      result = attack_hashed_solve_in_n(si,n,n_min);
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
      result = degenerate_tree_direct_solve_in_n(si,n,n_min);
      break;

    case STKeepMatingGuardAttackerFilter:
      result = keepmating_guard_direct_solve_in_n(si,n,n_min);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_solve_in_n(si,n,n_min);
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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STLeafDirect:
    {
      stip_length_type const length = slack_length_battle+2;
      stip_length_type const min_length = slack_length_battle+2;
      stip_length_type const sol_length = attack_solve_in_n(si,
                                                            length,min_length);
      if (sol_length==min_length-2)
        result = opponent_self_check;
      else if (sol_length<=length)
        result = has_solution;
      else
        result = has_no_solution;
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
      stip_length_type const sol_length = attack_solve_in_n(si,
                                                            length,min_length);
      if (sol_length==min_length-2)
        result = opponent_self_check;
      else if (sol_length<=length)
        result = has_solution;
      else
        result = has_no_solution;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_root_solve(si);
      break;

    case STAttackHashed:
      result = attack_root_solve(slices[si].u.pipe.next);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_root_solve(si);
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
