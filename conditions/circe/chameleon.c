#include "conditions/circe/chameleon.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "conditions/circe/circe.h"
#include "pieces/walks.h"
#include "debugging/trace.h"

#include <assert.h>

static PieNam reborn_pieces[PieceCount];

static boolean are_reborn_pieces_implicit;

/* Reset the mapping from captured to reborn pieces
 */
void chameleon_circe_reset_reborn_pieces(void)
{
  PieNam p;
  for (p = Empty; p!=PieceCount; ++p)
    reborn_pieces[p] = p;

  are_reborn_pieces_implicit = true;
}

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured piece
 * @param reborn type of reborn piece if a piece of type captured is captured
 */
void chameleon_circe_set_reborn_piece_explicit(PieNam from, PieNam to)
{
  reborn_pieces[from] = to;
  are_reborn_pieces_implicit = false;
}

/* Initialise the reborn pieces if they haven't been already initialised
 * from the explicit indication
 */
void chameleon_circe_init_implicit(void)
{
  if (are_reborn_pieces_implicit)
  {
    reborn_pieces[standard_walks[Knight]] = standard_walks[Bishop];
    reborn_pieces[standard_walks[Bishop]] = standard_walks[Rook];
    reborn_pieces[standard_walks[Rook]] = standard_walks[Queen];
    reborn_pieces[standard_walks[Queen]] = standard_walks[Knight];
  }
}

/* What kind of piece is to be reborn if a certain piece is captured?
 * @param captured kind of captured piece
 * @return kind of piece to be reborn
 */
PieNam chameleon_circe_get_reborn_piece(PieNam captured)
{
  return reborn_pieces[captured];
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
stip_length_type chameleon_circe_adapt_reborn_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_circe_reborn_piece[nbply] = chameleon_circe_get_reborn_piece(current_circe_reborn_piece[nbply]);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_chameleon_circe(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonCirceAdaptRebornPiece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
