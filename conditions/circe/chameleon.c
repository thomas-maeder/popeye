#include "conditions/circe/chameleon.h"
#include "pieces/pieces.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "conditions/circe/circe.h"
#include "pieces/walks/walks.h"
#include "debugging/trace.h"

#include <assert.h>

static PieNam reborn_walks[PieceCount];

static boolean are_reborn_walks_implicit;

/* Reset the mapping from captured to reborn walks
 */
void chameleon_circe_reset_reborn_walks(void)
{
  PieNam p;
  for (p = Empty; p!=PieceCount; ++p)
    reborn_walks[p] = p;

  are_reborn_walks_implicit = true;
}

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured walk
 * @param reborn type of reborn walk if a piece with walk captured is captured
 */
void chameleon_circe_set_reborn_walk_explicit(PieNam from, PieNam to)
{
  reborn_walks[from] = to;
  are_reborn_walks_implicit = false;
}

/* Initialise the reborn pieces if they haven't been already initialised
 * from the explicit indication
 */
void chameleon_circe_init_implicit(void)
{
  if (are_reborn_walks_implicit)
  {
    reborn_walks[standard_walks[Knight]] = standard_walks[Bishop];
    reborn_walks[standard_walks[Bishop]] = standard_walks[Rook];
    reborn_walks[standard_walks[Rook]] = standard_walks[Queen];
    reborn_walks[standard_walks[Queen]] = standard_walks[Knight];
  }
}

/* What kind of piece is to be reborn if a certain piece is captured?
 * @param captured kind of captured piece
 * @return kind of piece to be reborn
 */
PieNam chameleon_circe_get_reborn_walk(PieNam captured)
{
  return reborn_walks[captured];
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_circe_adapt_reborn_walk_solve(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk = chameleon_circe_get_reborn_walk(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk);
  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_walk = circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Override the Circe instrumentation of the solving machinery with
 * Chameleon Circe
 * @param si identifies root slice of stipulation
 */
void chameleon_circe_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonCirceAdaptRebornWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
