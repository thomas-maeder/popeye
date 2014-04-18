#include "solving/boolean/and.h"
#include "solving/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/constraint.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void and_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve(slices[si].next1);

  if (previous_move_is_illegal<solve_result
      && solve_result<=MOVE_HAS_SOLVED_LENGTH())
    solve(slices[si].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_shortcut_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[slices[si].next2].tester!=no_slice);
  pipe_append(slices[si].next1,
              alloc_constraint_tester_slice(slices[slices[si].next2].tester));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Enable shortcut logic for all STAnd slices
 * @param si root slice of the stipulation
 */
void and_enable_shortcut_logic(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STAnd,&insert_shortcut_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
