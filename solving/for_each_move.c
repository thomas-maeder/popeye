#include "solving/for_each_move.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move_player.h"
#include "solving/solving.h"
#include "solving/find_move.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STForEachMove slice.
 * @return index of allocated slice
 */
slice_index alloc_for_each_move_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STForEachMove);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_move_iterator_move(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = (st->activity==stip_traversal_activity_testing
                                   ? alloc_find_move_slice()
                                   : alloc_for_each_move_slice());
    slice_index const prototypes[] = {
        prototype,
        alloc_move_player_slice(),
        alloc_pipe(STLandingAfterMovePlay)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,prototypes,nr_prototypes);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,prototypes,nr_prototypes);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,prototypes,nr_prototypes);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const move_iterator_inserters[] =
{
  { STMove, &insert_move_iterator_move },
};

enum
{
  nr_move_iterator_inserters = (sizeof move_iterator_inserters
                                / sizeof move_iterator_inserters[0])
};

/* Instrument the stipulation with move iterator slices
 * @param root_slice identifies root slice of stipulation
 */
void stip_insert_move_iterators(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    move_iterator_inserters,
                                    nr_move_iterator_inserters);
  stip_traverse_structure(root_slice,&st);

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
stip_length_type for_each_move_attack(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (encore())
  {
    stip_length_type const length_sol = attack(slices[si].next1,n);
    if (slack_length<length_sol && length_sol<result)
      result = length_sol;
  }

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
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type for_each_move_defend(slice_index si, stip_length_type n)
{
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while(encore())
  {
    stip_length_type const length_sol = defend(slices[si].next1,n);
    if (result<length_sol)
      result = length_sol;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
