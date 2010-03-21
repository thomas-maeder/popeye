#include "stipulation/battle_play/defense_move.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/move.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseMove defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_move_slice(stip_length_type length,
                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_battle)
    min_length += 2;
  assert(min_length>=slack_length_battle);
  result = alloc_branch(STDefenseMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_insert_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index * const root = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    *root = alloc_defense_move_slice(length,min_length);
    pipe_set_successor(*root,slices[si].u.pipe.next);

    slices[si].u.branch.length -= 2;
    if (slices[si].u.branch.min_length>=slack_length_battle+2)
      slices[si].u.branch.min_length -= 2;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void defense_move_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();
  
  if (slices[si].starter==no_side)
    slices[si].starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean defense_move_root_defend(slice_index si)
{
  Side const defender = slices[si].starter;
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const n_next = length-1;
  stip_length_type const parity = (n_next-slack_length_battle)%2;
  stip_length_type n_min = slack_length_battle-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n_next+min_length>n_min+length)
    n_min = n_next-(length-min_length);

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && attack_solve_in_n(next,n_next,n_min)<=n_next)
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean defense_move_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_next = n-1;
  stip_length_type const parity = (n_next-slack_length_battle)%2;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_min = slack_length_battle-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n_next+min_length>n_min+length)
    n_min = n_next-(length-min_length);

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && attack_solve_in_n(next,n_next,n_min)>n_next)
      result = true;

    repcoup();
  }

  finply();

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
unsigned int defense_move_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          unsigned int max_result)
{
  Side const defender = slices[si].starter;
  unsigned int result = 0;
  slice_index const next = slices[si].u.pipe.next;
  boolean isDefenderImmobile = true;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n-1+min_length>slack_length_battle+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_battle-parity;

  active_slice[nbply+1] = si;
  move_generation_mode =
      n-1>slack_length_battle+2
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result<=max_result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const length_sol = attack_has_solution_in_n(next,
                                                                   n-1,
                                                                   n_min_next);
      if (length_sol>=n)
      {
        ++result;
        coupfort();
        isDefenderImmobile = false;
      }
      else if (length_sol>=n_min_next)
        isDefenderImmobile = false;
      else
      {
        /* self check */
      }
    }

    repcoup();
  }

  finply();

  if (isDefenderImmobile)
    result = max_result+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void defense_move_make_setplay_slice(slice_index si, stip_structure_traversal *st)
{
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    prod->setplay_slice = alloc_help_move_slice(length,min_length);
    pipe_set_successor(prod->setplay_slice,slices[si].u.pipe.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void defense_move_reduce_to_postkey_play(slice_index si,
                                         stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
