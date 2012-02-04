#include "solving/battle_play/min_length_guard.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/branch.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STMinLengthGuard slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_min_length_guard(stip_length_type length,
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

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
min_length_guard_can_attack(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const length = slices[si].u.branch.length;
  slice_index const min_length = slices[si].u.branch.min_length;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(next,n);

  if (result>slack_length_battle-2 && n+min_length>length+result)
    /* the defender has refuted by reaching the goal too early */
    result = n+2;

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
stip_length_type min_length_guard_attack(slice_index si, stip_length_type n)
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

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type min_length_guard_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_defend(next,n);

  if (slack_length_battle<=result && n+min_length>length+result)
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
 * @return <slack_length_battle - no legal defense found
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

  if (slack_length_battle<=result && n+min_length>length+result)
    /* the defender has refuted by reaching the goal too early */
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_min_length_solvers_adapter(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si);
    if (defense!=no_slice)
    {
      stip_length_type const length = slices[defense].u.branch.length;
      stip_length_type const min_length = slices[defense].u.branch.min_length;

      if (min_length>slack_length_battle+1)
      {
        slice_index const prototype = alloc_min_length_guard(length-1,min_length-1);
        battle_branch_insert_slices(defense,&prototype,1);

        if (min_length>slack_length_battle+2)
        {
          slice_index const prototypes[] =
          {
            alloc_min_length_optimiser_slice(length-1,min_length-1),
            alloc_min_length_guard(length-2,min_length-2)
          };
          enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
          slice_index const attack = branch_find_slice(STReadyForAttack,defense);
          assert(attack!=no_slice);
          battle_branch_insert_slices(attack,prototypes,nr_prototypes);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const min_length_solver_inserters[] =
{
  { STAttackAdapter,  &insert_min_length_solvers_adapter },
  { STDefenseAdapter, &insert_min_length_solvers_adapter }
};

enum
{
  nr_min_length_solver_inserters = sizeof min_length_solver_inserters / sizeof min_length_solver_inserters[0]
};

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
  stip_structure_traversal_override(&st,
                                    min_length_solver_inserters,
                                    nr_min_length_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
