#include "optimisations/killer_move/collector.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

static void remember_killer_move()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  kpilcd[nbply] = move_generation_stack[nbcou].departure;
  kpilca[nbply] = move_generation_stack[nbcou].arrival;

  TraceValue("%u",nbply);
  TraceSquare(kpilcd[nbply]);
  TraceSquare(kpilca[nbply]);
  TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STKillerMoveCollector slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_move_collector_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKillerMoveCollector);

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
stip_length_type
killer_move_collector_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(next,n);
  if (result>n)
    remember_killer_move();

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
stip_length_type
killer_move_collector_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(next,n);

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
stip_length_type killer_move_collector_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);
  if (slack_length_battle<=result && result<=n)
    remember_killer_move();

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
stip_length_type
killer_move_collector_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_defend(next,n);
  if (slack_length_battle<=result && result<=n)
    remember_killer_move();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
