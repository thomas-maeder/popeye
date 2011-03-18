#include "stipulation/battle_play/attack_play.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pyreflxg.h"
#include "pymovenb.h"
#include "pykeepmt.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "pyquodli.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/attack_move_generator.h"
#include "stipulation/battle_play/attack_fork.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/dead_end.h"
#include "stipulation/battle_play/min_length_attack_filter.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "stipulation/series_play/play.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/killer_move/attack_generator.h"
#include "optimisations/killer_move/collector.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/decoration_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "trace.h"

#include <assert.h>

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STRefutationsCollector:
      result = refutations_collector_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STVariationWriter:
      result = variation_writer_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_has_solution_in_n(si,
                                                           n,
                                                           n_max_unsolvable);
      break;

    case STNoShortVariations:
      result = no_short_variations_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STBattleDeadEnd:
      result = battle_play_dead_end_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STMinLengthAttackFilter:
      result = min_length_attack_filter_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackFork:
      result = attack_fork_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackFindShortest:
      result = attack_find_shortest_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackMoveGenerator:
      result = attack_move_generator_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackMove:
      result = attack_move_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackHashed:
      result = attack_hashed_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STSeriesAdapter:
    {
      stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
      stip_length_type const nr_moves_needed = series_has_solution_in_n(si,
                                                                        n_ser);
      if (nr_moves_needed==n_ser+2)
        result = slack_length_battle-2;
      else if (nr_moves_needed==n_ser+1)
        result = n+2;
      else
        result = n;
      break;
    }

    case STSelfDefense:
      result = self_defense_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STQuodlibet:
      result = quodlibet_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_attack_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_attack_has_solution_in_n(si,
                                                          n,n_max_unsolvable);
      break;

    case STDegenerateTree:
      result = degenerate_tree_direct_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_has_solution_in_n(si,
                                                           n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_has_solution_in_n(si,
                                                                      n,
                                                                      n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_has_solution_in_n(si,
                                                                         n,
                                                                         n_max_unsolvable);
      break;

    case STRefutationWriter:
      result = refutation_writer_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STDoubleMateFilter:
      result = doublemate_attacker_filter_has_solution_in_n(si,
                                                            n,
                                                            n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_attacker_filter_has_solution_in_n(si,
                                                             n,
                                                             n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STCastlingFilter:
      result = castling_filter_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_has_solution_in_n(si,
                                                             n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_has_solution_in_n(si,
                                                                   n,
                                                                   n_max_unsolvable);
      break;

    case STKillerMoveAttackGenerator:
      result = killer_move_attack_generator_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_has_solution_in_n(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    default:
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = slack_length_battle-2;
          break;

        case has_solution:
          result = slack_length_battle;
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_solve_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STZugzwangWriter:
      result = zugzwang_writer_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STRefutationsCollector:
      result = refutations_collector_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STVariationWriter:
      result = variation_writer_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STNoShortVariations:
      result = no_short_variations_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STQuodlibet:
      result = quodlibet_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackFindShortest:
    case STDegenerateTree:
      result = attack_find_shortest_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackMoveGenerator:
    case STKillerMoveAttackGenerator:
      result = attack_move_generator_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackMove:
      result = attack_move_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STBattleDeadEnd:
      result = battle_play_dead_end_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STMinLengthAttackFilter:
      result = min_length_attack_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STAttackFork:
      result = attack_fork_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STSeriesAdapter:
    {
      stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
      stip_length_type nr_moves_needed = series_solve_in_n(si,n_ser);
      if (nr_moves_needed==n_ser+2)
        result = slack_length_battle-2;
      else if (nr_moves_needed==n_ser+1)
        result = n+2;
      else
        result = n;
      break;
    }

    case STAttackHashed:
      result = attack_hashed_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STSelfDefense:
      result = self_defense_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STReflexAttackerFilter:
      result = reflex_attacker_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_attack_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_attack_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_solve_in_n(si,
                                                               n,
                                                               n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_solve_in_n(si,
                                                                  n,
                                                                  n_max_unsolvable);
      break;

    case STRefutationWriter:
      result = refutation_writer_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STDoubleMateFilter:
      result = doublemate_attacker_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_attacker_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STCastlingFilter:
      result = castling_filter_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_solve_in_n(si,
                                                            n,
                                                            n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    default:
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = slack_length_battle-2;
          break;

        case has_solution:
          result = slack_length_battle;
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
