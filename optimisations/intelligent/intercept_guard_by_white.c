#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "trace.h"

#include <assert.h>

/* Intercept with a white piece on some square
 * @param where_to_intercept where to intercept
 * @param go_on what to do after each successful interception?
 */
static void black_piece_on(square where_to_intercept, void (*go_on)(void))
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      black[i].usage = piece_intercepts;
      intelligent_place_black_piece(i,where_to_intercept,go_on);
      black[i].usage = piece_is_unused;
    }

  e[where_to_intercept] = vide;
  spec[where_to_intercept] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a black piece
 * @param guard of what square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void black_piece(square target, int dir_from_rider, void (*go_on)(void))
{
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = target-dir_from_rider;
       e[where_to_intercept]==vide;
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0
      /* avoid testing the same position twice */
        && *where_to_start_placing_black_pieces<=where_to_intercept)
      black_piece_on(where_to_intercept,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a promoted white pawn
 * @param intercepter_index identifies intercepting pawn
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void promoted_white_pawn(unsigned int intercepter_index,
                                square where_to_intercept,
                                boolean is_diagonal,
                                void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",intercepter_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(intercepter_index,
                                                                where_to_intercept))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      switch (pp)
      {
        case db:
          break;

        case tb:
          if (is_diagonal)
            intelligent_place_promoted_white_rider(tb,
                                                   intercepter_index,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case fb:
          if (!is_diagonal)
            intelligent_place_promoted_white_rider(fb,
                                                   intercepter_index,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case cb:
          intelligent_place_promoted_white_knight(intercepter_index,
                                                  where_to_intercept,
                                                  go_on);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with the white king on a square
 * @param guard of what square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void white_king(square target,
                       int dir_from_rider,
                       void (*go_on)(void))
{
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%u",dir_from_rider);
  TraceFunctionParamListEnd();

  white[index_of_king].usage = piece_intercepts;

  for (where_to_intercept = target-dir_from_rider;
       e[where_to_intercept]==vide;
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      intelligent_place_white_king(where_to_intercept,go_on);

  white[index_of_king].usage = piece_is_unused;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a white piece on a square
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void white_piece_on(square where_to_intercept,
                           boolean is_diagonal,
                           void (*go_on)(void))
{
  unsigned int intercepter_index;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  for (intercepter_index = 1; intercepter_index<MaxPiece[White]; ++intercepter_index)
    if (white[intercepter_index].usage==piece_is_unused)
    {
      white[intercepter_index].usage = piece_intercepts;

      switch (white[intercepter_index].type)
      {
        case db:
          break;

        case tb:
          if (is_diagonal)
            intelligent_place_white_rider(intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case fb:
          if (!is_diagonal)
            intelligent_place_white_rider(intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case cb:
          intelligent_place_white_knight(intercepter_index,
                                         where_to_intercept,
                                         go_on);
          break;

        case pb:
          promoted_white_pawn(intercepter_index,where_to_intercept,is_diagonal,go_on);
          intelligent_place_unpromoted_white_pawn(intercepter_index,
                                                  where_to_intercept,
                                                  go_on);
          break;

        default:
          assert(0);
          break;
      }

      white[intercepter_index].usage = piece_is_unused;
    }

  e[where_to_intercept] = vide;
  spec[where_to_intercept] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a white piece
 * @param target guard of what square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void white_piece(square target, int dir_from_rider, void (*go_on)(void))
{
  square const start = target-dir_from_rider;
  boolean const is_diagonal = SquareCol(start)==SquareCol(target);
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = start;
       e[where_to_intercept]==vide;
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      white_piece_on(where_to_intercept,is_diagonal,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a guard by white
 * @param target guard of what square
 * @param dir_from_rider direction from guarding rider
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_guard_by_white(square target,
                                          int dir_from_rider,
                                          void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    black_piece(target,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  if (white[index_of_king].usage==piece_is_unused)
    white_king(target,dir_from_rider,go_on);

  if (intelligent_reserve_masses(White,1))
  {
    white_piece(target,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
