#include "stipulation/battle_play/defense_root.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/fork.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseRoot defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_root_slice(stip_length_type length,
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
  result = alloc_branch(STDefenseRoot,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean defense_root_root_defend(slice_index si)
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

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean defense_root_detect_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();
  
  if (slices[si].starter==no_side)
    slices[si].starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean defense_root_make_setplay_slice(slice_index si,
                                        struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (prod->sibling!=no_slice)
  {
    slice_index const branch = alloc_help_move_slice(slack_length_help+1,
                                                     slack_length_help+1);
    slice_index const fork = alloc_help_fork_slice(slack_length_help,
                                                   slack_length_help,
                                                   slices[si].u.pipe.next);
    pipe_link(branch,fork);
    prod->setplay_slice = alloc_help_root_slice(slack_length_help+1,
                                                slack_length_help+1,
                                                branch,
                                                no_slice);
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
boolean defense_root_reduce_to_postkey_play(slice_index si,
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
