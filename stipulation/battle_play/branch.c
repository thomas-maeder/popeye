#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "pymovein.h"
#include "stipulation/constraint.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/attack_move_generator.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/defense_move_generator.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types appear in battle branches
 * some types are not mentioned because they have variable ranks.
 */
static slice_index const slice_rank_order[] =
{
  STAttackAdapter,
  STTrivialEndFilter,
  STEndOfIntro,
  STReadyForAttack,
  STAttackHashed,
  STThreatStart,
  STConstraint,
  STMinLengthOptimiser,
  STCastlingFilter,
  STCounterMateFilter,
  STDoubleMateFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STDegenerateTree,
  STFindShortest,
  STShortSolutionsStart,
  STForkOnRemaining,
  STAttackMoveGenerator,
  STKillerMoveMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STMove,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STRestartGuard,
  STEndOfSolutionWriter,
  STThreatCollector,
  STKillerMoveCollector,
  STGoalReachedTester,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STDeadEndGoal,
  STSelfCheckGuard,
  STKeepMatingFilter,
  STEndOfBranch,
  STMaxNrNonTrivial,
  STMaxNrNonChecks,

  STDefenseAdapter,
  STRefutationsAllocator,
  STContinuationSolver,
  STMoveWriter,
  STKeyWriter,
  STTrySolver,
  STRefutationsSolver,
  STTryWriter,
  STCheckDetector,
  STMaxFlightsquares,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STPostKeyPlaySuppressor,
  STMinLengthGuard,
  STReadyForDefense,
  STConstraint,
  STEndOfBranchForced,
  STMaxThreatLength,
  STThreatSolver,
  STCounterMateFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STCheckZigzagJump,
  STForkOnRemaining,
  STForkOnRemaining,
  STDefenseMoveGenerator,
  STKillerMoveMoveGenerator,
  STCountNrOpponentMovesMoveGenerator,
  STKillerMoveFinalDefenseMove,
  STMove,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STRefutationWriter,
  STEndOfRoot,
  STKillerMoveCollector,
  STMinLengthGuard,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STSelfCheckGuard,
  STCheckZigzagLanding,
  STMaxThreatLengthHook, /* separate from STThreatStart to enable hashing*/
  STNoShortVariations,
  STKeepMatingFilter,
  STThreatEnforcer,
  STMoveWriter,
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
      else if (slices[next].type==STProxy)
        si = next;
      else if (slices[next].type==STOr
               || slices[next].type==STAnd)
      {
        battle_branch_insert_slices_recursive(slices[next].u.binary.op1,
                                              prototypes,nr_prototypes,
                                              base);
        battle_branch_insert_slices_recursive(slices[next].u.binary.op2,
                                              prototypes,nr_prototypes,
                                              base);
        break;
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
        else if (slices[next].type==STGoalReachedTester)
        {
          branch_insert_slices_nested(next,prototypes,nr_prototypes);
          break;
        }
        else if (slices[next].type==STEndOfBranch
                 || slices[next].type==STEndOfBranchGoal
                 || slices[next].type==STEndOfBranchGoalImmobile
                 || slices[next].type==STEndOfBranchForced)
        {
          branch_insert_slices_nested(slices[next].u.fork.fork,
                                      prototypes,nr_prototypes);
          si = next;
        }
        else if (slices[next].type==STForkOnRemaining
                 || slices[next].type==STCheckZigzagJump)
        {
          battle_branch_insert_slices_recursive(slices[next].u.fork.fork,
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

  if (slices[si].type==STProxy)
    battle_branch_insert_slices_nested(slices[si].u.pipe.next,
                                       prototypes,nr_prototypes);
  else
  {
    base = get_slice_rank(slices[si].type,0);
    assert(base!=no_battle_branch_slice_type);

    battle_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);
  }

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
    else if (slices[next].type==STOr)
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
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    slice_index const ready = alloc_branch(STReadyForDefense,length,min_length);
    slice_index const deadend = alloc_dead_end_slice();
    slice_index const generator = alloc_defense_move_generator_slice();
    slice_index const defense = alloc_move_slice();

    pipe_link(adapter,ready);
    pipe_link(ready,deadend);
    pipe_link(deadend,generator);
    pipe_link(generator,defense);
    pipe_link(defense,next);

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
    slice_index const adapter = alloc_attack_adapter_slice(length,min_length);
    slice_index const aready = alloc_branch(STReadyForAttack,length,min_length);
    slice_index const adeadend = alloc_dead_end_slice();
    slice_index const agenerator = alloc_attack_move_generator_slice();
    slice_index const attack = alloc_move_slice();
    slice_index const dready = alloc_branch(STReadyForDefense,
                                            length-1,min_length-1);
    slice_index const ddeadend = alloc_dead_end_slice();
    slice_index const dgenerator = alloc_defense_move_generator_slice();
    slice_index const defense = alloc_move_slice();

    pipe_link(adapter,aready);
    pipe_link(aready,adeadend);
    pipe_link(adeadend,agenerator);
    pipe_link(agenerator,attack);
    pipe_link(attack,dready);
    pipe_link(dready,ddeadend);
    pipe_link(ddeadend,dgenerator);
    pipe_link(dgenerator,defense);
    pipe_link(defense,adapter);

    result = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a branch leading to a goal to be an attack branch
 * @param si identifies entry slice of branch
 */
void stip_make_goal_attack_branch(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  {
    slice_index const adapter = alloc_attack_adapter_slice(slack_length_battle+1,
                                                           slack_length_battle);
    slice_index const prototypes[] =
    {
      alloc_branch(STReadyForAttack,slack_length_battle+1,slack_length_battle),
      alloc_dead_end_slice(),
      alloc_attack_move_generator_slice(),
      alloc_move_slice(),
      alloc_defense_adapter_slice(slack_length_battle,slack_length_battle-1)
    };
    enum {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    pipe_append(si,adapter);
    battle_branch_insert_slices(adapter,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the copy of a slice into the set play branch
 * @param si slice index
 * @param st state of traversal
 */
static void copy_to_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const copy = copy_slice(si);
    link_to_branch(copy,*result);
    *result = copy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void serve_as_root_hook(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *root_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param adapter identifies the adapter slice into the battle branch
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index battle_branch_make_setplay(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const start = branch_find_slice(STReadyForDefense,adapter);
    stip_structure_traversal st;
    slice_structural_type type;

    slice_index const prototype = alloc_pipe(STEndOfRoot);
    battle_branch_insert_slices(adapter,&prototype,1);

    assert(start!=no_slice);

    stip_structure_traversal_init(&st,&result);
    for (type = 0; type!=nr_slice_structure_types; ++type)
      stip_structure_traversal_override_by_structure(&st,type,&copy_to_setplay);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(start,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Make the postkey play representation of a non-postkey play representation
 * @param adapter identifies adapter slice into battle branch
 * @return identifier to adapter slice into postkey representation
 */
slice_index battle_branch_make_postkeyplay(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  TraceStipulation(adapter);

  assert(slices[adapter].type==STAttackAdapter);

  {
    stip_length_type const length = slices[adapter].u.branch.length;
    stip_length_type const min_length = slices[adapter].u.branch.min_length;
    slice_index const proto = alloc_defense_adapter_slice(length-1,
                                                          min_length-1);
    battle_branch_insert_slices(adapter,&proto,1);

    result = branch_find_slice(STDefenseAdapter,adapter);
    assert(result!=no_slice);

    {
      slice_index si;
      for (si = adapter; si!=result; si = slices[si].u.pipe.next)
        if (slice_has_structure(si,slice_structure_branch))
        {
          slices[si].u.branch.length -= 2;
          slices[si].u.branch.min_length -= 2;
        }
    }

    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static void attack_adapter_make_postkeyplay(slice_index adapter,
                                            stip_structure_traversal *st)
{
  slice_index * const postkey = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *postkey = battle_branch_make_postkeyplay(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Attempt to apply the postkey play option to the current stipulation
 * @param root_proxy identifies root proxy slice
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean battle_branch_apply_postkeyplay(slice_index root_proxy)
{
  boolean result;
  slice_index postkey_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(root_proxy);

  stip_structure_traversal_init(&st,&postkey_slice);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &pipe_make_root);
  stip_structure_traversal_override_single(&st,
                                           STAttackAdapter,
                                           &attack_adapter_make_postkeyplay);
  stip_traverse_structure(slices[root_proxy].u.pipe.next,&st);

  if (postkey_slice==no_slice)
    result = false;
  else
  {
    link_to_branch(root_proxy,postkey_slice);

    {
      slice_index const prototype = alloc_move_inverter_slice();
      root_branch_insert_slices(root_proxy,&prototype,1);
    }

    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of root slice
 */
slice_index battle_make_root(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    slice_structural_type i;

    slice_index const prototype = alloc_pipe(STEndOfRoot);
    battle_branch_insert_slices(adapter,&prototype,1);

    stip_structure_traversal_init(&st,&result);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,
                                             STEndOfRoot,
                                             &serve_as_root_hook);
    stip_traverse_structure(adapter,&st);

    {
      slice_index si;
      for (si = adapter; slices[si].type!=STEndOfRoot; si = slices[si].u.pipe.next)
        if (slice_has_structure(si,slice_structure_branch))
        {
          slices[si].u.branch.length -= 2;
          slices[si].u.branch.min_length -= 2;
        }
    }

    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Spin the intro slices off a nested battle branch
 * @param adapter identifies adapter slice of the nested help branch
 */
void battle_spin_off_intro(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STAttackAdapter);

  TraceStipulation(adapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfIntro);
    battle_branch_insert_slices(adapter,&prototype,1);
  }

  if (branch_find_slice(STEndOfIntro,adapter)!=no_slice)
  {
    slice_index const next = slices[adapter].u.pipe.next;
    slice_index nested = no_slice;
    stip_structure_traversal st;
    slice_structural_type i;

    stip_structure_traversal_init(&st,&nested);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,STEndOfIntro,&serve_as_root_hook);
    stip_traverse_structure(next,&st);

    pipe_link(slices[adapter].prev,next);
    link_to_branch(adapter,nested);
    slices[adapter].prev = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with STEndOfBranchForced slices (typically for a
 * (semi-)reflex stipulation)
 * @param si root of branch to be instrumented
 * @param forced identifies branch that needs to be guarded from
 */
void battle_branch_insert_end_of_branch_forced(slice_index si,
                                               slice_index forced)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",forced);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(forced);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    assert(ready!=no_slice);
    pipe_append(ready,alloc_end_of_branch_forced(forced));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_constraint(slice_index si,
                                            slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_constraint_slice(constraint);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_constraint(slice_index si,
                                            slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    slice_index const prototype = alloc_constraint_slice(constraint);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch_goal(slice_index si,
                                                    slice_index goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(goal);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_end_of_branch_goal(goal);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param next identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(next);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_end_of_branch_slice(next);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch for detecting whether the defense was forced to reach a
 * goal
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch_goal(slice_index si,
                                                  slice_index goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(goal);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    slice_index const prototype = alloc_end_of_branch_goal(goal);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
