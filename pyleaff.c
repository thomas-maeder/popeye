#include "pyleaff.h"
#include "pyleafs.h"
#include "pyleaf.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */


/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_forced_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].u.leaf.starter;

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solution of a leaf slice at root level.
 * @param leaf identifies leaf slice
 */
void leaf_forced_root_solve(slice_index leaf)
{
  Side const defender = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  if (leaf_is_end_in_1_forced(defender,leaf))
  {
    output_start_postkey_level();
    leaf_s_solve_final_move(leaf,defender);
    output_end_postkey_level();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}
