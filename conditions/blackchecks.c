#include "conditions/blackchecks.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STNullMovePlayer slice.
 * @param after_move identifies landing slice after move playing
 * @return index of allocated slice
 */
static slice_index alloc_null_move_player_slice(slice_index after_move)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",after_move);
  TraceFunctionParamListEnd();

  result = alloc_fork_slice(STNullMovePlayer,after_move);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_null_move_handler(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const landing = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_null_move_player_slice(proxy);

  assert(*landing!=no_slice);
  link_to_branch(proxy,*landing);

  branch_insert_slices_contextual(si,st->context,&prototype,1);
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterMovingPieceMovement);
  branch_insert_slices_contextual(si,st->context,&prototype,1);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;
  slice_index const save_landing = *landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *landing = no_slice;
  insert_landing(si,st);

  stip_traverse_structure_children(si,st);

  insert_null_move_handler(si,st);
  *landing = save_landing;

  {
    slice_index const prototype = alloc_pipe(STBlackChecks);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(*landing==no_slice);
  *landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_blackchecks(slice_index si)
{
  stip_structure_traversal st;
  slice_index landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&landing);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterMovingPieceMovement,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type blackchecks_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[current_move[nbply]].arrival==nullsquare)
    result = attack(next,n);
  else if (trait[nbply]==Black && !echecc(White))
    result = n+2;
  else
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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type blackchecks_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[current_move[nbply]].arrival==nullsquare)
    result = defend(next,n);
  else if (trait[nbply]==Black && !echecc(White))
    result = slack_length-1;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_null_move(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_generation_stack[current_move[nbply]].arrival==nullsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type null_move_player_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_null_move())
  {
    result = attack(slices[si].next2,n);
    --current_move[nbply];
  }
  else
    result = attack(slices[si].next1,n);

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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type null_move_player_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_null_move())
  {
    result = defend(slices[si].next2,n);
    --current_move[nbply];
  }
  else
    result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
