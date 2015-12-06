#include "conditions/nocapture.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

static boolean is_false(numecoup n)
{
  return false;
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
void nocapture_remove_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_false);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  boolean const (* const enabled)[nr_sides] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*enabled)[SLICE_STARTER(si)])
  {
    slice_index const prototype = alloc_pipe(STNocaptureRemoveCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_nocapture(slice_index si)
{
  stip_structure_traversal st;
  boolean enabled[nr_sides] =
  {
      CondFlag[nocapture] || CondFlag[nowhcapture],
      CondFlag[nocapture] || CondFlag[noblcapture]
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  solving_impose_starter(si,SLICE_STARTER(si));

  stip_structure_traversal_init(&st,&enabled);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_structure_traversal_override_single(&st,
                                           STExecutingKingCapture,
                                           &stip_structure_visitor_noop);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
