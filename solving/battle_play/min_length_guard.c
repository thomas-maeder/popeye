#include "solving/battle_play/min_length_guard.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solving.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STMinLengthGuard slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_min_length_guard(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMinLengthGuard,length,min_length);

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
stip_length_type min_length_guard_attack(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const length = slices[si].u.branch.length;
  slice_index const min_length = slices[si].u.branch.min_length;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(next,n);

  if (result>slack_length-2 && n+min_length>length+result)
    /* the defender has refuted by reaching the goal too early */
    result = n+2;

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
stip_length_type min_length_guard_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);

  if (slack_length<=result && n+min_length>length+result)
    /* the defender has refuted by reaching the goal too early */
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_min_length_solvers_attack(slice_index si,
                                             stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (min_length>slack_length+1)
  {
    slice_index const prototype = alloc_min_length_optimiser_slice(length,
                                                                   min_length);
    attack_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_min_length_solvers_defense(slice_index si,
                                              stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (min_length>slack_length+1)
  {
    slice_index const prototype = alloc_min_length_guard(length-1,min_length-1);
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation to be able to cope with minimum lengths
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_min_length_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_single(&st,
                                           STReadyForAttack,
                                           &insert_min_length_solvers_attack);
  stip_structure_traversal_override_single(&st,
                                           STReadyForDefense,
                                           &insert_min_length_solvers_defense);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
/* Callback to stip_spin_off_testers
 * Spin a tester slice off a STMinLengthGuard slice
 * @param si identifies the STMinLengthGuard slice
 * @param st address of structure representing traversal
 */
void spin_off_testers_min_length_guard(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_attack)
    spin_off_testers_move_pipe_to_testers(si,st);
  else
    stip_spin_off_testers_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
