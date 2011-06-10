#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "stipulation/setplay_fork.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/false.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/not.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "stipulation/goals/any/reached_tester.h"
#include "solving/play_suppressor.h"
#include "pymovein.h"
#include "pyhash.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "pykeepmt.h"
#include "pypipe.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/circuit_special.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "options/maxsolutions/initialiser.h"
#include "options/maxsolutions/guard.h"
#include "options/stoponshortsolutions/initialiser.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/line_writer.h"

#include <assert.h>
#include <stdlib.h>


#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(opponent_self_check),              \
    ENUMERATOR(has_no_solution),                \
    ENUMERATOR(has_solution)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STTrue:
      result = true_solve(si);
      break;

    case STFalse:
      result = false_solve(si);
      break;

    case STSetplayFork:
      result = setplay_fork_solve(si);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_solve(si);
      break;

    case STAttackAdapter:
      result = attack_adapter_solve(si);
      break;

    case STDefenseAdapter:
      result = defense_adapter_solve(si);
      break;

    case STHelpAdapter:
      result = help_adapter_solve(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_solve(si);
      break;

    case STOr:
      result = or_solve(si);
      break;

    case STAnd:
      result = and_solve(si);
      break;

    case STNot:
      result = not_solve(si);
      break;

    case STMoveInverter:
      result = move_inverter_solve(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_solve(si);
      break;

    case STMaxSolutionsInitialiser:
      result = maxsolutions_initialiser_solve(si);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_solve(si);
      break;

    case STStopOnShortSolutionsInitialiser:
      result = stoponshortsolutions_initialiser_solve(si);
      break;

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_solve(si);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_solve(si);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_solve(si);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_solve(si);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = goal_writer_solve(si);
      break;

    case STOutputPlaintextMoveInversionCounter:
      result = output_plaintext_move_inversion_counter_solve(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_solve(si);
      break;

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_solve(si);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_solve(si);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_solve(si);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_solve(si);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_solve(si);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_solve(si);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_solve(si);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_solve(si);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_solve(si);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_solve(si);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_solve(si);
      break;

    case STPlaySuppressor:
      result = play_suppressor_solve(si);
      break;

    default:
      assert(0);
      result = has_no_solution;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STTrue:
      result = true_has_solution(si);
      break;

    case STFalse:
      result = false_has_solution(si);
      break;

    case STSetplayFork:
      result = setplay_fork_has_solution(si);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_has_solution(si);
      break;

    case STGoalTargetReachedTester:
      result = goal_target_reached_tester_has_solution(si);
      break;

    case STGoalCheckReachedTester:
      result = goal_check_reached_tester_has_solution(si);
      break;

    case STGoalCaptureReachedTester:
      result = goal_capture_reached_tester_has_solution(si);
      break;

    case STGoalSteingewinnReachedTester:
      result = goal_steingewinn_reached_tester_has_solution(si);
      break;

    case STGoalEnpassantReachedTester:
      result = goal_enpassant_reached_tester_has_solution(si);
      break;

    case STGoalDoubleMateReachedTester:
      result = goal_doublemate_reached_tester_has_solution(si);
      break;

    case STGoalCounterMateReachedTester:
      result = goal_countermate_reached_tester_has_solution(si);
      break;

    case STGoalCastlingReachedTester:
      result = goal_castling_reached_tester_has_solution(si);
      break;

    case STGoalCircuitReachedTester:
      result = goal_circuit_reached_tester_has_solution(si);
      break;

    case STGoalExchangeReachedTester:
      result = goal_exchange_reached_tester_has_solution(si);
      break;

    case STGoalCircuitByRebirthReachedTester:
      result = goal_circuit_by_rebirth_reached_tester_has_solution(si);
      break;

    case STGoalExchangeByRebirthReachedTester:
      result = goal_exchange_by_rebirth_reached_tester_has_solution(si);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      result = goal_proofgame_reached_tester_has_solution(si);
      break;

    case STGoalImmobileReachedTester:
      result = goal_immobile_reached_tester_has_solution(si);
      break;

    case STGoalNotCheckReachedTester:
      result = goal_notcheck_reached_tester_has_solution(si);
      break;

    case STGoalAnyReachedTester:
      result = goal_any_reached_tester_has_solution(si);
      break;

    case STOr:
      result = or_has_solution(si);
      break;

    case STAnd:
      result = and_has_solution(si);
      break;

    case STNot:
      result = not_has_solution(si);
      break;

    case STAttackAdapter:
      result = attack_adapter_has_solution(si);
      break;

    case STDefenseAdapter:
      result = defense_adapter_has_solution(si);
      break;

    case STHelpAdapter:
      result = help_adapter_has_solution(si);
      break;

    case STMoveInverter:
      result = move_inverter_has_solution(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_solution(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_has_solution(si);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_has_solution(si);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_has_solution(si);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_has_solution(si);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = goal_writer_has_solution(si);
      break;

    case STOutputPlaintextMoveInversionCounter:
      result = output_plaintext_move_inversion_counter_has_solution(si);
      break;

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_has_solution(si);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_has_solution(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_has_solution(si);
      break;

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_has_solution(si);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_has_solution(si);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_has_solution(si);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_has_solution(si);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_has_solution(si);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_has_solution(si);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_has_solution(si);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_has_solution(si);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_has_solution(si);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_has_solution(si);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_has_solution(si);
      break;

    case STPlaySuppressor:
      result = play_suppressor_has_solution(si);
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
