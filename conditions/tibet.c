#include "conditions/tibet.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "conditions/andernach.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type double_tibet_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  square const sq_departure = move_generation_stack[coup_id].departure;
  square const sq_arrival = move_generation_stack[coup_id].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]!=vide && e[sq_arrival]!=-pprise[nbply]
      && sq_departure!=prev_king_square[slices[si].starter][nbply])
    andernach_assume_side(advers(slices[si].starter));

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_double_tibet(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDoubleTibetSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type tibet_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  square const sq_departure = move_generation_stack[coup_id].departure;
  square const sq_arrival = move_generation_stack[coup_id].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (slices[si].starter==Black
      && pprise[nbply]!=vide
      && e[sq_arrival]!=-pprise[nbply]
      && sq_departure!=prev_king_square[Black][nbply])
    andernach_assume_side(advers(slices[si].starter));

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_tibet(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STTibetSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
