#include "solving/legal_move_counter.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* current value of the count */
unsigned int legal_move_counter_count[maxply];

/* stop the move iteration once legal_move_counter_count exceeds this number */
unsigned int legal_move_counter_interesting[maxply];

#if !defined(NDEBUG)
static boolean is_init[maxply];
#endif

/* Allocate a STLegalAttackCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_attack_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLegalAttackCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STLegalDefenseCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_defense_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLegalDefenseCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STAnyMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_any_move_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnyMoveCounter);

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
void legal_attack_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++legal_move_counter_count[parent_ply[nbply]];

  assert(is_init[parent_ply[nbply]]);

  pipe_this_move_solves_exactly_if(legal_move_counter_count[parent_ply[nbply]]
                                   >legal_move_counter_interesting[parent_ply[nbply]]);

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
void legal_defense_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++legal_move_counter_count[parent_ply[nbply]];

  assert(is_init[parent_ply[nbply]]);

  pipe_this_move_solves_exactly_if(legal_move_counter_count[parent_ply[nbply]]
                                   <=legal_move_counter_interesting[parent_ply[nbply]]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void legal_move_count_init(unsigned int nr_interesting)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_interesting);
  TraceFunctionParamListEnd();

  assert(legal_move_counter_count[nbply]==0);
  legal_move_counter_interesting[nbply] = nr_interesting;

#if !defined(NDEBUG)
  is_init[nbply] = true;
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void legal_move_count_fini(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  legal_move_counter_count[nbply] = 0;

#if !defined(NDEBUG)
  is_init[nbply] = false;
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
