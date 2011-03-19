#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pyquodli.h"
#include "pybrafrk.h"
#include "pynot.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pydirctg.h"
#include "pypipe.h"
#include "stipulation/operators/binary.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/end_of_branch.h"
#include "stipulation/leaf.h"
#include "stipulation/branch.h"
#include "stipulation/setplay_fork.h"
#include "stipulation/reflex_attack_solver.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/fork.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_fork.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/dead_end.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/help_play/ready_for_help_move.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/find_shortest.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/move_to_goal.h"
#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/series_play/ready_for_series_move.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/find_shortest.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/parry_fork.h"
#include "stipulation/proxy.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_fork.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
  ENUMERATOR(STProxy),                                                  \
    ENUMERATOR(STAttackAdapter),   /* switch from generic play to attack play */ \
    ENUMERATOR(STDefenseAdapter),  /* switch from generic play to defense play */ \
    ENUMERATOR(STAttackMoveGenerator), /* unoptimised move generator */ \
    ENUMERATOR(STAttackMove),                                           \
    ENUMERATOR(STAttackFindShortest),                                   \
    ENUMERATOR(STDefenseMove),                                          \
    ENUMERATOR(STReflexAttackerFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexDefenderFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */ \
    ENUMERATOR(STDefenseFork),     /* battle play, continue with subsequent branch */ \
    ENUMERATOR(STDefenseMoveGenerator), /* unoptimised move generator slice */ \
    ENUMERATOR(STReadyForAttack),     /* proxy mark before we start playing attacks */ \
    ENUMERATOR(STReadyForDefense),     /* proxy mark before we start playing defenses */ \
    ENUMERATOR(STBattleDeadEnd), /* stop solving if there are no moves left to be played */ \
    ENUMERATOR(STMinLengthAttackFilter), /* don't even try attacks in less than min_length moves */ \
    ENUMERATOR(STHelpAdapter), /* switch from generic play to help play */ \
    ENUMERATOR(STHelpFindShortest), /* find the shortest solution(s) */ \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpShortcut),    /* selects branch for solving short solutions */        \
    ENUMERATOR(STHelpMove),      /* M-N moves of help play */           \
    ENUMERATOR(STHelpMoveToGoal),  /* last help move reaching goal */   \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STReadyForHelpMove),                                     \
    ENUMERATOR(STReflexHelpFilter),/* stop when wrong side can reach goal */ \
    ENUMERATOR(STSeriesAdapter), /* switch from generic play to series play */ \
    ENUMERATOR(STSeriesFindShortest), /* find the shortest solution(s) */ \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */      \
    ENUMERATOR(STSeriesShortcut),  /* selects branch for solving short solutions */ \
    ENUMERATOR(STSeriesMove),    /* M-N moves of series play */         \
    ENUMERATOR(STSeriesMoveToGoal),   /* last series move reaching goal */ \
    ENUMERATOR(STSeriesDummyMove),    /* dummy move by the side that does *not* play the series */ \
    ENUMERATOR(STReadyForSeriesMove),                                   \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    ENUMERATOR(STReflexSeriesFilter),     /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSetplayFork),                                          \
    ENUMERATOR(STEndOfRoot), /* proxy slice marking the end of the root branch */ \
    ENUMERATOR(STEndOfBranch), /* can leave a branch towards the next one? */ \
    ENUMERATOR(STGoalReachedTesting), /* proxy slice marking the start of goal testing */ \
    ENUMERATOR(STGoalMateReachedTester), /* tests whether a mate goal has been reached */ \
    ENUMERATOR(STGoalStalemateReachedTester), /* tests whether a stalemate goal has been reached */ \
    ENUMERATOR(STGoalDoubleStalemateReachedTester), /* tests whether a double stalemate goal has been reached */ \
    ENUMERATOR(STGoalTargetReachedTester), /* tests whether a target goal has been reached */ \
    ENUMERATOR(STGoalCheckReachedTester), /* tests whether a check goal has been reached */ \
    ENUMERATOR(STGoalCaptureReachedTester), /* tests whether a capture goal has been reached */ \
    ENUMERATOR(STGoalSteingewinnReachedTester), /* tests whether a steingewinn goal has been reached */ \
    ENUMERATOR(STGoalEnpassantReachedTester), /* tests whether an en passant goal has been reached */ \
    ENUMERATOR(STGoalDoubleMateReachedTester), /* tests whether a double mate goal has been reached */ \
    ENUMERATOR(STGoalCounterMateReachedTester), /* tests whether a counter-mate goal has been reached */ \
    ENUMERATOR(STGoalCastlingReachedTester), /* tests whether a castling goal has been reached */ \
    ENUMERATOR(STGoalAutoStalemateReachedTester), /* tests whether an auto-stalemate goal has been reached */ \
    ENUMERATOR(STGoalCircuitReachedTester), /* tests whether a circuit goal has been reached */ \
    ENUMERATOR(STGoalExchangeReachedTester), /* tests whether an exchange goal has been reached */ \
    ENUMERATOR(STGoalCircuitByRebirthReachedTester), /* tests whether a "circuit by rebirth" goal has been reached */ \
    ENUMERATOR(STGoalExchangeByRebirthReachedTester), /* tests whether an "exchange by rebirth" goal has been reached */ \
    ENUMERATOR(STGoalAnyReachedTester), /* tests whether an any goal has been reached */ \
    ENUMERATOR(STGoalProofgameReachedTester), /* tests whether a proof game goal has been reached */ \
    ENUMERATOR(STGoalAToBReachedTester), /* tests whether an "A to B" goal has been reached */ \
    ENUMERATOR(STGoalMateOrStalemateReachedTester), /* just a placeholder - we test using the mate and stalemate testers */ \
    ENUMERATOR(STGoalImmobileReachedTester), /* auxiliary slice testing whether a side is immobile */ \
    ENUMERATOR(STGoalNotCheckReachedTester), /* auxiliary slice enforcing that a side is not in check */ \
    ENUMERATOR(STGoalReachedTested), /* proxy slice marking the end of goal testing */ \
    ENUMERATOR(STLeaf),            /* leaf slice */                     \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    ENUMERATOR(STCheckDetector), /* detect check delivered by previous move */ \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side exposes its king */ \
    ENUMERATOR(STMoveInverter),    /* inverts side to move */ \
    ENUMERATOR(STStipulationReflexAttackSolver), /* solve forced attack after reflex-specific refutation */  \
    ENUMERATOR(STMinLengthGuard), /* make sure that the minimum length of a branch is respected */  \
    ENUMERATOR(STRefutationsAllocator), /* (de)allocate the table holding the refutations */ \
    ENUMERATOR(STTrySolver), /* find battle play solutions */           \
    ENUMERATOR(STPostKeyPlaySuppressor), /* suppresses output of post key play */ \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatStart), /* proxy slice marking where to start solving threats */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STRefutationsCollector), /* collections refutations */   \
    ENUMERATOR(STDoubleMateFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STCounterMateFilter),  /* enforces precondition for counter-mate */ \
    ENUMERATOR(STPrerequisiteOptimiser), /* optimise if prerequisites are not met */ \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STRestartGuard),    /* write move numbers */             \
    ENUMERATOR(STOrthodoxMatingMoveFork),                               \
    ENUMERATOR(STOrthodoxMatingMoveGenerator),                          \
    ENUMERATOR(STKillerMoveCollector), /* remember killer moves */      \
    ENUMERATOR(STKillerMoveAttackGenerator), /* generate attack moves, prioritise killer move (if any) */ \
    ENUMERATOR(STKillerMoveFinalDefenseMove), /* priorise killer move */ \
    ENUMERATOR(STEnPassantFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STCastlingFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STAttackHashed),    /* hash table support for attack */  \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
    ENUMERATOR(STIntelligentHelpFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STIntelligentSeriesFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardHelpFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardSeriesFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
    ENUMERATOR(STKeepMatingFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonChecks), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonTrivialCounter), /* deals with option NonTrivial */ \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
    ENUMERATOR(STMaxThreatLengthHook), /* where should STMaxThreatLength start looking for threats */ \
    ENUMERATOR(STMaxTimeRootDefenderFilter), /* deals with option maxtime */ \
    ENUMERATOR(STMaxTimeDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeHelpFilter), /* deals with option maxtime */    \
    ENUMERATOR(STMaxTimeSeriesFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxSolutionsRootSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsRootDefenderFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsHelpFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSeriesFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STStopOnShortSolutionsInitialiser), /* intialise stoponshortsolutions machinery */  \
    ENUMERATOR(STStopOnShortSolutionsFilter), /* enforce option stoponshortsolutions */  \
    ENUMERATOR(STAmuMateFilter), /* detect whether AMU prevents a mate */ \
    ENUMERATOR(STUltraschachzwangGoalFilter), /* suspend Ultraschachzwang when testing for mate */ \
    ENUMERATOR(STCirceSteingewinnFilter), /* is 'won' piece reborn? */ \
    ENUMERATOR(STCirceCircuitSpecial), /* has reborn capturee made a circuit? */ \
    ENUMERATOR(STCirceExchangeSpecial), /* has reborn capturee made an exchange? */ \
    ENUMERATOR(STAnticirceTargetSquareFilter), /* target square is not reached by capture */ \
    ENUMERATOR(STAnticirceCircuitSpecial), /* special circuit by rebirth */ \
    ENUMERATOR(STAnticirceExchangeSpecial), /* special exchange by rebirth */ \
    ENUMERATOR(STAnticirceExchangeFilter), /* only apply special test after capture in Anticirce */ \
    ENUMERATOR(STPiecesParalysingMateFilter), /* goal not reached because of special rule? */ \
    ENUMERATOR(STPiecesParalysingStalemateSpecial), /* stalemate by special rule? */ \
    ENUMERATOR(STPiecesKamikazeTargetSquareFilter), /* target square not reached because of capture by Kamikaze piece? */ \
    ENUMERATOR(STIllegalSelfcheckWriter), /* indicate illegal self-check in the diagram position */ \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */  \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STContinuationWriter), /* writes battle play continuations */ \
    ENUMERATOR(STKeyWriter), /* write battle play solutions */ \
    ENUMERATOR(STTryWriter), /* write "but" */                          \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STVariationWriter), /* writes variations */              \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextTreeDecorationWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextTreeGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextTreeMoveInversionCounter), /* plain text output, tree mode: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineMoveInversionCounter), /* plain text output, line mode: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineEndOfIntroSeriesMarker), /* handles the end of the intro series */  \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


#define ENUMERATION_TYPENAME branch_level
#define ENUMERATORS \
  ENUMERATOR(toplevel_branch),                  \
    ENUMERATOR(nested_branch)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


Slice slices[max_nr_slices];


/* Keep track of allocated slice indices
 */
static boolean is_slice_index_free[max_nr_slices];


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
  ENUMERATOR(slice_structure_leaf),                              \
    ENUMERATOR(slice_structure_binary),                          \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_branch),                          \
    ENUMERATOR(slice_structure_fork),                            \
    ENUMERATOR(nr_slice_structure_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

static slice_structural_type highest_structural_type[nr_slice_types] =
{
  slice_structure_pipe,   /* STProxy */
  slice_structure_branch, /* STAttackAdapter */
  slice_structure_branch, /* STDefenseAdapter */
  slice_structure_pipe,   /* STAttackMoveGenerator */
  slice_structure_branch, /* STAttackMove */
  slice_structure_branch, /* STAttackFindShortest */
  slice_structure_branch, /* STDefenseMove */
  slice_structure_fork,   /* STReflexAttackerFilter */
  slice_structure_fork,   /* STReflexDefenderFilter */
  slice_structure_fork,   /* STSelfDefense */
  slice_structure_fork,   /* STDefenseFork */
  slice_structure_pipe,   /* STDefenseMoveGenerator */
  slice_structure_branch, /* STReadyForAttack */
  slice_structure_branch, /* STReadyForDefense */
  slice_structure_pipe,   /* STBattleDeadEnd */
  slice_structure_branch, /* STMinLengthAttackFilter */
  slice_structure_branch, /* STHelpAdapter */
  slice_structure_branch, /* STHelpFindShortest */
  slice_structure_branch, /* STHelpRoot */
  slice_structure_fork,   /* STHelpShortcut */
  slice_structure_branch, /* STHelpMove */
  slice_structure_branch, /* STHelpMoveToGoal */
  slice_structure_fork,   /* STHelpFork */
  slice_structure_branch, /* STReadyForHelpMove */
  slice_structure_fork,   /* STReflexHelpFilter */
  slice_structure_branch, /* STSeriesAdapter */
  slice_structure_branch, /* STSeriesFindShortest */
  slice_structure_branch, /* STSeriesRoot */
  slice_structure_fork,   /* STSeriesShortcut */
  slice_structure_branch, /* STSeriesMove */
  slice_structure_branch, /* STSeriesMoveToGoal */
  slice_structure_pipe,   /* STSeriesDummyMove */
  slice_structure_branch, /* STReadyForSeriesMove */
  slice_structure_fork,   /* STSeriesFork */
  slice_structure_fork,   /* STParryFork */
  slice_structure_fork,   /* STReflexSeriesFilter */
  slice_structure_fork,   /* STSetplayFork */
  slice_structure_pipe,   /* STEndOfRoot */
  slice_structure_fork,   /* STEndOfBranch */
  slice_structure_pipe,   /* STGoalReachedTesting */
  slice_structure_pipe,   /* STGoalMateReachedTester */
  slice_structure_pipe,   /* STGoalStalemateReachedTester */
  slice_structure_pipe,   /* STGoalDoubleStalemateReachedTester */
  slice_structure_pipe,   /* STGoalTargetReachedTester */
  slice_structure_pipe,   /* STGoalCheckReachedTester */
  slice_structure_pipe,   /* STGoalCaptureReachedTester */
  slice_structure_pipe,   /* STGoalSteingewinnReachedTester */
  slice_structure_pipe,   /* STGoalEnpassantReachedTester */
  slice_structure_pipe,   /* STGoalDoubleMateReachedTester */
  slice_structure_pipe,   /* STGoalCounterMateReachedTester */
  slice_structure_pipe,   /* STGoalCastlingReachedTester */
  slice_structure_pipe,   /* STGoalAutoStalemateReachedTester */
  slice_structure_pipe,   /* STGoalCircuitReachedTester */
  slice_structure_pipe,   /* STGoalExchangeReachedTester */
  slice_structure_pipe,   /* STGoalCircuitByRebirthReachedTester */
  slice_structure_pipe,   /* STGoalExchangeByRebirthReachedTester */
  slice_structure_pipe,   /* STGoalAnyReachedTester */
  slice_structure_pipe,   /* STGoalProofgameReachedTester */
  slice_structure_pipe,   /* STGoalAToBReachedTester */
  slice_structure_pipe,   /* STGoalMateOrStalemateReachedTester */
  slice_structure_pipe,   /* STGoalImmobileReachedTester */
  slice_structure_pipe,   /* STGoalNotCheckReachedTester */
  slice_structure_pipe,   /* STGoalReachedTested */
  slice_structure_leaf,   /* STLeaf */
  slice_structure_binary, /* STReciprocal */
  slice_structure_binary, /* STQuodlibet */
  slice_structure_pipe,   /* STCheckDetector */
  slice_structure_pipe,   /* STNot */
  slice_structure_pipe,   /* STSelfCheckGuard */
  slice_structure_pipe,   /* STMoveInverter */
  slice_structure_fork,   /* STStipulationReflexAttackSolver */
  slice_structure_branch, /* STMinLengthGuard */
  slice_structure_pipe,   /* STRefutationsAllocator */
  slice_structure_pipe,   /* STTrySolver */
  slice_structure_branch, /* STPostKeyPlaySuppressor */
  slice_structure_branch, /* STContinuationSolver */
  slice_structure_fork,   /* STThreatSolver */
  slice_structure_pipe,   /* STThreatEnforcer */
  slice_structure_pipe,   /* STThreatStart */
  slice_structure_pipe,   /* STThreatCollector */
  slice_structure_pipe,   /* STRefutationsCollector */
  slice_structure_pipe,   /* STDoubleMateFilter */
  slice_structure_pipe,   /* STCounterMateFilter */
  slice_structure_pipe,   /* STPrerequisiteOptimiser */
  slice_structure_pipe,   /* STNoShortVariations */
  slice_structure_pipe,   /* STRestartGuard */
  slice_structure_fork,   /* STOrthodoxMatingMoveFork */
  slice_structure_pipe,   /* STOrthodoxMatingMoveGenerator */
  slice_structure_pipe,   /* STKillerMoveCollector */
  slice_structure_pipe,   /* STKillerMoveAttackGenerator */
  slice_structure_branch, /* STKillerMoveFinalDefenseMove */
  slice_structure_branch, /* STEnPassantFilter */
  slice_structure_branch, /* STCastlingFilter */
  slice_structure_branch, /* STAttackHashed */
  slice_structure_branch, /* STHelpHashed */
  slice_structure_branch, /* STSeriesHashed */
  slice_structure_pipe,   /* STIntelligentHelpFilter */
  slice_structure_pipe,   /* STIntelligentSeriesFilter */
  slice_structure_pipe,   /* STGoalReachableGuardHelpFilter */
  slice_structure_pipe,   /* STGoalReachableGuardSeriesFilter */
  slice_structure_pipe,   /* STIntelligentDuplicateAvoider */
  slice_structure_pipe,   /* STKeepMatingFilter */
  slice_structure_pipe,   /* STMaxFlightsquares */
  slice_structure_pipe,   /* STDegenerateTree */
  slice_structure_pipe,   /* STMaxNrNonTrivial */
  slice_structure_pipe,   /* STMaxNrNonChecks */
  slice_structure_pipe,   /* STMaxNrNonTrivialCounter */
  slice_structure_fork,   /* STMaxThreatLength */
  slice_structure_pipe,   /* STMaxThreatLengthHook */
  slice_structure_pipe,   /* STMaxTimeRootDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeHelpFilter */
  slice_structure_pipe,   /* STMaxTimeSeriesFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootSolvableFilter */
  slice_structure_pipe,   /* STMaxSolutionsSolvableFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootDefenderFilter */
  slice_structure_pipe,   /* STMaxSolutionsHelpFilter */
  slice_structure_pipe,   /* STMaxSolutionsSeriesFilter */
  slice_structure_pipe,   /* STStopOnShortSolutionsInitialiser */
  slice_structure_branch, /* STStopOnShortSolutionsFilter */
  slice_structure_pipe,   /* STAmuMateFilter */
  slice_structure_pipe,   /* STUltraschachzwangGoalFilter */
  slice_structure_pipe,   /* STCirceSteingewinnFilter */
  slice_structure_pipe,   /* STCirceCircuitSpecial */
  slice_structure_pipe,   /* STCirceExchangeSpecial */
  slice_structure_pipe,   /* STAnticirceTargetSquareFilter */
  slice_structure_pipe,   /* STAnticirceCircuitSpecial */
  slice_structure_pipe,   /* STAnticirceExchangeSpecial */
  slice_structure_pipe,   /* STAnticirceExchangeFilter */
  slice_structure_pipe,   /* STPiecesParalysingMateFilter */
  slice_structure_pipe,   /* STPiecesParalysingStalemateSpecial */
  slice_structure_pipe,   /* STPiecesKamikazeTargetSquareFilter */
  slice_structure_pipe,   /* STIllegalSelfcheckWriter */
  slice_structure_pipe,   /* STEndOfPhaseWriter */
  slice_structure_pipe,   /* STEndOfSolutionWriter */
  slice_structure_pipe,   /* STContinuationWriter */
  slice_structure_pipe,   /* STKeyWriter */
  slice_structure_pipe,   /* STTryWriter */
  slice_structure_pipe,   /* STZugzwangWriter */
  slice_structure_branch, /* STVariationWriter */
  slice_structure_pipe,   /* STRefutingVariationWriter */
  slice_structure_pipe,   /* STRefutationWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeCheckWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeDecorationWriter */
  slice_structure_pipe,   /* STOutputPlaintextLineLineWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeGoalWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeMoveInversionCounter */
  slice_structure_pipe,   /* STOutputPlaintextLineMoveInversionCounter */
  slice_structure_pipe    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Determine whether a slice is of some structural type
 * @param si identifies slice
 * @param type identifies type
 * @return true iff slice si has (at least) structural type type
 */
boolean slice_has_structure(slice_index si, slice_structural_type type)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_structural_type,type,"");
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"");
  TraceEnumerator(slice_structural_type,
                  highest_structural_type[slices[si].type],
                  "\n");

  switch (highest_structural_type[slices[si].type])
  {
    case slice_structure_leaf:
      result = type==slice_structure_leaf;
      break;

    case slice_structure_binary:
      result = type==slice_structure_binary;
      break;

    case slice_structure_pipe:
      result = type==slice_structure_pipe;
      break;

    case slice_structure_branch:
      result = type==slice_structure_pipe || type==slice_structure_branch;
      break;

    case slice_structure_fork:
      result = (type==slice_structure_pipe
                || type==slice_structure_branch
                || type==slice_structure_fork);
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

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void)
{
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
  {
    if (!is_slice_index_free[i])
    {
      TraceValue("leaked:%u",i);
      TraceEnumerator(SliceType,slices[i].type,"\n");
    }
    assert(is_slice_index_free[i]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_allocator(void)
{
  slice_index si;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (si = 0; si!=max_nr_slices; ++si)
    is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice index
 * @param type which type
 * @return a so far unused slice index
 */
slice_index alloc_slice(SliceType type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (result = 0; result!=max_nr_slices; ++result)
    if (is_slice_index_free[result])
      break;

  assert(result<max_nr_slices);

  is_slice_index_free[result] = false;

  slices[result].type = type;
  slices[result].starter = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(!is_slice_index_free[si]);
  is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",original);
  TraceFunctionParamListEnd();

  result = alloc_slice(slices[original].type);

  slices[result] = slices[original];
  slice_set_predecessor(result,no_slice);

  TraceEnumerator(Side,slices[original].starter,"");
  TraceEnumerator(Side,slices[result].starter,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",slice);
  TraceFunctionParam("%u",pred);
  TraceFunctionParamListEnd();

  slices[slice].prev = pred;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si)
{
  slice_index i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_traverse_structure(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (st.traversed[i]==slice_traversed)
      dealloc_slice(i);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void move_to_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].prev!=no_slice)
    pipe_unlink(slices[si].prev);
  pipe_link(si,*root_slice);
  *root_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_slice_inserters[] =
{
  { STMoveInverter,         &move_to_root                     },

  { STAttackAdapter,        &move_to_root                     },
  { STReadyForAttack,       &ready_for_attack_make_root       },
  { STEndOfBranch,          &end_of_branch_make_root          },
  { STAttackFindShortest,   &attack_find_shortest_make_root   },
  { STDefenseMove,          &defense_move_make_root           },

  { STHelpAdapter,          &move_to_root                     },
  { STHelpFindShortest,     &help_find_shortest_make_root     },
  { STReadyForHelpMove,     &ready_for_help_move_make_root    },
  { STHelpMove,             &help_move_make_root              },
  { STHelpFork,             &stip_traverse_structure_pipe     },

  { STSeriesAdapter,        &move_to_root                     },
  { STSeriesFindShortest,   &series_find_shortest_make_root   },
  { STReadyForSeriesMove,   &ready_for_series_move_make_root  },
  { STSeriesMove,           &series_move_make_root            },
  { STSeriesFork,           &stip_traverse_structure_pipe     },

  { STLeaf,                 &leaf_make_root                   },
  { STReciprocal,           &binary_make_root                 },
  { STQuodlibet,            &binary_make_root                 },

  { STReflexAttackerFilter, &reflex_attacker_filter_make_root },
  { STReflexHelpFilter,     &reflex_help_filter_make_root     },
  { STReflexSeriesFilter,   &reflex_series_filter_make_root   }
};

enum
{
  nr_root_slice_inserters = (sizeof root_slice_inserters
                             / sizeof root_slice_inserters[0])
};

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void stip_insert_root_slices(slice_index si)
{
  stip_structure_traversal st;
  slice_index root_slice = no_slice;
  slice_index const next = slices[si].u.pipe.next;
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[si].type==STProxy);

  stip_structure_traversal_init(&st,&root_slice);
  for (i = 0; i!=nr_slice_structure_types; ++i)
    stip_structure_traversal_override_by_type(&st,i,&pipe_make_root);
  stip_structure_traversal_override(&st,
                                    root_slice_inserters,
                                    nr_root_slice_inserters);
  stip_traverse_structure(next,&st);

  pipe_link(si,root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_move(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  TraceValue("%u\n",*result);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const get_max_nr_moves_functions[] =
{
  { STAttackMove,             &get_max_nr_moves_move   },
  { STDefenseMove,            &get_max_nr_moves_move   },
  { STHelpMove,               &get_max_nr_moves_move   },
  { STSeriesMove,             &get_max_nr_moves_move   },
  { STSeriesMoveToGoal,       &get_max_nr_moves_move   },
  { STQuodlibet,              &get_max_nr_moves_binary },
  { STReciprocal,             &get_max_nr_moves_binary }
};

enum
{
  nr_get_max_nr_moves_functions = (sizeof get_max_nr_moves_functions
                                   / sizeof get_max_nr_moves_functions[0])
};

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  stip_moves_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  stip_moves_traversal_init(&st,&result);
  stip_moves_traversal_override(&st,
                                get_max_nr_moves_functions,
                                nr_get_max_nr_moves_functions);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  Goal unique_goal;
  boolean is_unique;
} find_unique_goal_state;

static void find_unique_goal_goal(slice_index si,
                                  stip_structure_traversal *st)
{
  find_unique_goal_state * const state = st->param;
  goal_type const goal = slices[si].u.goal_writer.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->unique_goal.type==no_goal)
    state->unique_goal.type = goal;
  else if (state->is_unique && state->unique_goal.type!=goal)
    state->is_unique = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return no_slice if goal is not unique; index of a slice with the
 * unique goal otherwise
 */
Goal find_unique_goal(slice_index si)
{
  stip_structure_traversal st;
  find_unique_goal_state result = { { no_goal, initsquare }, true };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTesting,
                                           &find_unique_goal_goal);
  stip_traverse_structure(si,&st);

  if (!result.is_unique)
    result.unique_goal.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result.unique_goal.type);
  TraceFunctionResultEnd();
  return result.unique_goal;
}

/* Auxiliary data structor for deep_copy: remembers slice copies
 * already made
 */
typedef slice_index copies_type[max_nr_slices];

/* Recursive implementation of in-place deep copying a stipulation
 * sub-tree
 * @param si root of sub-tree
 * @param copies address of array remembering what copies have already
 *               been made
 */
static slice_index deep_copy_recursive(slice_index si, copies_type *copies)
{
  slice_index result = (*copies)[si];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (result==no_slice)
  {
    result = copy_slice(si);
    slices[result].starter = no_side;
    (*copies)[si] = result;

    TraceEnumerator(SliceType,slices[si].type,"\n");
    switch (highest_structural_type[slices[si].type])
    {
      case slice_structure_leaf:
        /* nothing */
        break;

      case slice_structure_pipe:
      case slice_structure_branch:
      {
        slice_index const next = slices[si].u.pipe.next;
        if (next!=no_slice)
        {
          slice_index const next_copy = deep_copy_recursive(next,copies);
          if (slices[next].prev==si)
            pipe_link(result,next_copy);
          else
            pipe_set_successor(result,next_copy);
        }
        break;
      }

      case slice_structure_fork:
      {
        slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
        slice_index const next = slices[si].u.pipe.next;
        if (to_goal!=no_slice)
        {
          slice_index const to_goal_copy = deep_copy_recursive(to_goal,copies);
          slices[result].u.branch_fork.towards_goal = to_goal_copy;
        }
        if (next!=no_slice)
        {
          slice_index const next_copy = deep_copy_recursive(next,copies);
          if (slices[next].prev==si)
            pipe_link(result,next_copy);
          else
            pipe_set_successor(result,next_copy);
        }
        break;
      }

      case slice_structure_binary:
      {
        slice_index const op1 = slices[si].u.binary.op1;
        slice_index const op2 = slices[si].u.binary.op2;
        slices[result].u.binary.op1 = deep_copy_recursive(op1,copies);
        slices[result].u.binary.op2 = deep_copy_recursive(op2,copies);
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si)
{
  copies_type copies;
  slice_index i;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    copies[i] = no_slice;

  result = deep_copy_recursive(si,&copies);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remove a pipe slice being travrsed
 * @param si identifies slice being traversed
 * @param st points to structure holding the state of the traversal
 */
static void remove_pipe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const defense_proxy_removers[] =
{
  { STAttackAdapter, &remove_pipe }
};

enum
{
  nr_defense_proxy_removers = (sizeof defense_proxy_removers
                               / sizeof defense_proxy_removers[0])
};

/* Remove the defensive proxy slices from a branch
 * @param si identifies slice where to start
 */
static void remove_defense_proxies(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    defense_proxy_removers,
                                    nr_defense_proxy_removers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_self_defense(slice_index si,
                                                stip_structure_traversal *st)
{
  slice_index * const proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *proxy_to_goal = stip_deep_copy(slices[si].u.branch_fork.towards_goal);
  remove_defense_proxies(*proxy_to_goal);
  stip_make_direct_goal_branch(*proxy_to_goal);

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_semi_reflex(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index * const new_proxy_to_goal = st->param;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
    slice_index const testing = branch_find_slice(STGoalReachedTesting,to_goal);
    slice_index const tester = slices[testing].u.pipe.next;
    slice_index const new_leaf = alloc_leaf_slice();
    slice_index const new_testing = copy_slice(testing);
    slice_index const new_tester = copy_slice(tester);
    slice_index const new_tested = alloc_pipe(STGoalReachedTested);

		/* The starting side of the copy may be different that that of the original!
		 * -> prevent the copied starter from determining the starting side of the
		 * entire problem. */
    slices[new_testing].starter = no_side;
    slices[new_tester].starter = no_side;

    pipe_link(new_testing,new_tester);
    pipe_link(new_tester,new_tested);
    pipe_link(new_tested,new_leaf);
    *new_proxy_to_goal = alloc_proxy_slice();
    pipe_link(*new_proxy_to_goal,new_testing);

    stip_make_direct_goal_branch(*new_proxy_to_goal);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_direct_guards(slice_index si,
                                 stip_structure_traversal *st)
{
  slice_index const * const proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.branch.length>slack_length_battle
      && *proxy_to_goal!=no_slice)
  {
    boolean const append_deadend = false;
    battle_branch_set_direct_goal_branch(si,*proxy_to_goal,append_deadend);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_branch_fork(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't recurse towards goal */
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors to_quodlibet_transformers[] =
{
  { STReadyForAttack,       &insert_direct_guards                },
  { STHelpFork,             &transform_to_quodlibet_branch_fork  },
  { STSeriesFork,           &transform_to_quodlibet_branch_fork  },
  { STNot,                  &stip_structure_visitor_noop         },
  { STReflexDefenderFilter, &transform_to_quodlibet_semi_reflex  },
  { STSelfDefense,          &transform_to_quodlibet_self_defense }
};

enum
{
  nr_to_quodlibet_transformers = (sizeof to_quodlibet_transformers
                                  / sizeof to_quodlibet_transformers[0])
};

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal.
 * @param si identifies slice where to start
 * @return true iff quodlibet could be applied
 */
boolean transform_to_quodlibet(slice_index si)
{
  stip_structure_traversal st;
  slice_index proxy_to_goal = no_slice;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&proxy_to_goal);
  stip_structure_traversal_override(&st,
                                    to_quodlibet_transformers,
                                    nr_to_quodlibet_transformers);
  stip_traverse_structure(si,&st);

  result = proxy_to_goal!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}


/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
static void trash_for_postkey_play(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  pipe_unlink(slices[si].prev);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors to_postkey_play_reducers[] =
{
  { STStipulationReflexAttackSolver, &trash_for_postkey_play                        },
  { STAttackAdapter,                 &trash_for_postkey_play                        },
  { STReadyForAttack,                &trash_for_postkey_play                        },
  { STBattleDeadEnd,                 &trash_for_postkey_play                        },
  { STMinLengthAttackFilter,         &trash_for_postkey_play                        },
  { STAttackMoveGenerator,           &trash_for_postkey_play                        },
  { STAttackMove,                    &trash_for_postkey_play                        },
  { STEndOfBranch,                   &end_of_branch_reduce_to_postkey_play          },
  { STContinuationSolver,            &trash_for_postkey_play                        },
  { STMinLengthGuard,                &trash_for_postkey_play                        },
  { STReflexDefenderFilter,          &reflex_defender_filter_reduce_to_postkey_play },
  { STReadyForDefense,               &ready_for_defense_reduce_to_postkey_play      }
};

enum
{
  nr_to_postkey_play_reducers = (sizeof to_postkey_play_reducers
                                 / sizeof to_postkey_play_reducers[0])
};

/* Install the slice representing the postkey slice at the stipulation
 * root
 * @param postkey_slice identifies slice to be installed
 */
static void install_postkey_slice(slice_index si, slice_index postkey_slice)
{
  slice_index inverter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",postkey_slice);
  TraceFunctionParamListEnd();

  inverter = alloc_move_inverter_slice();
  pipe_link(inverter,postkey_slice);
  pipe_link(si,inverter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Attempt to apply the postkey play option to the current stipulation
 * @param si identifies slice where to start
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean stip_apply_postkeyplay(slice_index si)
{
  boolean result;
  slice_index postkey_slice = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&postkey_slice);
  stip_structure_traversal_override(&st,
                                    to_postkey_play_reducers,
                                    nr_to_postkey_play_reducers);
  stip_traverse_structure(si,&st);

  TraceValue("%u\n",postkey_slice);
  if (postkey_slice==no_slice)
    result = false;
  else
  {
    install_postkey_slice(si,postkey_slice);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static structure_traversers_visitors setplay_makers[] =
{
  { STReadyForDefense,      &ready_for_defense_make_setplay_slice            },
  { STDefenseMove,          &defense_move_make_setplay_slice                 },

  { STReadyForHelpMove,     &ready_for_help_move_make_setplay_slice          },
  { STHelpFork,             &stip_traverse_structure_pipe                    },
  { STHelpMove,             &help_move_make_setplay_slice                    },

  { STSeriesDummyMove,      &stip_structure_visitor_noop                     },
  { STSeriesFork,           &series_fork_make_setplay                        },

  { STReflexDefenderFilter, &reflex_guard_defender_filter_make_setplay_slice }
};

enum
{
  nr_setplay_makers = (sizeof setplay_makers / sizeof setplay_makers[0])
};

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index stip_make_setplay(slice_index si)
{
  slice_index result = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,setplay_makers,nr_setplay_makers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static structure_traversers_visitors setplay_appliers[] =
{
  { STAttackMove,                    &attack_move_apply_setplay    },
  { STDefenseAdapter,                &stip_structure_visitor_noop  },
  { STStipulationReflexAttackSolver, &stip_traverse_structure_pipe },

  { STHelpShortcut,                  &stip_traverse_structure_pipe },
  { STHelpMove,                      &help_move_apply_setplay      },
  { STHelpFork,                      &stip_structure_visitor_noop  },

  { STSeriesShortcut,                &stip_traverse_structure_pipe },
  { STSeriesMove,                    &series_move_apply_setplay    },
  { STSeriesFork,                    &stip_structure_visitor_noop  },

  { STMoveInverter,                  &move_inverter_apply_setplay  }
};

enum
{
  nr_setplay_appliers = (sizeof setplay_appliers / sizeof setplay_appliers[0])
};

/* Combine the set play slices into the current stipulation
 * @param setplay slice index of set play
 */
static void insert_set_play(slice_index si, slice_index setplay_slice)
{
  slice_index set;
  slice_index regular;
  slice_index set_fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",setplay_slice);
  TraceFunctionParamListEnd();

  set = alloc_move_inverter_slice();
  link_to_branch(set,setplay_slice);

  if (slices[next].prev==si)
    regular = next;
  else
  {
    regular = alloc_proxy_slice();
    pipe_set_successor(regular,next);
  }

  set_fork = alloc_setplay_fork_slice(set);

  pipe_link(si,set_fork);
  pipe_link(set_fork,regular);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Attempt to add set play to the stipulation
 * @param si identifies the root from which to apply set play
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(slice_index si)
{
  boolean result;
  slice_index setplay_slice = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&setplay_slice);
  stip_structure_traversal_override(&st,setplay_appliers,nr_setplay_appliers);
  stip_traverse_structure_pipe(si,&st);

  if (setplay_slice==no_slice)
    result = false;
  else
  {
    insert_set_play(si,setplay_slice);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

typedef struct
{
    goal_type const goal_type;
    boolean result;
} goal_search;

static void ends_in_goal(slice_index si, stip_structure_traversal *st)
{
  goal_search * const search = st->param;
  goal_type const goal = slices[si].u.goal_writer.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  search->result = search->result || search->goal_type==goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Does the current stipulation end in a specific goal?
 * @param si identifies slice where to start
 * @param goal identifies the goal
 * @return true iff one of the goals of the stipulation is goal
 */
boolean stip_ends_in(slice_index si, goal_type goal)
{
  goal_search search = { goal, false };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&search);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTesting,
                                           &ends_in_goal);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",search.result);
  TraceFunctionResultEnd();
  return search.result;
}

static structure_traversers_visitors starter_detectors[] =
{
  { STAttackMove,       &attack_move_detect_starter   },
  { STDefenseMove,      &defense_move_detect_starter  },
  { STHelpMove,         &help_move_detect_starter     },
  { STSeriesMove,       &series_move_detect_starter   },
  { STSeriesMoveToGoal, &series_move_detect_starter   },
  { STSeriesDummyMove,  &series_move_detect_starter   },
  { STReciprocal,       &reci_detect_starter          },
  { STQuodlibet,        &quodlibet_detect_starter     },
  { STMoveInverter,     &move_inverter_detect_starter },
  { STHelpShortcut,     &pipe_detect_starter          },
  { STSeriesShortcut,   &pipe_detect_starter          },
  { STParryFork,        &pipe_detect_starter          },
  /* .to_attacker has different starter -> detect starter from .next
   * only */
  { STThreatSolver,     &pipe_detect_starter          },
  { STMaxThreatLength,  &pipe_detect_starter          }
};

enum
{
  nr_starter_detectors = (sizeof starter_detectors
                          / sizeof starter_detectors[0])
};

/* Detect the starting side from the stipulation
 * @param si identifies slice whose starter to find
 */
void stip_detect_starter(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,NULL);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_fork,
                                            &branch_fork_detect_starter);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_pipe,
                                            &pipe_detect_starter);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_branch,
                                            &pipe_detect_starter);
  stip_structure_traversal_override(&st,
                                    starter_detectors,
                                    nr_starter_detectors);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Impose the starting side on a stipulation
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
static void default_impose_starter(slice_index si,
                                   stip_structure_traversal *st)
{
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
static void impose_inverted_starter(slice_index si,
                                    stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  *starter = advers(*starter);
  stip_traverse_structure_children(si,st);
  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice types that change the starting side
 */
static SliceType starter_inverters[] =
{
  STAttackMove,
  STDefenseMove,
  STHelpMove,
  STHelpMoveToGoal,
  STSeriesMove,
  STSeriesMoveToGoal,
  STSeriesDummyMove,
  STMoveInverter
};

enum
{
  nr_starter_inverters = (sizeof starter_inverters
                          / sizeof starter_inverters[0])
};

/* Set the starting side of the stipulation
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 */
void stip_impose_starter(slice_index si, Side starter)
{
  stip_structure_traversal st;
  unsigned int i;
  SliceType type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&starter);

  for (type = 0; type!=nr_slice_types; ++type)
    stip_structure_traversal_override_single(&st,
                                             type,
                                             &default_impose_starter);

  for (i = 0; i!=nr_starter_inverters; ++i)
    stip_structure_traversal_override_single(&st,
                                             starter_inverters[i],
                                             &impose_inverted_starter);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Dispatch a slice structure operation to a slice based on its type
 * @param si identifies slice
 * @param ops contains addresses of visitors per slice type
 * @param st address of structure defining traversal
 */
static
void stip_structure_visit_slice(slice_index si,
                                stip_structure_visitor (*ops)[nr_slice_types],
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    stip_structure_visitor const operation = (*ops)[slices[si].type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Query the traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",st->traversed[si]);
  TraceFunctionResultEnd();
  return st->traversed[si];
}

/* (Approximately) depth-first traversal of the stipulation
 * @param ops mapping from slice types to operations
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (root!=no_slice)
    if (st->traversed[root]==slice_not_traversed)
    {
      /* avoid infinite recursion */
      st->traversed[root] = slice_being_traversed;
      stip_structure_visit_slice(root,&st->map.visitors,st);
      st->traversed[root] = slice_traversed;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor structure_children_traversers[] =
{
  &stip_traverse_structure_pipe,            /* STProxy */
  &stip_traverse_structure_pipe,            /* STAttackAdapter */
  &stip_traverse_structure_pipe,            /* STDefenseAdapter */
  &stip_traverse_structure_pipe,            /* STAttackMoveGenerator */
  &stip_traverse_structure_pipe,            /* STAttackMove */
  &stip_traverse_structure_pipe,            /* STAttackFindShortest */
  &stip_traverse_structure_pipe,            /* STDefenseMove */
  &stip_traverse_structure_reflex_filter,   /* STReflexAttackerFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexDefenderFilter */
  &stip_traverse_structure_battle_fork,     /* STSelfDefense */
  &stip_traverse_structure_battle_fork,     /* STDefenseFork */
  &stip_traverse_structure_pipe,            /* STDefenseMoveGenerator */
  &stip_traverse_structure_pipe,            /* STReadyForAttack */
  &stip_traverse_structure_pipe,            /* STReadyForDefense */
  &stip_traverse_structure_pipe,            /* STBattleDeadEnd */
  &stip_traverse_structure_pipe,            /* STMinLengthAttackFilter */
  &stip_traverse_structure_pipe,            /* STHelpAdapter */
  &stip_traverse_structure_pipe,            /* STHelpFindShortest */
  &stip_traverse_structure_pipe,            /* STHelpRoot */
  &stip_traverse_structure_help_shortcut,   /* STHelpShortcut */
  &stip_traverse_structure_pipe,            /* STHelpMove */
  &stip_traverse_structure_pipe,            /* STHelpMoveToGoal */
  &stip_traverse_structure_help_fork,       /* STHelpFork */
  &stip_traverse_structure_pipe,            /* STReadyForHelpMove */
  &stip_traverse_structure_reflex_filter,   /* STReflexHelpFilter */
  &stip_traverse_structure_pipe,            /* STSeriesAdapter */
  &stip_traverse_structure_pipe,            /* STSeriesFindShortest */
  &stip_traverse_structure_pipe,            /* STSeriesRoot */
  &stip_traverse_structure_series_shortcut, /* STSeriesShortcut */
  &stip_traverse_structure_pipe,            /* STSeriesMove */
  &stip_traverse_structure_pipe,            /* STSeriesMoveToGoal */
  &stip_traverse_structure_pipe,            /* STSeriesDummyMove */
  &stip_traverse_structure_pipe,            /* STReadyForSeriesMove */
  &stip_traverse_structure_series_fork,     /* STSeriesFork */
  &stip_traverse_structure_parry_fork,      /* STParryFork */
  &stip_traverse_structure_reflex_filter,   /* STReflexSeriesFilter */
  &stip_traverse_structure_setplay_fork,    /* STSetplayFork */
  &stip_traverse_structure_pipe,            /* STEndOfRoot */
  &stip_traverse_structure_battle_fork,     /* STEndOfBranch */
  &stip_traverse_structure_pipe,            /* STGoalReachedTesting */
  &stip_traverse_structure_pipe,            /* STGoalMateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalStalemateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalDoubleStalemateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalTargetReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCheckReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCaptureReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalSteingewinnReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalEnpassantReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalDoubleMateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCounterMateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCastlingReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalAutoStalemateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCircuitReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalExchangeReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalCircuitByRebirthReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalExchangeByRebirthReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalAnyReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalProofgameReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalAToBReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalMateOrStalemateReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalImmobileReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalNotCheckReachedTester */
  &stip_traverse_structure_pipe,            /* STGoalReachedTested */
  &stip_structure_visitor_noop,             /* STLeaf */
  &stip_traverse_structure_binary,          /* STReciprocal */
  &stip_traverse_structure_binary,          /* STQuodlibet */
  &stip_traverse_structure_pipe,            /* STCheckDetector */
  &stip_traverse_structure_pipe,            /* STNot */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuard */
  &stip_traverse_structure_pipe,            /* STMoveInverter */
  &stip_traverse_structure_battle_fork,     /* STStipulationReflexAttackSolver */
  &stip_traverse_structure_pipe,            /* STMinLengthGuard */
  &stip_traverse_structure_pipe,            /* STRefutationsAllocator */
  &stip_traverse_structure_pipe,            /* STTrySolver */
  &stip_traverse_structure_pipe,            /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_pipe,            /* STContinuationSolver */
  &stip_traverse_structure_battle_fork,     /* STThreatSolver */
  &stip_traverse_structure_pipe,            /* STThreatEnforcer */
  &stip_traverse_structure_pipe,            /* STThreatStart */
  &stip_traverse_structure_pipe,            /* STThreatCollector */
  &stip_traverse_structure_pipe,            /* STRefutationsCollector */
  &stip_traverse_structure_pipe,            /* STDoubleMateFilter */
  &stip_traverse_structure_pipe,            /* STCounterMateFilter */
  &stip_traverse_structure_pipe,            /* STPrerequisiteOptimiser */
  &stip_traverse_structure_pipe,            /* STNoShortVariations */
  &stip_traverse_structure_pipe,            /* STRestartGuard */
  &stip_traverse_structure_battle_fork,     /* STOrthodoxMatingMoveFork */
  &stip_traverse_structure_pipe,            /* STOrthodoxMatingMoveGenerator */
  &stip_traverse_structure_pipe,            /* STKillerMoveCollector */
  &stip_traverse_structure_pipe,            /* STKillerMoveAttackGenerator */
  &stip_traverse_structure_pipe,            /* STKillerMoveFinalDefenseMove */
  &stip_traverse_structure_pipe,            /* STEnPassantFilter */
  &stip_traverse_structure_pipe,            /* STCastlingFilter */
  &stip_traverse_structure_pipe,            /* STAttackHashed */
  &stip_traverse_structure_pipe,            /* STHelpHashed */
  &stip_traverse_structure_pipe,            /* STSeriesHashed */
  &stip_traverse_structure_pipe,            /* STIntelligentHelpFilter */
  &stip_traverse_structure_pipe,            /* STIntelligentSeriesFilter */
  &stip_traverse_structure_pipe,            /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_pipe,            /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_pipe,            /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_pipe,            /* STKeepMatingFilter */
  &stip_traverse_structure_pipe,            /* STMaxFlightsquares */
  &stip_traverse_structure_pipe,            /* STDegenerateTree */
  &stip_traverse_structure_pipe,            /* STMaxNrNonTrivial */
  &stip_traverse_structure_pipe,            /* STMaxNrNonChecks */
  &stip_traverse_structure_pipe,            /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_pipe,            /* STMaxThreatLength */
  &stip_traverse_structure_pipe,            /* STMaxThreatLengthHook */
  &stip_traverse_structure_pipe,            /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeHelpFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_pipe,            /* STStopOnShortSolutionsInitialiser */
  &stip_traverse_structure_pipe,            /* STStopOnShortSolutionsFilter */
  &stip_traverse_structure_pipe,            /* STAmuMateFilter */
  &stip_traverse_structure_pipe,            /* STUltraschachzwangGoalFilter */
  &stip_traverse_structure_pipe,            /* STCirceSteingewinnFilter */
  &stip_traverse_structure_pipe,            /* STCirceCircuitSpecial */
  &stip_traverse_structure_pipe,            /* STCirceExchangeSpecial */
  &stip_traverse_structure_pipe,            /* STAnticirceTargetSquareFilter */
  &stip_traverse_structure_pipe,            /* STAnticirceCircuitSpecial */
  &stip_traverse_structure_pipe,            /* STAnticirceExchangeSpecial */
  &stip_traverse_structure_pipe,            /* STAnticirceExchangeFilter */
  &stip_traverse_structure_pipe,            /* STPiecesParalysingMateFilter */
  &stip_traverse_structure_pipe,            /* STPiecesParalysingStalemateSpecial */
  &stip_traverse_structure_pipe,            /* STPiecesKamikazeTargetSquareFilter */
  &stip_traverse_structure_pipe,            /* STIllegalSelfcheckWriter */
  &stip_traverse_structure_pipe,            /* STEndOfPhaseWriter */
  &stip_traverse_structure_pipe,            /* STEndOfSolutionWriter */
  &stip_traverse_structure_pipe,            /* STContinuationWriter */
  &stip_traverse_structure_pipe,            /* STKeyWriter */
  &stip_traverse_structure_pipe,            /* STTryWriter */
  &stip_traverse_structure_pipe,            /* STZugzwangWriter */
  &stip_traverse_structure_pipe,            /* STVariationWriter */
  &stip_traverse_structure_pipe,            /* STRefutingVariationWriter */
  &stip_traverse_structure_pipe,            /* STRefutationWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeCheckWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeDecorationWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_pipe             /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st, void *param)
{
  unsigned int i;
  for (i = 0; i!=max_nr_slices; ++i)
    st->traversed[i] = slice_not_traversed;

  for (i = 0; i!=nr_slice_types; ++i)
    st->map.visitors[i] = structure_children_traversers[i];

  st->param = param;
}

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_type(stip_structure_traversal *st,
                                               slice_structural_type type,
                                               stip_structure_visitor visitor)
{
  unsigned int i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (highest_structural_type[i]==type)
      st->map.visitors[i] = visitor;
}

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_single(stip_structure_traversal *st,
                                              SliceType type,
                                              stip_structure_visitor visitor)
{
  st->map.visitors[type] = visitor;
}

/* Override some of the visitors of a traversal
 * @param st to be initialised
 * @param visitors overriding visitors
 * @param nr_visitors number of visitors
 */
void
stip_structure_traversal_override(stip_structure_traversal *st,
                                  structure_traversers_visitors const visitors[],
                                  unsigned int nr_visitors)
{
  unsigned int i;
  for (i = 0; i!=nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_visit_slice(si,&structure_children_traversers,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_visitor_map_type const moves_children_traversers =
{
  {
    &stip_traverse_moves_pipe,                  /* STProxy */
    &stip_traverse_moves_battle_adapter_slice,  /* STAttackAdapter */
    &stip_traverse_moves_battle_adapter_slice,  /* STDefenseAdapter */
    &stip_traverse_moves_pipe,                  /* STAttackMoveGenerator */
    &stip_traverse_moves_move_slice,            /* STAttackMove */
    &stip_traverse_moves_pipe,                  /* STAttackFindShortest */
    &stip_traverse_moves_move_slice,            /* STDefenseMove */
    &stip_traverse_moves_battle_fork,           /* STReflexAttackerFilter */
    &stip_traverse_moves_battle_fork,           /* STReflexDefenderFilter */
    &stip_traverse_moves_battle_fork,           /* STSelfDefense */
    &stip_traverse_moves_defense_fork,          /* STDefenseFork */
    &stip_traverse_moves_pipe,                  /* STDefenseMoveGenerator */
    &stip_traverse_moves_pipe,                  /* STReadyForAttack */
    &stip_traverse_moves_pipe,                  /* STReadyForDefense */
    &stip_traverse_moves_battle_play_dead_end,  /* STBattleDeadEnd */
    &stip_traverse_moves_pipe,                  /* STMinLengthAttackFilter */
    &stip_traverse_moves_help_adapter_slice,    /* STHelpAdapter */
    &stip_traverse_moves_pipe,                  /* STHelpFindShortest */
    &stip_traverse_moves_pipe,                  /* STHelpRoot */
    &stip_traverse_moves_help_shortcut,         /* STHelpShortcut */
    &stip_traverse_moves_move_slice,            /* STHelpMove */
    &stip_traverse_moves_move_slice,            /* STHelpMoveToGoal */
    &stip_traverse_moves_help_fork,             /* STHelpFork */
    &stip_traverse_moves_pipe,                  /* STReadyForHelpMove */
    &stip_traverse_moves_help_fork,             /* STReflexHelpFilter */
    &stip_traverse_moves_series_adapter_slice,  /* STSeriesAdapter */
    &stip_traverse_moves_pipe,                  /* STSeriesFindShortest */
    &stip_traverse_moves_pipe,                  /* STSeriesRoot */
    &stip_traverse_moves_series_shortcut,       /* STSeriesShortcut */
    &stip_traverse_moves_move_slice,            /* STSeriesMove */
    &stip_traverse_moves_move_slice,            /* STSeriesMoveToGoal */
    &stip_traverse_moves_move_slice,            /* STSeriesDummyMove */
    &stip_traverse_moves_pipe,                  /* STReadyForSeriesMove */
    &stip_traverse_moves_series_fork,           /* STSeriesFork */
    &stip_traverse_moves_parry_fork,            /* STParryFork */
    &stip_traverse_moves_reflex_series_filter,  /* STReflexSeriesFilter */
    &stip_traverse_moves_setplay_fork,          /* STSetplayFork */
    &stip_traverse_moves_pipe,                  /* STEndOfRoot */
    &stip_traverse_moves_end_of_branch,         /* STEndOfBranch */
    &stip_traverse_moves_pipe,                  /* STGoalReachedTesting */
    &stip_traverse_moves_pipe,                  /* STGoalMateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalStalemateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalDoubleStalemateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalTargetReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCheckReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCaptureReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalSteingewinnReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalEnpassantReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalDoubleMateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCounterMateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCastlingReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalAutoStalemateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCircuitReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalExchangeReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalCircuitByRebirthReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalExchangeByRebirthReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalAnyReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalProofgameReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalAToBReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalMateOrStalemateReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalImmobileReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalNotCheckReachedTester */
    &stip_traverse_moves_pipe,                  /* STGoalReachedTested */
    &stip_traverse_moves_noop,                  /* STLeaf */
    &stip_traverse_moves_binary,                /* STReciprocal */
    &stip_traverse_moves_binary,                /* STQuodlibet */
    &stip_traverse_moves_pipe,                  /* STCheckDetector */
    &stip_traverse_moves_pipe,                  /* STNot */
    &stip_traverse_moves_pipe,                  /* STSelfCheckGuard */
    &stip_traverse_moves_pipe,                  /* STMoveInverter */
    &stip_traverse_moves_reflex_attacker_solver,/* STStipulationReflexAttackSolver */
    &stip_traverse_moves_pipe,                  /* STMinLengthGuard */
    &stip_traverse_moves_pipe,                  /* STRefutationsAllocator */
    &stip_traverse_moves_pipe,                  /* STTrySolver */
    &stip_traverse_moves_pipe,                  /* STPostKeyPlaySuppressor */
    &stip_traverse_moves_pipe,                  /* STContinuationSolver */
    &stip_traverse_moves_pipe,                  /* STThreatSolver */
    &stip_traverse_moves_pipe,                  /* STThreatEnforcer */
    &stip_traverse_moves_pipe,                  /* STThreatStart */
    &stip_traverse_moves_pipe,                  /* STThreatCollector */
    &stip_traverse_moves_pipe,                  /* STRefutationsCollector */
    &stip_traverse_moves_pipe,                  /* STDoubleMateFilter */
    &stip_traverse_moves_pipe,                  /* STCounterMateFilter */
    &stip_traverse_moves_pipe,                  /* STPrerequisiteOptimiser */
    &stip_traverse_moves_pipe,                  /* STNoShortVariations */
    &stip_traverse_moves_pipe,                  /* STRestartGuard */
    &stip_traverse_moves_orthodox_mating_move_fork,           /* STOrthodoxMatingMoveFork */
    &stip_traverse_moves_pipe,                  /* STOrthodoxMatingMoveGenerator */
    &stip_traverse_moves_pipe,                  /* STKillerMoveCollector */
    &stip_traverse_moves_pipe,                  /* STKillerMoveAttackGenerator */
    &stip_traverse_moves_pipe,                  /* STKillerMoveFinalDefenseMove */
    &stip_traverse_moves_pipe,                  /* STEnPassantFilter */
    &stip_traverse_moves_pipe,                  /* STCastlingFilter */
    &stip_traverse_moves_pipe,                  /* STAttackHashed */
    &stip_traverse_moves_pipe,                  /* STHelpHashed */
    &stip_traverse_moves_pipe,                  /* STSeriesHashed */
    &stip_traverse_moves_pipe,                  /* STIntelligentHelpFilter */
    &stip_traverse_moves_pipe,                  /* STIntelligentSeriesFilter */
    &stip_traverse_moves_pipe,                  /* STGoalReachableGuardHelpFilter */
    &stip_traverse_moves_pipe,                  /* STGoalReachableGuardSeriesFilter */
    &stip_traverse_moves_pipe,                  /* STIntelligentDuplicateAvoider */
    &stip_traverse_moves_pipe,                  /* STKeepMatingFilter */
    &stip_traverse_moves_pipe,                  /* STMaxFlightsquares */
    &stip_traverse_moves_pipe,                  /* STDegenerateTree */
    &stip_traverse_moves_pipe,                  /* STMaxNrNonTrivial */
    &stip_traverse_moves_pipe,                  /* STMaxNrNonChecks */
    &stip_traverse_moves_pipe,                  /* STMaxNrNonTrivialCounter */
    &stip_traverse_moves_pipe,                  /* STMaxThreatLength */
    &stip_traverse_moves_pipe,                  /* STMaxThreatLengthHook */
    &stip_traverse_moves_pipe,                  /* STMaxTimeRootDefenderFilter */
    &stip_traverse_moves_pipe,                  /* STMaxTimeDefenderFilter */
    &stip_traverse_moves_pipe,                  /* STMaxTimeHelpFilter */
    &stip_traverse_moves_pipe,                  /* STMaxTimeSeriesFilter */
    &stip_traverse_moves_pipe,                  /* STMaxSolutionsRootSolvableFilter */
    &stip_traverse_moves_pipe,                  /* STMaxSolutionsSolvableFilter */
    &stip_traverse_moves_pipe,                  /* STMaxSolutionsRootDefenderFilter */
    &stip_traverse_moves_pipe,                  /* STMaxSolutionsHelpFilter */
    &stip_traverse_moves_pipe,                  /* STMaxSolutionsSeriesFilter */
    &stip_traverse_moves_pipe,                  /* STStopOnShortSolutionsInitialiser */
    &stip_traverse_moves_pipe,                  /* STStopOnShortSolutionsFilter */
    &stip_traverse_moves_pipe,                  /* STAmuMateFilter */
    &stip_traverse_moves_pipe,                  /* STUltraschachzwangGoalFilter */
    &stip_traverse_moves_pipe,                  /* STCirceSteingewinnFilter */
    &stip_traverse_moves_pipe,                  /* STCirceCircuitSpecial */
    &stip_traverse_moves_pipe,                  /* STCirceExchangeSpecial */
    &stip_traverse_moves_pipe,                  /* STAnticirceTargetSquareFilter */
    &stip_traverse_moves_pipe,                  /* STAnticirceCircuitSpecial */
    &stip_traverse_moves_pipe,                  /* STAnticirceExchangeSpecial */
    &stip_traverse_moves_pipe,                  /* STAnticirceExchangeFilter */
    &stip_traverse_moves_pipe,                  /* STPiecesParalysingMateFilter */
    &stip_traverse_moves_pipe,                  /* STPiecesParalysingStalemateSpecial */
    &stip_traverse_moves_pipe,                  /* STPiecesKamikazeTargetSquareFilter */
    &stip_traverse_moves_pipe,                  /* STIllegalSelfcheckWriter */
    &stip_traverse_moves_pipe,                  /* STEndOfPhaseWriter */
    &stip_traverse_moves_pipe,                  /* STEndOfSolutionWriter */
    &stip_traverse_moves_pipe,                  /* STContinuationWriter */
    &stip_traverse_moves_pipe,                  /* STKeyWriter */
    &stip_traverse_moves_pipe,                  /* STTryWriter */
    &stip_traverse_moves_pipe,                  /* STZugzwangWriter */
    &stip_traverse_moves_pipe,                  /* STVariationWriter */
    &stip_traverse_moves_pipe,                  /* STRefutingVariationWriter */
    &stip_traverse_moves_pipe,                  /* STRefutationWriter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeCheckWriter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeDecorationWriter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextLineLineWriter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeGoalWriter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeMoveInversionCounter */
    &stip_traverse_moves_pipe,                  /* STOutputPlaintextLineMoveInversionCounter */
    &stip_traverse_moves_pipe                   /* STOutputPlaintextLineEndOfIntroSeriesMarker */
  }
};

/* Initialise a move traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st, void *param)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map = moves_children_traversers;

  for (i = 0; i!=nr_slice_types; ++i)
    st->map.visitors[i] = moves_children_traversers.visitors[i];

  st->level = 0;
  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->param = param;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the number of moves at the start of the traversal. Normally, this is
 * determined while traversing the stipulation. Only invoke
 * stip_moves_traversal_set_remaining() when the traversal is started in the
 * middle of a stipulation.
 * @param st to be initialised
 * @param remaining number of remaining moves (without slack)
 * @param full_length full number of moves of the initial branch (without slack)
 */
void stip_moves_traversal_set_remaining(stip_moves_traversal *st,
                                        stip_length_type remaining,
                                        stip_length_type full_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->remaining = remaining;
  st->full_length = full_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at some slice types
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 */
void stip_moves_traversal_override(stip_moves_traversal *st,
                                   moves_traversers_visitors const visitors[],
                                   unsigned int nr_visitors)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          SliceType type,
                                          stip_moves_visitor visitor)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map.visitors[type] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of the stipulation
 * @param root start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->remaining);

  TraceEnumerator(SliceType,slices[root].type,"\n");
  assert(slices[root].type<=nr_slice_types);

  {
    stip_moves_visitor const operation = st->map.visitors[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No-op callback for move traversals
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_noop(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    SliceType const type = slices[si].type;
    stip_moves_visitor const operation = moves_children_traversers.visitors[type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
