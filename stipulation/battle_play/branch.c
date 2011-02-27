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
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/dead_end.h"
#include "stipulation/battle_play/min_length_attack_filter.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types appear in battle branches
 * some types are not mentioned because they have variable ranks.
 */
static slice_index const slice_rank_order[] =
{
  STAttackAdapter,
  STReadyForAttack,
  STBattleDeadEnd,
  STMinLengthAttackFilter,
  STStipulationReflexAttackSolver,
  STReflexAttackerFilter,
  STRootAttackFork,
  STAttackFork,
  STSaveUselessLastMove,
  STDegenerateTree,
  STAttackFindShortest,
  STCastlingAttackerFilter,
  STCounterMateFilter,
  STDoubleMateFilter,
  STEnPassantAttackerFilter,
  STAttackRoot,
  STAttackMove,
  STAttackMoveToGoal,
  STMaxTimeDefenderFilter,
  STMaxSolutionsRootDefenderFilter,
  STRestartGuard,
  STEndOfSolutionWriter,
  STThreatCollector,
  STKillerMoveCollector,
  STAttackMoveShoeHorningDone,
  STSelfCheckGuard,
  STAttackMoveLegalityChecked,
  STKeepMatingFilter,
  STMaxNrNonTrivial,
  STMaxNrNonChecks,

  STDefenseAdapter,
  STReadyForDefense,
  STContinuationSolver,
  STKeyWriter,
  STTrySolver,
  STTryWriter,
  STContinuationWriter,
  STCheckDetector,
  STMaxFlightsquares,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STMaxThreatLength,
  STPostKeyPlaySuppressor,
  STReflexDefenderFilter,
  STBattleDeadEnd,
  STThreatSolver,
  STDefenseFork,
  STDefenseMove,
  STDefenseMovePlayed,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STRefutationWriter,
  STDefenseMoveShoeHorningDone,
  STKillerMoveCollector,
  STSelfDefense,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STSelfCheckGuard,
  STDefenseMoveLegalityChecked,
  STNoShortVariations,
  STAttackHashed,
  STThreatEnforcer,
  STKeepMatingFilter,
  STVariationWriter,
  STRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeDecorationWriter
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
      if (next==no_slice)
        break;
      else if (slices[next].type==STGoalReachedTesting)
      {
        leaf_branch_insert_slices_nested(si,prototypes,nr_prototypes);
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
      else if (slices[next].type==STRootAttackFork
               || slices[next].type==STAttackFork)
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
          slice_index const copy = copy_slice(prototypes[0]);
          pipe_append(si,copy);
          if (nr_prototypes>1)
            battle_branch_insert_slices_recursive(copy,
                                                  prototypes+1,nr_prototypes-1,
                                                  prototype_rank+1);
          break;
        }
        else if (slices[next].type==STSelfDefense)
        {
          battle_branch_insert_slices_recursive(slices[next].u.branch_fork.towards_goal,
                                                prototypes,nr_prototypes,
                                                base);
          si = next;
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

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index si,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes)
{
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_slice_rank(slices[si].type,0);
  assert(base!=no_battle_branch_slice_type);

  battle_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  battle_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remove slices from a battle branch
 * @param si identifies starting point of deletion
 * @param types contains the types of slices to be removed in the order that
 *              they occur in types
 * @param nr_types number of elements of array types
 */
void battle_branch_remove_slices(slice_index si,
                                 SliceType const types[],
                                 unsigned int nr_types)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_types);
  TraceFunctionParamListEnd();

  do
  {
    slice_index next = slices[si].u.pipe.next;
    if (slices[next].type==types[0])
    {
      pipe_remove(next);
      if (nr_types>1)
      {
        ++types,
        --nr_types;
      }
      else
        break;
    }
    else if (slices[next].type==STProxy)
      si = next;
    else if (slices[next].type==STQuodlibet)
    {
      battle_branch_remove_slices(slices[next].u.binary.op1,types,nr_types);
      battle_branch_remove_slices(slices[next].u.binary.op2,types,nr_types);
      break;
    }
  } while (1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a branch consisting mainly of an defense move
 * @param next identifies the slice that the defense branch lead sto
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(slice_index next,
                                 stip_length_type length,
                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    slice_index const solver = alloc_continuation_solver_slice(length,
                                                               min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const dplayed = alloc_defense_move_played_slice(length-1,
                                                                min_length-1);
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length-1,
                                                 min_length-1);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice();

    pipe_link(adapter,solver);
    pipe_link(solver,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,next);

    result = adapter;
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
  assert(min_length>=slack_length_battle);

  {
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length,min_length);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice();
    slice_index const aready = alloc_ready_for_attack_slice(length,min_length);
    slice_index const shortest = alloc_attack_find_shortest_slice(length,
                                                                  min_length);

    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const ashoehorned = alloc_pipe(STAttackMoveShoeHorningDone);
    slice_index const checked = alloc_pipe(STAttackMoveLegalityChecked);
    slice_index const dready = alloc_ready_for_defense_slice(length-1,
                                                             min_length-1);
    slice_index const solver = alloc_continuation_solver_slice(length-1,
                                                               min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const dplayed = alloc_defense_move_played_slice(length-2,
                                                                min_length-2);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,aready);
    pipe_link(aready,shortest);
    pipe_link(shortest,attack);
    pipe_link(attack,ashoehorned);
    pipe_link(ashoehorned,checked);
    pipe_link(checked,dready);
    pipe_link(dready,solver);
    pipe_link(solver,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);

    if (min_length>slack_length_battle+1)
      pipe_append(aready,
                  alloc_min_length_attack_filter_slice(length,min_length));

    if ((length-slack_length_battle)%2==1)
      pipe_append(solver,alloc_battle_play_dead_end_slice());
    else
      pipe_append(aready,alloc_battle_play_dead_end_slice());

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
