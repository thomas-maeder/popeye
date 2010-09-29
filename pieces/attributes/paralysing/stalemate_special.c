#include "pieces/attributes/paralysing/stalemate_special.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingStalemateFilter
 */

/* Allocate a STPiecesParalysingStalemateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_paralysing_stalemate_special_slice(Side side)
{
  slice_index result;
  slice_index tested;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingStalemateFilter);
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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
paralysing_stalemate_special_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = n+4;
  else if (has_move(goaled))
    result = n+4;
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
paralysing_stalemate_special_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = n+4;
  else if (has_move(goaled))
    result = n+4;
  else
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
paralysing_stalemate_special_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = n+2;
  else if (has_move(goaled))
    result = n+2;
  else
    result = attack_solve_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
paralysing_stalemate_special_has_solution_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const goaled = slices[si].u.goal_filter.goaled;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (starter==goaled && echecc(nbply,advers(starter)))
    result = n+2;
  else if (has_move(goaled))
    result = n+2;
  else
    result = attack_has_solution_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
