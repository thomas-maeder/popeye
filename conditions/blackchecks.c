#include "conditions/blackchecks.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "options/nullmoves.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Determine the length of a move for the Black Checks condition; the higher the
 * value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type blackchecks_measure_length(void)
{
   return move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival!=nullsquare;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (SLICE_STARTER(si)==Black)
  {
    slice_index const prototype = alloc_pipe(STBlackChecks);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for BlackChecks
 * @param si identifies root slice of stipulation
 */
void blackchecks_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nullmoves_initialise_solving(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&stip_structure_visitor_noop);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

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
void blackchecks_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,
                            move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival!=nullsquare
                            && !is_in_check(White));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
