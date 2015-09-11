#include "solving/zeroposition.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "debugging/trace.h"

/* Instrument the solving machinery with zeroposition logic
 * @param si hook for instrumentation
 */
void problem_instrument_zeroposition(slice_index si)
{
  slice_index const prototypes[] =
  {
      alloc_pipe(STZeroPositionInitialiser)
  };
  enum
  {
    nr_prototypes = sizeof prototypes / sizeof prototypes[0]
  };
  slice_insertion_insert(si,prototypes,nr_prototypes);
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
void zeroposition_initialiser_solve(slice_index si)
{
  slice_index const prototype = alloc_pipe(STZeropositionSolvingStopper);
  slice_insertion_insert(si,&prototype,1);

  pipe_solve_delegate(si);

  pipe_remove(si);
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
void zeroposition_solving_stopper_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
