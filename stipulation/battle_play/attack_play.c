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
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move_played.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "solving/fork_on_remaining.h"
#include "solving/find_shortest.h"
#include "solving/move_generator.h"
#include "solving/for_each_move.h"
#include "solving/find_move.h"
#include "solving/play_suppressor.h"
#include "solving/avoid_unsolvable.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/check_detector.h"
#include "conditions/bgl.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/killer_move/move_generator.h"
#include "optimisations/killer_move/collector.h"
#include "solving/trivial_end_filter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "trace.h"

#include <assert.h>

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_attack(si,n);
      break;

    case STZugzwangWriter:
      result = zugzwang_writer_attack(si,n);
      break;

    case STRefutationsAvoider:
      result = refutations_avoider_attack(si,n);
      break;

    case STRefutationsFilter:
      result = refutations_filter_attack(si,n);
      break;

    case STMoveWriter:
      result = move_writer_attack(si,n);
      break;

    case STTrivialEndFilter:
      result = trivial_end_filter_attack(si,n);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_attack(si,n);
      break;

    case STNoShortVariations:
      result = no_short_variations_attack(si,n);
      break;

    case STOr:
      result = or_attack(si,n);
      break;

    case STFindShortest:
      result = find_shortest_attack(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_attack(si,n);
      break;

    case STForEachMove:
      result = for_each_move_attack(si,n);
      break;

    case STMovePlayed:
      result = move_played_attack(si,n);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      result = move_tracer_attack(si,n);
      break;
#endif

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_attack(si,n);
      break;

    case STDeadEnd:
      result = dead_end_attack(si,n);
      break;

    case STMinLengthOptimiser:
      result = min_length_optimiser_attack(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_attack(si,n);
      break;

    case STAttackHashed:
      result = attack_hashed_attack(si,n);
      break;

    case STEndOfBranch:
      result = end_of_branch_attack(si,n);
      break;

    case STEndOfBranchGoal:
      result = end_of_branch_goal_attack(si,n);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_attack(si,n);
      break;

    case STAvoidUnsolvable:
      result = avoid_unsolvable_attack(si,n);
      break;

    case STConstraintSolver:
      result = constraint_attack(si,n);
      break;

    case STConstraintTester:
      result = constraint_tester_attack(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_attack(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_attack(si,n);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_attack(si,n);
      break;

    case STRefutationWriter:
      result = refutation_writer_attack(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_attack(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_attack(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_attack(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_attack(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_attack(si,n);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_attack(si,n);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_attack(si,n);
      break;

    case STBGLFilter:
      result = bgl_filter_attack(si,n);
      break;

    case STCheckDetector:
      result = check_detector_attack(si,n);
      break;

    case STRefutationsCollector:
      result = refutations_collector_attack(si,n);
      break;

    case STMinLengthGuard:
      result = min_length_guard_attack(si,n);
      break;

    case STFindMove:
      result = find_move_attack(si,n);
      break;

    case STAttackHashedTester:
      result = attack_hashed_tester_attack(si,n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_attack(si,n);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_attack(si,n);
      break;

    case STKillerMoveMoveGenerator:
      result = killer_move_move_generator_attack(si,n);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_attack(si,n);
      break;

    case STTrue:
      result = slack_length;
      break;

    case STFalse:
      result = n+2;
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
