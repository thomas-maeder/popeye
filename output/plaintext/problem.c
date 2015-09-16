#include "output/plaintext/problem.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/position.h"
#include "output/plaintext/twinning.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"

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
void output_plaintext_problem_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_plaintext_build_position_writers(si);

  {
    slice_index const prototypes[] =
    {
        output_plaintext_alloc_twin_intro_writer_builder(),
        alloc_pipe(STOutputPlainTextInstrumentSolversBuilder)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);

  output_plaintext_print_time(" ","");
  output_plaintext_message(NewLine);
  output_plaintext_message(NewLine);
  output_plaintext_message(NewLine);
  protocol_fflush(stdout);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
