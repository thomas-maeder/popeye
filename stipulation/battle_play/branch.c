#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_move_played.h"
#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types appear in battle branches
 * some types are not mentioned because they have variable ranks.
 */
static slice_index const slice_rank_order[] =
{
  STReflexDefenderFilter,
  STReadyForDefense,
  STThreatSolver,
  STDefenseFork,
  STDefenseMove,
  STDefenseMovePlayed,
  STSeriesMovePlayed,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STRefutationWriter,
  STDefenseMoveShoeHorningDone,
  STKillerMoveCollector,
  STSelfDefense,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STGoalMateReachedTester,
  STGoalStalemateReachedTester,
  STGoalDoubleStalemateReachedTester,
  STAnticirceTargetSquareFilter,
  STGoalTargetReachedTester,
  STGoalCaptureReachedTester,
  STCirceSteingewinnFilter,
  STGoalSteingewinnReachedTester,
  STGoalEnpassantReachedTester,
  STGoalDoubleMateReachedTester,
  STGoalCounterMateReachedTester,
  STGoalCastlingReachedTester,
  STGoalAutoStalemateReachedTester,
  STGoalCircuitReachedTester,
  STAnticirceExchangeFilter,
  STGoalExchangeReachedTester,
  STCirceCircuitSpecial,
  STGoalCircuitByRebirthReachedTester,
  STCirceExchangeSpecial,
  STGoalExchangeByRebirthReachedTester,
  STGoalAnyReachedTester,
  STGoalProofgameReachedTester,
  STGoalAToBReachedTester,
  STGoalMateOrStalemateReachedTester,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STGoalNotCheckReachedTester,
  STGoalImmobileReachedTester,
  STGoalReachedTested,
  STDefenseMoveLegalityChecked,
  STSeriesMoveLegalityChecked,
  STNoShortVariations,
  STAttackHashed,
  STThreatEnforcer,
  STKeepMatingGuardAttackerFilter,
  STDefenseMoveFiltered,
  STVariationWriter,
  STRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STDefenseDealtWith,
  STStipulationReflexAttackSolver,
  STReadyForAttack,
  STReflexAttackerFilter,
  STAttackFork,
  STDegenerateTree,
  STAttackFindShortest,
  STCastlingAttackerFilter,
  STCounterMateAttackerFilter,
  STDoubleMateAttackerFilter,
  STEnPassantAttackerFilter,
  STAttackRoot,
  STAttackMove,
  STAttackMoveToGoal,
  STMaxSolutionsRootDefenderFilter,
  STRestartGuard,
  STAttackMovePlayed,
  STEndOfSolutionWriter,
  STThreatCollector,
  STKillerMoveCollector,
  STAttackMoveShoeHorningDone,
  STSelfCheckGuard,
  STAttackMoveLegalityChecked,
  STMaxNrNonTrivial,
  STMaxNrNonChecks,
  STKeepMatingGuardDefenderFilter,
  STAttackMoveFiltered,
  STContinuationSolver,
  STKeyWriter,
  STTrySolver,
  STTryWriter,
  STContinuationWriter,
  STDefenseRoot,
  STCheckDetector,
  STMaxFlightsquares,
  STAttackDealtWith,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STMaxThreatLength,
  STPostKeyPlaySuppressor
};

enum
{
  nr_slice_rank_order_elmts = (sizeof slice_rank_order
                               / sizeof slice_rank_order[0]),
  no_battle_branch_slice_type = INT_MAX
};

/* Determine the rank of a defense slice type, relative to some base rank
 * @param type defense slice type
 * @param base base rank value
 * @return rank of type (>=base)
 *         base+nr_slice_rank_order_elmts if the rank can't be determined
 */
static unsigned int get_slice_rank(SliceType type, unsigned int base)
{
  unsigned int result = no_battle_branch_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_rank_order_elmts; ++i)
    if (slice_rank_order[(i+base)%nr_slice_rank_order_elmts]==type)
    {
      result = i+base;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void battle_branch_insert_slices_recursive(slice_index si_start,
                                                  slice_index const prototypes[],
                                                  unsigned int nr_prototypes,
                                                  unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si_start);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    slice_index si = si_start;
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_slice_rank(prototype_type,base);

    do
    {
      slice_index const next = slices[si].u.pipe.next;
      if (slices[next].type==STGoalReachedTesting)
      {
        leaf_branch_insert_slices_nested(next,prototypes,nr_prototypes);
        break;
      }
      else if (slices[next].type==STProxy)
        si = next;
      else if (slices[next].type==STQuodlibet
               || slices[next].type==STReciprocal)
      {
        battle_branch_insert_slices_recursive(slices[next].u.binary.op1,
                                              prototypes,nr_prototypes,
                                              base);
        battle_branch_insert_slices_recursive(slices[next].u.binary.op2,
                                              prototypes,nr_prototypes,
                                              base);
        break;
      }
      else if (slices[next].type==STAttackFork)
      {
        battle_branch_insert_slices_recursive(slices[next].u.branch_fork.towards_goal,
                                              prototypes,nr_prototypes,
                                              base);
        si = next;
      }
      else
      {
        unsigned int const rank_next = get_slice_rank(slices[next].type,base);
        if (rank_next==no_battle_branch_slice_type)
          break;
        else if (rank_next>prototype_rank)
        {
          pipe_append(si,copy_slice(prototypes[0]));
          if (nr_prototypes>1)
            battle_branch_insert_slices_recursive(si,
                                                  prototypes+1,nr_prototypes-1,
                                                  prototype_rank);
          break;
        }
        else
        {
          base = rank_next;
          si = next;
        }
      }
    } while (si!=si_start && prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battle_branch_insert_slices_base(slice_index si,
                                             slice_index const prototypes[],
                                             unsigned int nr_prototypes)
{
  switch (slices[si].type)
  {
    case STProxy:
    case STNot:
      battle_branch_insert_slices_base(slices[si].u.pipe.next,
                                       prototypes,nr_prototypes);
      break;

    case STQuodlibet:
    case STReciprocal:
      battle_branch_insert_slices_base(slices[si].u.binary.op1,
                                       prototypes,nr_prototypes);
      battle_branch_insert_slices_base(slices[si].u.binary.op2,
                                       prototypes,nr_prototypes);
      break;

    case STGoalReachedTesting:
      leaf_branch_insert_slices_nested(si,prototypes,nr_prototypes);
      break;

    default:
    {
      unsigned int const base = get_slice_rank(slices[si].type,0);
      assert(base!=no_battle_branch_slice_type);
      battle_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);
      break;
    }
  }
}

/* Insert slices into a battle branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  battle_branch_insert_slices_base(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a branch consisting mainly of an attack move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_attack_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;
  slice_index shortest;
  slice_index attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_ready_for_attack_slice(length,min_length);
  shortest = alloc_attack_find_shortest_slice(length,min_length);
  attack = alloc_attack_move_slice(length,min_length);
  pipe_link(result,shortest);
  pipe_link(shortest,attack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch consisting mainly of an defense move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies slice where the defense branch leads to
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  {
    slice_index const dready = alloc_ready_for_defense_slice(length,
                                                             min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const dplayed = alloc_defense_move_played_slice(length-1,
                                                                min_length-1);
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length-1,
                                                           min_length-1);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length-1,min_length-1);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length-1,min_length-1);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,next);

    result = dready;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert(length>slack_length_battle);
  assert(min_length>slack_length_battle);

  {
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length,min_length);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length,
                                                           min_length);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length,min_length);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length,min_length);
    slice_index const aready = alloc_ready_for_attack_slice(length,min_length);
    slice_index const shortest = alloc_attack_find_shortest_slice(length,
                                                                  min_length);

    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const aplayed = alloc_branch(STAttackMovePlayed,
                                             length-1,min_length-1);
    slice_index const ashoehorned = alloc_branch(STAttackMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const checked = alloc_branch(STAttackMoveLegalityChecked,
                                             length-1,min_length-1);
    slice_index const afiltered = alloc_branch(STAttackMoveFiltered,
                                               length-1,min_length-1);
    slice_index const solver = alloc_branch(STContinuationSolver,
                                            length-1,min_length-1);
    slice_index const adealt = alloc_branch(STAttackDealtWith,
                                            length-1,min_length-1);
    slice_index const dready = alloc_ready_for_defense_slice(length-1,
                                                             min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const dplayed = alloc_defense_move_played_slice(length-2,
                                                                min_length-2);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,aready);
    pipe_link(aready,shortest);
    pipe_link(shortest,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,ashoehorned);
    pipe_link(ashoehorned,checked);
    pipe_link(checked,afiltered);
    pipe_link(afiltered,solver);
    pipe_link(solver,adealt);
    pipe_link(adealt,dready);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);

    result = dchecked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a battle slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void battle_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
