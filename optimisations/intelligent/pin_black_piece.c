#include "optimisations/intelligent/pin_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void (*go_on_after)(void);

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
                                                pinner_type,
                                                pin_from))
  {
    SetPiece(pinner_type,pin_from,pinner_flags);
    (*go_on_after)();
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
    (*go_on_after)();
    intelligent_unreserve();
  }

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(pinner_comes_from,
                                                             minor_pinner_type,
                                                             pin_from))
  {
    SetPiece(minor_pinner_type,pin_from,pinner_flags);
    (*go_on_after)();
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
 * @param dir_to_touble_maker direction from king to piece to be pinned
 */
void intelligent_pin_black_piece(square position_of_trouble_maker,
                                 void (*go_on)(void))
{
  int const dir_to_touble_maker = CheckDir[Queen][position_of_trouble_maker-king_square[Black]];
  void (* const save_go_on)(void) = go_on_after;

  TraceFunctionEntry(__func__);
  TraceSquare(position_of_trouble_maker);
  TraceFunctionParamListEnd();

  if (dir_to_touble_maker!=0 /* we can only pin on queen lines */
      && is_line_empty(king_square[Black],
                       position_of_trouble_maker,
                       dir_to_touble_maker)
      && intelligent_reserve_masses(White,1))
  {
    boolean const is_pin_on_diagonal = SquareCol(king_square[Black]+dir_to_touble_maker)==SquareCol(king_square[Black]);

    square pin_on;

    go_on_after = go_on;

    remember_to_keep_rider_line_open(king_square[Black],
                                     position_of_trouble_maker,
                                     dir_to_touble_maker,
                                     +1);

    for (pin_on = position_of_trouble_maker+dir_to_touble_maker; e[pin_on]==vide; pin_on += dir_to_touble_maker)
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

    for (pin_on -= dir_to_touble_maker; pin_on!=position_of_trouble_maker; pin_on -= dir_to_touble_maker)
      --nr_reasons_for_staying_empty[pin_on];

    remember_to_keep_rider_line_open(king_square[Black],
                                     position_of_trouble_maker,
                                     dir_to_touble_maker,
                                     -1);

    go_on_after = save_go_on;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
