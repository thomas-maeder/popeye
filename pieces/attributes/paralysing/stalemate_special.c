#include "pieces/attributes/paralysing/stalemate_special.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingStalemateSpecial
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_paralysing_stalemate_special_slice(Side side)
{
  slice_index result;
  slice_index tested;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingStalemateSpecial);
  slices[result].u.goal_filter.goaled = side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the move generator produces some halfway (i.e. modulo self
 * check) legal moves
 * @param side side for which to find moves
 * @return true iff side has >=1 move
 */
static boolean has_move(Side side)
{
  boolean result;
  move_generation_mode = move_generation_not_optimized;
  genmove(side);
  result = encore();
  finply();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_stalemate_special_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = opponent_self_check;
  else if (has_move(goaled))
    result = has_no_solution;
  else
    result = slice_has_solution(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_stalemate_special_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = opponent_self_check;
  else if (has_move(goaled))
    result = has_no_solution;
  else
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
