#include "optimisations/goals/enpassant/filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEnPassantFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_enpassant_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEnPassantFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type enpassant_filter_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = can_attack(next,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type enpassant_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = attack(next,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type enpassant_filter_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = defend(next,slack_length_battle+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type enpassant_filter_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = can_defend(next,slack_length_battle+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

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
stip_length_type enpassant_filter_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = help(next,slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

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
stip_length_type enpassant_filter_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (ep[nbply]!=initsquare || ep2[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = can_help(next,slack_length_help+1);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
