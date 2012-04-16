#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "platform/maxtime.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Place some unused black piece on some square
 * @param where_to_place where to place some piece
 */
static void place_some_piece_on(square where_to_place)
{
  TraceFunctionEntry(__func__);
  TraceSquare(where_to_place);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    unsigned int i;
    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_blocks;
        intelligent_place_black_piece(i,
                                      where_to_place,
                                      &intelligent_stalemate_test_target_position);
        black[i].usage = piece_is_unused;
      }

    e[where_to_place] = vide;
    spec[where_to_place] = EmptySpec;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place some unused black piece
 */
static void place_some_piece(void)
{
  square const *bnp;
  square const * const save_start = where_to_start_placing_black_pieces;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = where_to_start_placing_black_pieces; *bnp; ++bnp)
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0)
    {
      where_to_start_placing_black_pieces = bnp;
      place_some_piece_on(*bnp);
    }

  where_to_start_placing_black_pieces = save_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* fix the white king on its diagram square
 */
static void fix_white_king_on_diagram_square(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  white[index_of_king].usage = piece_is_fixed_to_diagram_square;

  intelligent_place_white_king(white[index_of_king].diagram_square,
                               &intelligent_stalemate_test_target_position);

  white[index_of_king].usage = piece_is_unused;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deal with unused black pieces
 */
void intelligent_stalemate_deal_with_unused_pieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].diagram_square!=square_e1
        && intelligent_get_nr_remaining_moves(White)==0)
    {
      if (e[white[index_of_king].diagram_square]==vide
          && nr_reasons_for_staying_empty[white[index_of_king].diagram_square]==0)
        fix_white_king_on_diagram_square();
    }
    else
    {
      unsigned int unused = 0;

      {
        unsigned int i;
        for (i = 1; i<MaxPiece[Black]; ++i)
          if (black[i].usage==piece_is_unused)
            ++unused;
      }

      TraceValue("%u\n",piece_is_unused);
      TraceValue("%u\n",MovesLeft[White]);
      if (unused>0)
        place_some_piece();

      if (unused<=MovesLeft[White])
      {
        CapturesLeft[1] = unused;
        solve_target_position();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
