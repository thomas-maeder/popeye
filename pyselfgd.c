#include "pyselfgd.h"
#include "pybrafrk.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STSelfAttack slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_self_attack(stip_length_type length,
                                     stip_length_type min_length,
                                     slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSelfAttack,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfDefense slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_self_defense(stip_length_type length,
                                      stip_length_type min_length,
                                      slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSelfDefense,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean self_defense_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
    slice_index const self_defense = alloc_self_defense(length,min_length,
                                                        to_goal);
    pipe_link(self_defense,*root);
    *root = self_defense;

    slices[si].u.branch.length -= 2;
    if (min_length>=slack_length_battle+2)
      slices[si].u.branch.min_length -= 2;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean self_attack_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);

  {
    slice_index * const root = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
    slice_index const self_attack = alloc_self_attack(length,min_length,
                                                      to_goal);

    pipe_link(self_attack,*root);
    *root = self_attack;

    slices[si].u.branch.length -= 2;
    if (min_length>=slack_length_battle+2)
      slices[si].u.branch.min_length -= 2;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type self_defense_direct_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n_min>=slack_length_battle-1);

  if (n_min==slack_length_battle-1
      && slice_has_non_starter_solved(towards_goal))
    result = n_min;
  else if (n>slack_length_battle)
    result = attack_has_solution_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean self_defense_are_threats_refuted_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n)
{
  boolean result = true;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const max_n_for_goal = length-min_length+slack_length_battle;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",max_n_for_goal);

  if (n<max_n_for_goal && slice_has_non_starter_solved(towards_goal))
    result = false;
  else
    result = attack_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type self_defense_direct_solve_threats_in_n(table threats,
                                                        slice_index si,
                                                        stip_length_type n,
                                                        stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack_solve_threats_in_n(threats,next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean self_attack_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const n_max_for_goal
      = length-min_length+slack_length_battle;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<=n_max_for_goal && !slice_defend(to_goal))
    result = false;
  else if (next!=no_slice)
    result = defense_defend_in_n(next,n);
  else
    result = true;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int self_attack_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int max_result)
{
  unsigned int result = max_result+1;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_battle)
    result = slice_count_refutations(to_goal,max_result);
  else
    result = defense_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean self_attack_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();
  
  /* We arrive here e.g. when solving the set play of a sXN
   */
  if (slices[si].u.branch.min_length==slack_length_battle
      && slice_root_solve(slices[si].u.branch_fork.towards_goal))
    result = true;
  else
    result = slice_root_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Slice ***************
 */

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean self_attack_root_make_setplay_slice(slice_index si,
                                            struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;
  slice_index const length = slices[si].u.branch.length;
  slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_battle)
  {
    assert(slices[proxy_to_goal].type==STProxy);
    prod->setplay_slice = slices[proxy_to_goal].u.pipe.next;
  }
  else
  {
    slice_index const copy = copy_slice(si);
    traverse_slices(slices[si].u.pipe.next,st);
    pipe_link(copy,prod->setplay_slice);
    prod->setplay_slice = copy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean self_attack_root_reduce_to_postkey_play(slice_index si,
                                                struct slice_traversal *st)
{
  boolean const result = true;
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_battle)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type self_defense_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (min_length==slack_length_battle-1
      && slice_has_non_starter_solved(towards_goal))
    result = (n-slack_length_battle)%2;
  else
    result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice - adapter for direct slices
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean self_defense_solve(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length==slack_length_battle-1
      && slice_has_non_starter_solved(towards_goal))
    result = false;
  else if (length>slack_length_battle
           && attack_has_solution_in_n(next,length,min_length)<=length)
    result = attack_solve(next);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean self_attack_root_defend(slice_index si)
{
  stip_length_type const length = slices[si].u.branch.length;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_battle)
    result = slice_root_defend(to_goal,max_nr_refutations);
  else
  {
    stip_length_type const min_length = slices[si].u.branch.min_length;
    if (min_length==slack_length_battle
        && !slice_root_defend(to_goal,0))
      result = false;
    else
      result = defense_root_defend(slices[si].u.pipe.next);
  }
        
  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean self_attack_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean self_defense_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  traverse_slices(slices[si].u.pipe.next,st);

  *starter = advers(*starter);
  traverse_slices(slices[si].u.branch_fork.towards_goal,st);
  *starter = advers(*starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfDefense slice after a defender slice
 * @param si identifies defender slice
 * @param proxy_to_goal identifies proxy slice leading towards goal
 */
static void insert_self_defense_after_defender(slice_index si,
                                               slice_index proxy_to_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const self_defense = alloc_self_defense(length-1,min_length-1,
                                                        proxy_to_goal);
    pipe_link(self_defense,slices[si].u.pipe.next);
    pipe_link(si,self_defense);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STSelfDefense after each STDefenseRoot slice if a
 * defender's move played in the STDefenseRoot slice is allowed
 * to solve the following branch (as typical in a non-exact stipulation).
 */
static
boolean self_guards_inserter_defense_root(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    if (slices[next_prev].type==STSelfDefense)
      /* we are attached to a loop; attach to the STSelfDefense in the
       * loop
       */
    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      pipe_set_successor(si,next_prev);
      slices[next_prev].u.branch.length = length-1;
      slices[next_prev].u.branch.min_length = min_length-1;
    }
    else
    {
      /* we are not attached to a loop; create our own STSelfDefense
       */
      slice_index const * const proxy_to_goal = st->param;
      insert_self_defense_after_defender(si,*proxy_to_goal);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfDefense after each STDefenseRoot slice if a
 * defender's move played in the STDefenseRoot slice is allowed
 * to solve the following branch (as typical in a non-exact stipulation).
 */
static boolean self_guards_inserter_defense_move(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[slices[si].u.pipe.next].type!=STSelfDefense);

  {
    slice_index const * const proxy_to_goal = st->param;
    insert_self_defense_after_defender(si,*proxy_to_goal);
  }

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfAttack after each STAttackMove and STAttackRoot slice
 */
static boolean self_guards_inserter_attack_move(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const proxy_to_goal = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index self_attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  self_attack = alloc_self_attack(length-1,min_length-1,*proxy_to_goal);
  pipe_link(self_attack,slices[si].u.pipe.next);
  pipe_link(si,self_attack);
  slice_traverse_children(self_attack,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const self_guards_inserters[] =
{
  &slice_traverse_children,           /* STProxy */
  &self_guards_inserter_attack_move,  /* STAttackMove */
  &self_guards_inserter_defense_move, /* STDefenseMove */
  &slice_traverse_children,           /* STHelpMove */
  &slice_traverse_children,           /* STHelpFork */
  &slice_traverse_children,           /* STSeriesMove */
  &slice_traverse_children,           /* STSeriesFork */
  &slice_operation_noop,              /* STLeafDirect */
  &slice_operation_noop,              /* STLeafHelp */
  &slice_traverse_children,           /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSeriesFilter */
  &self_guards_inserter_attack_move,  /* STAttackRoot */
  &slice_traverse_children,           /* STBattlePlaySolutionWriter */
  &slice_traverse_children,           /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,           /* STContinuationWriter */
  &slice_traverse_children,           /* STTryWriter */
  &slice_traverse_children,           /* STThreatWriter */
  &self_guards_inserter_defense_root, /* STDefenseRoot */
  &slice_traverse_children,           /* STThreatEnforcer */
  &slice_traverse_children,           /* STRefutationsCollector */
  &slice_traverse_children,           /* STVariationWriter */
  &slice_traverse_children,           /* STRefutingVariationWriter */
  &slice_traverse_children,           /* STNoShortVariations */
  &slice_traverse_children,           /* STAttackHashed */
  &slice_traverse_children,           /* STHelpRoot */
  &slice_traverse_children,           /* STHelpShortcut */
  &slice_traverse_children,           /* STHelpHashed */
  &slice_traverse_children,           /* STSeriesRoot */
  &slice_traverse_children,           /* STSeriesShortcut */
  &slice_traverse_children,           /* STParryFork */
  &slice_traverse_children,           /* STSeriesHashed */
  &slice_traverse_children,           /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,           /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,           /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,           /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,           /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,           /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,           /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,           /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,           /* STDirectDefense */
  &slice_traverse_children,           /* STReflexHelpFilter */
  &slice_traverse_children,           /* STReflexSeriesFilter */
  &slice_traverse_children,           /* STReflexRootSolvableFilter */
  &slice_traverse_children,           /* STReflexAttackerFilter */
  &slice_traverse_children,           /* STReflexDefenderFilter */
  &slice_traverse_children,           /* STSelfAttack */
  &slice_traverse_children,           /* STSelfDefense */
  &slice_traverse_children,           /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,           /* STRestartGuardHelpFilter */
  &slice_traverse_children,           /* STRestartGuardSeriesFilter */
  &slice_traverse_children,           /* STIntelligentHelpFilter */
  &slice_traverse_children,           /* STIntelligentSeriesFilter */
  &slice_traverse_children,           /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,           /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,           /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,           /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,           /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,           /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,           /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,           /* STMaxFlightsquares */
  &slice_traverse_children,           /* STDegenerateTree */
  &slice_traverse_children,           /* STMaxNrNonTrivial */
  &slice_traverse_children,           /* STMaxThreatLength */
  &slice_traverse_children,           /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,           /* STMaxTimeDefenderFilter */
  &slice_traverse_children,           /* STMaxTimeHelpFilter */
  &slice_traverse_children,           /* STMaxTimeSeriesFilter */
  &slice_traverse_children,           /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,           /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,           /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,           /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,           /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,           /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children            /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a branch with STSelfAttack and STSelfDefense slices
 * @param si root of branch to be instrumented
 * @param to_goal identifies slice leading towards goal
 * @return identifier of branch entry slice after insertion
 */
slice_index slice_insert_self_guards(slice_index si, slice_index to_goal)
{
  slice_traversal st;
  slice_index const result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  slice_traversal_init(&st,&self_guards_inserters,&to_goal);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
