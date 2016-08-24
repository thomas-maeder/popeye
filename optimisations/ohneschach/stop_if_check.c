#include "optimisations/ohneschach/stop_if_check.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "solving/check.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef enum
{
  fate_dont_know,
  fate_immobility_tester_obsolete,
  fate_immobility_tester_still_used,
  fate_stop_to_be_optimised,
  fate_stop_not_to_be_optimised
} fate_type;

static fate_type fate[max_nr_slices];

/* Plan optimising away a STOhneschachStopIfCheckAndNotMate slice
 * @param stop identifies STOhneschachStopIfCheckAndNotMate slice
 * @param to_be_optimised true iff stop is going to be optimised away
 */
void ohneschach_stop_if_check_plan_to_optimise_away_stop(slice_index stop,
                                                         boolean to_be_optimised)
{
  slice_index const immobility_tester = SLICE_NEXT2(stop);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stop);
  TraceFunctionParam("%u",to_be_optimised);
  TraceFunctionParamListEnd();

  TraceValue("%u",fate[immobility_tester]);
  TraceEOL();

  if (to_be_optimised)
  {
    fate[stop] = fate_stop_to_be_optimised;
    if (fate[immobility_tester]==fate_dont_know)
      fate[immobility_tester] = fate_immobility_tester_obsolete;
  }
  else
  {
    fate[stop] = fate_stop_not_to_be_optimised;
    fate[immobility_tester] = fate_immobility_tester_still_used;
  }

  TraceValue("->%u",fate[immobility_tester]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_stop(slice_index stop, stip_structure_traversal *st)
{
  slice_index const immobility_tester = SLICE_NEXT2(stop);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stop);
  TraceFunctionParamListEnd();

  TraceValue("%u",fate[stop]);
  TraceEOL();
  TraceValue("%u",immobility_tester);
  TraceValue("%u",fate[immobility_tester]);
  TraceEOL();

  switch (fate[stop])
  {
    case fate_stop_to_be_optimised:
      if (fate[immobility_tester]==fate_immobility_tester_obsolete)
        dealloc_slices(immobility_tester);

      if (SLICE_TESTER(stop)!=no_slice && fate[SLICE_TESTER(stop)]==fate_dont_know)
        /* substitute unreachable tester slice */
        pipe_substitute(SLICE_TESTER(stop),alloc_pipe(STOhneschachStopIfCheck));

      pipe_substitute(stop,alloc_pipe(STOhneschachStopIfCheck));
      break;

    case fate_stop_not_to_be_optimised:
      assert(fate[immobility_tester]!=fate_immobility_tester_obsolete);
      break;

    default:
      assert(0);
      break;
  }

  fate[stop] = fate_dont_know;
  fate[immobility_tester] = fate_dont_know;

  stip_traverse_structure_children_pipe(stop,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Execute the optimisations planned before and communicated using a series of
 * invokations of ohneschach_stop_if_check_plan_to_optimise_away_stop()
 * @param root root slice of the stiptulation
 */
void ohneschach_stop_if_check_execute_optimisations(slice_index root)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STOhneschachStopIfCheckAndNotMate,
                                           &optimise_stop);
  stip_traverse_structure(root,&st);

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
void ohneschach_stop_if_check_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_in_check(SLICE_STARTER(si)))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
