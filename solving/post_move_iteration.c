#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

unsigned int post_move_iteration_id[maxply+1];

post_move_iteration_id_type post_move_iteration_stack[post_move_iteration_stack_size];

unsigned int post_move_iteration_stack_pointer = 1;

static boolean post_move_iteration_lock_pointer[maxply+1];

static unsigned int post_move_iteration_highwater[maxply+1];

void post_move_iteration_init_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceValue("%u",post_move_iteration_stack[post_move_iteration_stack_pointer]);
  TraceEOL();

  assert(post_move_iteration_stack[post_move_iteration_stack_pointer]==0);

  post_move_iteration_id[nbply] = 1;
  post_move_iteration_highwater[nbply] = post_move_iteration_stack_pointer;

  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Lock post move iterations in the current move retraction
 */
void post_move_iteration_lock(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  post_move_iteration_lock_pointer[nbply] = true;
  ++post_move_iteration_id[nbply];
  TraceValue("%u",nbply);TraceValue("%u",post_move_iteration_id[nbply]);TraceEOL();

  post_move_iteration_stack[post_move_iteration_stack_pointer] = post_move_iteration_id[nbply];
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceValue("%u",post_move_iteration_stack[post_move_iteration_stack_pointer]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void post_move_iteration_end(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();
  assert(post_move_iteration_highwater[nbply]==post_move_iteration_stack_pointer+1
         || post_move_iteration_highwater[nbply]==post_move_iteration_stack_pointer);

  post_move_iteration_highwater[nbply] = post_move_iteration_stack_pointer;
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceEOL();

  post_move_iteration_stack[post_move_iteration_stack_pointer] = 0;
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceValue("%u",post_move_iteration_stack[post_move_iteration_stack_pointer]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve the next pipe while post move iterating
 * @param si identifies the iterating slice
 */
void post_move_iteration_solve_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(post_move_iteration_stack_pointer<post_move_iteration_stack_size);

  post_move_iteration_highwater[nbply] = ++post_move_iteration_stack_pointer;

  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();

  pipe_solve_delegate(si);

  --post_move_iteration_stack_pointer;
  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
void post_move_iteration_solve_fork(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(post_move_iteration_stack_pointer<post_move_iteration_stack_size);

  post_move_iteration_highwater[nbply] = ++post_move_iteration_stack_pointer;

  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();

  fork_solve_delegate(si);

  --post_move_iteration_stack_pointer;
  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Unlock post move iteration while taking back a move.
 * Useful in rare situations where e.g. promotion to queen is enough.
 */
void post_move_iteration_cancel(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_highwater[nbply]);
  TraceValue("%u",post_move_iteration_stack[post_move_iteration_highwater[nbply]]);
  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceEOL();

  while (post_move_iteration_highwater[nbply]>=post_move_iteration_stack_pointer)
  {
    post_move_iteration_stack[post_move_iteration_highwater[nbply]--] = 0;
    TraceValue("%u",post_move_iteration_highwater[nbply]);
    TraceValue("%u",post_move_iteration_stack[post_move_iteration_highwater[nbply]]);
    TraceEOL();
  }

  post_move_iteration_lock_pointer[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is post move iteration locked by an inner iteration?
 * @param *lock our lock
 * @return true iff is post move iteration is locked
 */
boolean post_move_iteration_is_locked(void)
{
  boolean result = post_move_iteration_lock_pointer[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (result)
  {
    post_move_iteration_stack[post_move_iteration_stack_pointer] = post_move_iteration_id[nbply];
    TraceValue("%u",post_move_iteration_stack_pointer);
    TraceValue("%u",post_move_iteration_stack[post_move_iteration_stack_pointer]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is the post move iterator holding an specific id iterating in the current ply?
 * @return true iff he is
 */
boolean post_move_am_i_iterating(void)
{
  boolean const result = post_move_iteration_id[nbply]==post_move_iteration_stack[post_move_iteration_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",post_move_iteration_stack_pointer);
  TraceValue("%u",post_move_iteration_stack[post_move_iteration_stack_pointer]);
  TraceValue("%u",nbply);
  TraceValue("%u",post_move_iteration_id[nbply]);
  TraceEOL();

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
void move_execution_post_move_iterator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (post_move_iteration_lock_pointer[nbply])
    post_move_iteration_lock_pointer[nbply] = false;
  else
  {
    pop_move();
    ++post_move_iteration_id[nbply];
    TraceValue("%u",nbply);TraceValue("%u",post_move_iteration_id[nbply]);TraceEOL();
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
    post_move_iteration_lock_pointer[nbply] = false;
    pipe_move_generation_delegate(si);
  } while (post_move_iteration_lock_pointer[nbply]);

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
    post_move_iteration_lock_pointer[nbply] = false;
    pipe_is_square_observed_delegate(si);
  } while (post_move_iteration_lock_pointer[nbply] && !observation_result);

  post_move_iteration_lock_pointer[nbply] = false;

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
