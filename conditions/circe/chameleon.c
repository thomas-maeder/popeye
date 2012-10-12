#include "conditions/circe/chameleon.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "conditions/circe/rebirth_handler.h"
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
    if (CondFlag[leofamily])
    {
      reborn_pieces[Leo] = Mao;
      reborn_pieces[Pao] = Leo;
      reborn_pieces[Vao] = Pao;
      reborn_pieces[Mao] = Vao;
    }
    else
    {
      PieNam const knight = CondFlag[cavaliermajeur] ? NightRider : Knight;
      reborn_pieces[knight] = Bishop;
      reborn_pieces[Bishop] = Rook;
      reborn_pieces[Rook] = Queen;
      reborn_pieces[Queen] = knight;
    }
  }
}

/* What kind of piece is to be reborn if a certain piece is captured?
 * @param captured kind of captured piece
 * @return kind of piece to be reborn
 */
piece chameleon_circe_get_reborn_piece(piece captured)
{
  return (captured<vide
          ? -reborn_pieces[-captured]
          : reborn_pieces[captured]);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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
