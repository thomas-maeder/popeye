#include "solving/battle_play/defense_adapter.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/battle_play/attack_adapter.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/machinery/solve.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Wrap the slices representing the nested slices
 * @param adapter identifies solve adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_intro(slice_index adapter,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  if (st->level==structure_traversal_level_nested
      && SLICE_U(adapter).branch.length>slack_length)
  {
    spin_off_state_type * const state = st->param;
    battle_spin_off_intro(adapter,state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void defense_adapter_solve(slice_index si)
{
  stip_length_type const length = SLICE_U(si).branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining==length_unspecified);

  solve_nr_remaining = length;
  pipe_solve_delegate(si);
  solve_nr_remaining = length_unspecified;

  if (solve_result==immobility_on_next_move)
    /* oops - unwanted stalemate or illegal move */;
  else
    pipe_this_move_solves_exactly_if(solve_result<=length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
