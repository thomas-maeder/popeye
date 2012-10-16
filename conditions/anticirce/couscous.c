#include "conditions/anticirce/couscous.h"
#include "conditions/anticirce/anticirce.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_couscous_determine_relevant_piece_solve(slice_index si,
                                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_anticirce_relevant_piece[nbply] = pprise[nbply];
  current_anticirce_relevant_spec[nbply] = pprispec[nbply];
  current_anticirce_relevant_side[nbply] = slices[si].starter;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_couscous(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_replace_anticirce_determine_relevant_piece(si,STAnticirceCouscousDetermineRevelantPiece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
