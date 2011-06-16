#include "optimisations/goals/castling/filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCastlingFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_filter_can_attack(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (starter==White
      ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
          : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle)
    SETFLAG(goal_preprequisites_met[nbply],goal_castling);
  result = can_attack(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_castling);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_filter_attack(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (starter==White
      ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
          : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle)
    SETFLAG(goal_preprequisites_met[nbply],goal_castling);
  result = attack(next,n,n_max_unsolvable);
  CLRFLAG(goal_preprequisites_met[nbply],goal_castling);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type castling_filter_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (starter==White
      ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
          : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle)
    SETFLAG(goal_preprequisites_met[nbply],goal_castling);
  result = help(next,slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_castling);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type castling_filter_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (starter==White
      ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
          : TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle)
    SETFLAG(goal_preprequisites_met[nbply],goal_castling);
  result = can_help(next,slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_castling);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
