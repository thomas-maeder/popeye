#include "solving/find_shortest.h"
#include "options/options.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/avoid_unsolvable.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STFindShortest slice.
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(stip_length_type length,
                                      stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STFindShortest,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void find_shortest_solve(slice_index si)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;
  stip_length_type const n_min = (min_length>=(length-solve_nr_remaining)+slack_length
                                  ? min_length-(length-solve_nr_remaining)
                                  : min_length);
  stip_length_type const save_solve_nr_remaining = solve_nr_remaining;

  solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(length>=solve_nr_remaining);

  for (solve_nr_remaining = n_min+(save_solve_nr_remaining-n_min)%2;
       solve_nr_remaining<=save_solve_nr_remaining;
       solve_nr_remaining += 2)
  {
    pipe_solve_delegate(si);
    if (solve_result<=MOVE_HAS_SOLVED_LENGTH())
      break;
  }

  solve_nr_remaining = save_solve_nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_find_shortest_battle_adapter(slice_index si,
                                                stip_structure_traversal *st)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length>=min_length+2)
  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si,st->context);
    assert(defense!=no_slice);
    slice_index const attack = branch_find_slice(STReadyForAttack,
                                                 defense,
                                                 stip_traversal_context_defense);
    assert(attack!=no_slice);
    slice_index const prototypes[] =
    {
        alloc_find_shortest_slice(length,min_length),
        alloc_learn_unsolvable_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    attack_branch_insert_slices(attack,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_ready_for_move_in_loop(slice_index ready_root)
{
  slice_index result = ready_root;
  do
  {
    result = branch_find_slice(STReadyForHelpMove,
                               result,
                               stip_traversal_context_help);
    assert(result!=no_slice);
  } while ((SLICE_U(result).branch.length-slack_length)%2
           !=(SLICE_U(ready_root).branch.length-slack_length)%2);
  return result;
}

static void insert_find_shortest_help_adapter(slice_index si,
                                              stip_structure_traversal *st)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length+2>=min_length)
  {
    if (st->level==structure_traversal_level_nested)
    {
      if (st->context==stip_traversal_context_intro)
      {
        switch (st->activity)
        {
          case stip_traversal_activity_solving:
          {
            slice_index const prototype = alloc_find_by_increasing_length_slice(length,min_length);
            slice_insertion_insert(si,&prototype,1);
            break;
          }

          case stip_traversal_activity_testing:
          {
            slice_index const prototype = alloc_find_shortest_slice(length,min_length);
            slice_insertion_insert(si,&prototype,1);
            break;
          }

          default:
            assert(0);
            break;
        }
      }
    }
    else /* root or set play */
    {
      if (!OptFlag[startmovenumber] && length>=slack_length+2)
      {
        {
          slice_index const prototype =
              alloc_find_by_increasing_length_slice(length,min_length);
          slice_insertion_insert(si,&prototype,1);
        }
        {
          slice_index const ready_root = branch_find_slice(STReadyForHelpMove,
                                                           si,
                                                           st->context);
          assert(ready_root!=no_slice);
          slice_index const ready_loop = find_ready_for_move_in_loop(ready_root);
          slice_index const proxy_root = alloc_proxy_slice();
          slice_index const proxy_loop = alloc_proxy_slice();
          pipe_set_successor(proxy_loop,ready_loop);
          pipe_link(SLICE_PREV(ready_root),
                    alloc_fork_on_remaining_slice(proxy_root,proxy_loop,
                                                  length-1-slack_length));
          pipe_link(proxy_root,ready_root);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const find_shortest_inserters[] =
{
  { STAttackAdapter,  &insert_find_shortest_battle_adapter },
  { STDefenseAdapter, &insert_find_shortest_battle_adapter },
  { STHelpAdapter,    &insert_find_shortest_help_adapter   }
};

enum
{
  nr_find_shortest_inserters = sizeof find_shortest_inserters / sizeof find_shortest_inserters[0]
};

/* Instrument the solving machinery with slices that attempt the shortest
 * solutions/variations
 * @param si root slice of the solving machinery
 */
void solving_insert_find_shortest_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override(&st,find_shortest_inserters,nr_find_shortest_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
