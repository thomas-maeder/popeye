#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

boolean post_move_iteration_locked[maxply+1];
unsigned int post_move_iteration_id[maxply+1];
post_move_iteration_id_type post_move_iteration_id_watermark;

/* Lock post move iterations in the current move retraction
 */
void lock_post_move_iterations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  post_move_iteration_locked[nbply] = true;
  ++post_move_iteration_id[nbply];
  TraceValue("%u",nbply);TraceValue("%u",post_move_iteration_id[nbply]);TraceEOL();
  if (post_move_iteration_id[nbply]<post_move_iteration_id_watermark)
    post_move_iteration_id[nbply] = ++post_move_iteration_id_watermark;
  else if (post_move_iteration_id[nbply]>post_move_iteration_id_watermark)
    post_move_iteration_id_watermark = post_move_iteration_id[nbply];

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
void move_execution_post_move_iterator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (post_move_iteration_locked[nbply])
    post_move_iteration_locked[nbply] = false;
  else
  {
    pop_move();
    ++post_move_iteration_id[nbply];
    TraceValue("%u",nbply);TraceValue("%u",post_move_iteration_id[nbply]);TraceEOL();
    if (post_move_iteration_id[nbply]<post_move_iteration_id_watermark)
      post_move_iteration_id[nbply] = ++post_move_iteration_id_watermark;
    else if (post_move_iteration_id[nbply]>post_move_iteration_id_watermark)
      post_move_iteration_id_watermark = post_move_iteration_id[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate if move generation uses a post-move iterating slice (e.g. for
 * MarsCirce Super)
 * @param si identifies move generator slice
 */
void move_generation_post_move_iterator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    post_move_iteration_locked[nbply] = false;
    pipe_move_generation_delegate(si);
  } while (post_move_iteration_locked[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate if square observation testing uses a post-move iterating slice (e.g.
 * for MarsCirce Super)
 * @param si identifies move generator slice
 */
void square_observation_post_move_iterator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    post_move_iteration_locked[nbply] = false;
    pipe_is_square_observed_delegate(si);
  } while (post_move_iteration_locked[nbply] && !observation_result);

  post_move_iteration_locked[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with post move iteration slices
 * @param si identifies the root of the solving machinery
 */
void solving_insert_post_move_iteration(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPostMoveIterationInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
