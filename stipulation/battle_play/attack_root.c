#include "stipulation/battle_play/attack_root.h"
#include "pydata.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "pyleaf.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/root.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_root_slice(stip_length_type length,
                                    stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert((length%2)==(min_length%2));

  result = alloc_branch(STAttackRoot,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean attack_root_root_solve(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  unsigned int const max_nr_refutations = 0;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  output_start_continuation_level();

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  active_slice[nbply+1] = si;
  if (length<=slack_length_battle
      && slices[si].u.branch.imminent_goal!=no_goal)
  {
    empile_for_goal = slices[si].u.branch.imminent_goal;
    empile_for_target = slices[si].u.branch.imminent_target;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;
  }
  else
    genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const
          nr_moves_needed = defense_root_defend(next,
                                                length-1,min_length-1,
                                                max_nr_refutations);
      if (min_length-1<=nr_moves_needed)
      {
        if (nr_moves_needed<=length-1)
          result = true;
        if (nr_moves_needed<=length+1)
          write_end_of_solution();
      }
    }

    repcoup();
  }

  output_end_continuation_level();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void attack_root_make_setplay_slice(slice_index si,
                                    stip_structure_traversal *st)
{
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* prod->sibling may already have been set
   */
  if (prod->sibling==no_slice)
    prod->sibling = si;

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const length_h = (length-1
                                       +slack_length_help
                                       -slack_length_battle);
    slice_index const help_root = alloc_help_root_slice(length_h,length_h);
    pipe_link(help_root,prod->setplay_slice);
    prod->setplay_slice = help_root;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void attack_root_reduce_to_postkey_play(slice_index si,
                                        stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(next,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
