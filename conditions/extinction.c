#include "conditions/extinction.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

static unsigned int prev_nbpiece[PieceCount];

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_extinction_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STExtinctionRememberThreatened);
  stip_instrument_moves(si,STExtinctionTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect whether the move just played has extincted a kind
 */
static boolean move_extincts_kind(Side starter)
{
  PieNam walk;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  for (walk = King; walk<PieceCount; ++walk)
    if (prev_nbpiece[walk]>0)
    {
      piece const p = starter==White ? walk : -walk;
      if (nbpiece[p]==0)
      {
        result = true;
        break;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type extinction_remember_threatened_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;
  PieNam walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (walk = King; walk<PieceCount; walk++)
  {
    piece const p = starter==White ? walk : -walk;
    prev_nbpiece[walk] = nbpiece[p];
  }

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type extinction_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_extincts_kind(starter))
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
