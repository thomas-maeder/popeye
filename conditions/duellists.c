#include "conditions/duellists.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

square duellists[nr_sides][maxply+1];

/* Determine the length of a move for the Duellists condition; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int duellists_measure_length_white(square sq_departure,
                                   square sq_arrival,
                                   square sq_capture)
{
  return sq_departure==duellists[White][parent_ply[nbply]];
}

/* Determine the length of a move for the Duellists condition; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int duellists_measure_length_black(square sq_departure,
                                   square sq_arrival,
                                   square sq_capture)
{
  return sq_departure==duellists[Black][parent_ply[nbply]];
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type duellists_remember_duellist_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  duellists[advers(slices[si].starter)][nbply] = duellists[advers(slices[si].starter)][parent_ply[nbply]];
  duellists[slices[si].starter][nbply] = move_generation_stack[current_move[nbply]].arrival;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_duellists(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDuellistsRememberDuellist);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
