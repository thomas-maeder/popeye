#include "stipulation/battle_play/attack_play.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pykeepmt.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "solving/fork_on_remaining.h"
#include "solving/find_shortest.h"
#include "solving/move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/continuation.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/killer_move/move_generator.h"
#include "optimisations/killer_move/collector.h"
#include "output/plaintext/trivial_end_filter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/decoration_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/move_writer.h"
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
stip_length_type can_attack(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_can_attack(si,n,n_max_unsolvable);
      break;

    case STRefutationsCollector:
      result = refutations_collector_can_attack(si,n,n_max_unsolvable);
      break;

    case STMoveWriter:
      result = move_writer_can_attack(si,n,n_max_unsolvable);
      break;

    case STTrivialEndFilter:
      result = trivial_end_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_can_attack(si,n,n_max_unsolvable);
      break;

    case STNoShortVariations:
      result = no_short_variations_can_attack(si,n,n_max_unsolvable);
      break;

    case STDeadEnd:
      result = dead_end_can_attack(si,n,n_max_unsolvable);
      break;

    case STMinLengthOptimiser:
      result = min_length_optimiser_can_attack(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_can_attack(si,n,n_max_unsolvable);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_can_attack(si,n,n_max_unsolvable);
      break;

    case STFindShortest:
      result = find_shortest_can_attack(si,n,n_max_unsolvable);
      break;

    case STMoveGenerator:
      result = move_generator_can_attack(si,n,n_max_unsolvable);
      break;

    case STMove:
      result = move_can_attack(si,n,n_max_unsolvable);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_can_attack(si,n,n_max_unsolvable);
      break;

    case STAttackHashed:
      result = attack_hashed_can_attack(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_goal_can_attack(si,n,n_max_unsolvable);
      break;

    case STOr:
      result = or_can_attack(si,n,n_max_unsolvable);
      break;

    case STConstraint:
      result = constraint_can_attack(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_can_attack(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STDegenerateTree:
      result = degenerate_tree_can_attack(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_can_attack(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_can_attack(si,
                                                             n,
                                                             n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_can_attack(si,
                                                                  n,
                                                                  n_max_unsolvable);
      break;

    case STRefutationWriter:
      result = refutation_writer_can_attack(si,n,n_max_unsolvable);
      break;

    case STDoubleMateFilter:
      result = doublemate_attacker_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_attacker_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STCastlingFilter:
      result = castling_filter_can_attack(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_can_attack(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_can_attack(si,
                                                            n,n_max_unsolvable);
      break;

    case STKillerMoveMoveGenerator:
      result = killer_move_move_generator_can_attack(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_can_attack(si,n,n_max_unsolvable);
      break;

    case STTrue:
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
stip_length_type attack(slice_index si,
                        stip_length_type n,
                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_attack(si,n,n_max_unsolvable);
      break;

    case STZugzwangWriter:
      result = zugzwang_writer_attack(si,n,n_max_unsolvable);
      break;

    case STRefutationsCollector:
      result = refutations_collector_attack(si,n,n_max_unsolvable);
      break;

    case STMoveWriter:
      result = move_writer_attack(si,n,n_max_unsolvable);
      break;

    case STTrivialEndFilter:
      result = trivial_end_filter_attack(si,n,n_max_unsolvable);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_attack(si,n,n_max_unsolvable);
      break;

    case STNoShortVariations:
      result = no_short_variations_attack(si,n,n_max_unsolvable);
      break;

    case STOr:
      result = or_attack(si,n,n_max_unsolvable);
      break;

    case STFindShortest:
    case STDegenerateTree:
      result = find_shortest_solve_in_n(si,n,n_max_unsolvable);
      break;

    case STMoveGenerator:
    case STKillerMoveMoveGenerator:
      result = move_generator_attack(si,n,n_max_unsolvable);
      break;

    case STMove:
      result = move_attack(si,n,n_max_unsolvable);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_attack(si,n,n_max_unsolvable);
      break;

    case STDeadEnd:
      result = dead_end_attack(si,n,n_max_unsolvable);
      break;

    case STMinLengthOptimiser:
      result = min_length_optimiser_attack(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_attack(si,n,n_max_unsolvable);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_attack(si,n,n_max_unsolvable);
      break;

    case STAttackHashed:
      result = attack_hashed_attack(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_goal_attack(si,n,n_max_unsolvable);
      break;

    case STConstraint:
      result = constraint_attack(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_attack(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_attack(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_attack(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_attack(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_attack(si,
                                                              n,
                                                              n_max_unsolvable);
      break;

    case STRefutationWriter:
      result = refutation_writer_attack(si,n,n_max_unsolvable);
      break;

    case STDoubleMateFilter:
      result = doublemate_attacker_filter_attack(si,n,n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_attacker_filter_attack(si,n,n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_attack(si,n,n_max_unsolvable);
      break;

    case STCastlingFilter:
      result = castling_filter_attack(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_attack(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_attack(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_attack(si,n,n_max_unsolvable);
      break;

    case STTrue:
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
