#include "conditions/isardam.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "conditions/madrasi.h"
#include "debugging/trace.h"

#include <assert.h>

static boolean pos_legal(void)
{
  square square_h = square_h8;
  boolean result = true;
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nextply(); /* for e.p. captures */

  for (i = nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    int j;
    square z = square_h;
    for (j = nr_files_on_board; j>0; j--, z += dir_left)
      if (e[z]!=vide && madrasi_is_observed(z))
      {
        result = false;
        break;
      }
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_isardam_legality_testers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STIsardamLegalityTester);

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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type isardam_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pos_legal())
    result = solve(next,n);
  else
    result = previous_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
