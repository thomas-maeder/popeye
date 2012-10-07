#include "pieces/attributes/paralysing/stalemate_special.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingStalemateSpecial
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @param starter_or_adversary is the starter stalemated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_stalemate_special_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingStalemateSpecial);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type paralysing_stalemate_special_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  goal_applies_to_starter_or_adversary const
    applies_to_who = slices[si].u.goal_filter.applies_to_who;
  Side const starter = slices[si].starter;
  Side const stalemated = (applies_to_who==goal_applies_to_starter
                           ? starter
                           : advers(starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (applies_to_who==goal_applies_to_starter && echecc(advers(starter)))
    result = slack_length-2;
  else if (suffocated_by_paralysis(stalemated))
    result = n;
  else
    result = solve(next,length_unspecified);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
