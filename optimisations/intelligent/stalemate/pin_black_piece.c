#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Detrmine whether some line is empty
 * @param start start of line
 * @param end end of line
 * @param dir direction from start to end
 * @return true iff the line is empty
 */
static boolean is_line_empty(square start, square end, int dir)
{
  boolean result = true;
  square sq;

  TraceFunctionEntry(__func__);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (sq = start+dir; e[sq]==vide; sq += dir)
  {
    /* nothing */
  }

  result = sq==end;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Pin the piece on a specific square with an original rider
 * @param sq_to_be_pinned position of piece to be pinned
 * @param pin_on where to put pinner
 * @param pinner_index identifiespinnter
 * @param is_pin_on_diagonal is the piece to be pinned on a diagonal
 */
static void pin_by_rider(piece pinner_type,
                         Flags pinner_flags,
                         square pinner_comes_from,
                         square pin_from)
{
  TraceFunctionEntry(__func__);
  TracePiece(pinner_type);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(pinner_comes_from,
                                                pin_from,
                                                pinner_type))
  {
    SetPiece(pinner_type,pin_from,pinner_flags);
    intelligent_stalemate_test_target_position();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Pin the piece on a specific square with a promoted rider
 * @param sq_to_be_pinned position of piece to be pinned
 * @param pin_on where to put pinner
 * @param pinner_index identifiespinnter
 * @param is_pin_on_diagonal is the piece to be pinned on a diagonal
 */
static void pin_by_promoted_pawn(Flags pinner_flags,
                                 square pinner_comes_from,
                                 square pin_from,
                                 boolean is_pin_on_diagonal)
{
  piece const minor_pinner_type = is_pin_on_diagonal ? fb : tb;

  TraceFunctionEntry(__func__);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParam("%u",is_pin_on_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(pinner_comes_from,
                                                             db,
                                                             pin_from))
  {
    SetPiece(db,pin_from,pinner_flags);
    intelligent_stalemate_test_target_position();
    intelligent_unreserve();
  }

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(pinner_comes_from,
                                                             minor_pinner_type,
                                                             pin_from))
  {
    SetPiece(minor_pinner_type,pin_from,pinner_flags);
    intelligent_stalemate_test_target_position();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Pin the piece on a specific square with an original rider
 * @param sq_to_be_pinned position of piece to be pinned
 * @param pin_on where to put pinner
 * @param pinner_index identifiespinnter
 * @param is_pin_on_diagonal is the piece to be pinned on a diagonal
 */
static void pin_specific_piece_on(square sq_to_be_pinned,
                                  square pin_on,
                                  unsigned int pinner_index,
                                  boolean is_pin_on_diagonal)
{
  piece const pinner_type = white[pinner_index].type;
  Flags const pinner_flags = white[pinner_index].flags;
  square const pinner_comes_from = white[pinner_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_to_be_pinned);
  TraceSquare(pin_on);
  TraceFunctionParam("%u",pinner_index);
  TraceFunctionParam("%u",is_pin_on_diagonal);
  TraceFunctionParamListEnd();
  switch (pinner_type)
  {
    case db:
      pin_by_rider(db,pinner_flags,pinner_comes_from,pin_on);
      break;

    case tb:
      if (!is_pin_on_diagonal)
        pin_by_rider(tb,pinner_flags,pinner_comes_from,pin_on);
      break;

    case fb:
      if (is_pin_on_diagonal)
        pin_by_rider(fb,pinner_flags,pinner_comes_from,pin_on);
      break;

    case cb:
      break;

    case pb:
      pin_by_promoted_pawn(pinner_flags,pinner_comes_from,pin_on,is_pin_on_diagonal);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Pin a mobile black piece
 * @param position_of_trouble_maker position of piece to be pinned
 */
void intelligent_stalemate_pin_black_piece(square position_of_trouble_maker)
{
  TraceFunctionEntry(__func__);
  TraceSquare(position_of_trouble_maker);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    int const dir = CheckDir[Queen][position_of_trouble_maker-king_square[Black]];
    piece const pinned_type = e[position_of_trouble_maker];

    if (dir!=0          /* we can only pin on queen lines */
        && pinned_type!=dn /* queens cannot be pinned */
        /* bishops can only be pined on rook lines and vice versa */
        && !(CheckDir[Bishop][dir]!=0 && pinned_type==fn)
        && !(CheckDir[Rook][dir]!=0 && pinned_type==tn)
        && is_line_empty(king_square[Black],position_of_trouble_maker,dir))
    {
      boolean const is_pin_on_diagonal = SquareCol(king_square[Black]+dir)==SquareCol(king_square[Black]);

      square pin_on;
      for (pin_on = position_of_trouble_maker+dir; e[pin_on]==vide; pin_on += dir)
      {
        if (nr_reasons_for_staying_empty[pin_on]==0)
        {
          unsigned int pinner_index;
          for (pinner_index = 1; pinner_index<MaxPiece[White]; ++pinner_index)
          {
            if (white[pinner_index].usage==piece_is_unused)
            {
              white[pinner_index].usage = piece_pins;

              pin_specific_piece_on(position_of_trouble_maker,
                                    pin_on,
                                    pinner_index,
                                    is_pin_on_diagonal);

              white[pinner_index].usage = piece_is_unused;
            }
          }

          e[pin_on] = vide;
          spec[pin_on] = EmptySpec;
        }

        ++nr_reasons_for_staying_empty[pin_on];
      }

      for (pin_on -= dir; pin_on!=position_of_trouble_maker; pin_on -= dir)
        --nr_reasons_for_staying_empty[pin_on];
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
