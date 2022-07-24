#include "options/degenerate_tree.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/branch.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static stip_length_type max_length_short_solutions;

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
void init_degenerate_tree(stip_length_type max_length_short)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",max_length_short);
  TraceFunctionParamListEnd();

  max_length_short_solutions = 2*max_length_short;
  TraceValue("%u",max_length_short_solutions);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Initialisation ***************
 */

/* Allocate a STDegenerateTree slice
 * @return allocated slice
 */
static slice_index alloc_degenerate_tree_guard_slice(stip_length_type length,
                                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDegenerateTree,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate finding a solution to the next1 slice, gradually increasing
 * the number of allowed half-moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves to try
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type delegate_solve(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result = n+2;
  stip_length_type const save_solve_nr_remaining = MOVE_HAS_SOLVED_LENGTH();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  for (solve_nr_remaining = n_min+(n-n_min)%2; solve_nr_remaining<=n; solve_nr_remaining += 2)
  {
    pipe_solve_delegate(si);

  result = solve_result;
    if (result<=MOVE_HAS_SOLVED_LENGTH())
      break;
  }

  solve_nr_remaining = save_solve_nr_remaining;

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
void degenerate_tree_solve(slice_index si)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;
  stip_length_type const n_min = (min_length>=(length-solve_nr_remaining)+slack_length
                                  ? min_length-(length-solve_nr_remaining)
                                  : min_length);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_nr_remaining>max_length_short_solutions+slack_length)
  {
    if (max_length_short_solutions>=2)
    {
      stip_length_type const parity = (solve_nr_remaining-slack_length)%2;
      stip_length_type const n_interm = max_length_short_solutions+slack_length-parity;
      if (delegate_solve(si,n_interm,n_min)>n_interm)
        delegate_solve(si,solve_nr_remaining,solve_nr_remaining);
    }
    else
      delegate_solve(si,solve_nr_remaining,solve_nr_remaining);
  }
  else
    delegate_solve(si,solve_nr_remaining,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void degenerate_tree_inserter_attack(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->activity==stip_traversal_activity_testing
      && SLICE_U(si).branch.length>=slack_length+2)
  {
    slice_index const finder = branch_find_slice(STFindShortest,
                                                 si,
                                                 stip_traversal_context_attack);
    if (finder!=no_slice) /* slice may already have been replaced */
    {
      stip_length_type const length = SLICE_U(finder).branch.length;
      stip_length_type const min_length = SLICE_U(finder).branch.min_length;
      pipe_substitute(finder,alloc_degenerate_tree_guard_slice(length,min_length));
    }
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STDegenerateTree slices
 * @param si identifies slice where to start
 */
void solving_insert_degenerate_tree_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STReadyForAttack,
                                           &degenerate_tree_inserter_attack);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
