#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static unsigned int current_level;
static unsigned int iteration_level;

void post_move_iteration_init_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",current_level);TraceEOL();

  if (iteration_level<current_level)
    iteration_level = current_level;

  TraceValue("%u",iteration_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean post_move_iteration_ply_was_ended(void)
{
  boolean result = iteration_level==current_level;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void post_move_iteration_end(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  assert(iteration_level==current_level+1);
  --iteration_level;

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

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

  ++current_level;
  TraceValue("%u",current_level);TraceEOL();

  if (current_level>iteration_level)
  {
    iteration_level = current_level;
    TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();
  }

  pipe_solve_delegate(si);

  --current_level;
  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void post_move_iteration_solve_fork(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++current_level;
  TraceValue("%u",current_level);TraceEOL();

  if (current_level>iteration_level)
  {
    iteration_level = current_level;
    TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();
  }

  fork_solve_delegate(si);

  --current_level;
  TraceValue("%u",current_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Recurse into the same pipe while post move iterating
 * @param si identifies the iterating slice
 */
void post_move_iteration_solve_recurse(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++current_level;
  TraceValue("%u",current_level);TraceEOL();

  if (current_level>iteration_level)
  {
    iteration_level = current_level;
    TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();
  }

  solve(si);

  --current_level;
  TraceValue("%u",current_level);TraceEOL();

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

  TraceValue("%u",iteration_level);
  TraceValue("%u",current_level);
  TraceEOL();

  iteration_level = current_level;
  TraceValue("%u",iteration_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is post move iteration locked by an inner iteration?
 * @param *lock our lock
 * @return true iff is post move iteration is locked
 */
boolean post_move_iteration_is_locked(void)
{
  boolean const result = iteration_level>current_level+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

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
  boolean const result = iteration_level>current_level;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the current post move iteration participant has the lock
 * @param true iff it has
 */
boolean post_move_have_i_lock(void)
{
  boolean const result = iteration_level==current_level+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

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

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  pipe_solve_delegate(si);

  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  if (iteration_level==current_level)
    pop_move();

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
    pipe_move_generation_delegate(si);
    TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();
  } while (iteration_level>current_level);

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

  ++current_level;
  TraceValue("%u",current_level);TraceEOL();

  if (current_level>iteration_level)
  {
    iteration_level = current_level;
    TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();
  }

  do
  {
    pipe_is_square_observed_delegate(si);
  } while (iteration_level>current_level && !observation_result);

  --current_level;
  TraceValue("%u",current_level);TraceEOL();

  post_move_iteration_cancel();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a square is observed by a side
 * @param side observing side
 * @param s the square
 * @param evaluate identifies the set of restrictions imposed on the observation
 * @return true iff the square is observed
 */
boolean is_square_observed_general_post_move_iterator_solve(Side side,
                                                            square s,
                                                            validator_id evaluate)
{
  unsigned int const save_iteration_level = iteration_level;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++current_level;
  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  result = is_square_observed_general(side,s,evaluate);

  --current_level;
  iteration_level = save_iteration_level;
  TraceValue("%u",iteration_level);TraceValue("%u",current_level);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
