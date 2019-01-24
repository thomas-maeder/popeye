#include "pieces/attributes/total_invisible.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int total_invisible_number = 1;

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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);
  solve_result = MOVE_HAS_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    pipe_link(substitute,state->the_copy);
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type state = { false, no_slice };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - prevent creation of self check guards
  //   - prevent king capture?
  //   - generate pawn captures to empty squares (if an invisible piece is left)
  //   - insert visibilisation logic
  // - in copy
  //   - ensure creation of self check guards
  //   - move repeaters instead of move generators
  //     - based on move effects journal (e.g. necessary for pawn promotions)
  //   - logic for iteration over all possibilities of invisibles

  // bail out at STAttackAdapter

  // input for total_invisible_number, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // STIllegalSelfcheckWriter: restore creation for everything except invisible
  // in solving_machinery_intro_builder_solve()


  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STHelpAdapter,
                                           &copy_help_branch);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &insert_copy);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
