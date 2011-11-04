#include "optimisations/intelligent/intercept_check_by_white.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Intercept a check to the white king with a white piece on some square
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

/* Intercept a check to the black king with a black piece
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void black_piece(int dir_from_rider, void (*go_on)(void))
{
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (where_to_intercept = king_square[Black]-dir_from_rider;
       e[where_to_intercept]==vide;
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0
      /* avoid testing the same position twice */
        && *where_to_start_placing_black_pieces<=where_to_intercept)
      black_piece_on(where_to_intercept,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place a promoted black piece on the check line
 * @param intercepter_index identifies intercepting pawn
 * @param where_to_intercept what square
 * @param go_on what to do after each successful interception?
 */
static void place_white_promotee(unsigned int blocker_index,
                                 piece promotee_type,
                                 square where_to_intercept,
                                 void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TracePiece(promotee_type);
  TraceSquare(where_to_intercept);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[blocker_index].diagram_square,
                                                             promotee_type,
                                                             where_to_intercept))
  {
    SetPiece(promotee_type,where_to_intercept,white[blocker_index].flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the black king with a promoted white pawn
 * @param intercepter_index identifies intercepting pawn
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void promoted_white_pawn(unsigned int blocker_index,
                                square where_to_intercept,
                                boolean is_diagonal,
                                void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(blocker_index,
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
            place_white_promotee(blocker_index,tb,where_to_intercept,go_on);
          break;

        case fb:
          if (!is_diagonal)
            place_white_promotee(blocker_index,fb,where_to_intercept,go_on);
          break;

        case cb:
          place_white_promotee(blocker_index,cb,where_to_intercept,go_on);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the black king with the white king on a square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void white_king(int dir_from_rider, void (*go_on)(void))
{
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  white[index_of_king].usage = piece_intercepts;

  for (where_to_intercept = king_square[Black]-dir_from_rider;
       e[where_to_intercept]==vide;
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      intelligent_place_white_king(where_to_intercept,go_on);

  white[index_of_king].usage = piece_is_unused;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king with a white officer on a square
 * @param intercepter_index identifies intercepting officer
 * @param where_to_intercept what square
 * @param go_on what to do after each successful interception?
 */
static void white_officer(unsigned int blocker_index,
                          square where_to_intercept,
                          void (*go_on)(void))
{
  piece const intercepter_type = white[blocker_index].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(white[blocker_index].diagram_square,
                                                intercepter_type,
                                                where_to_intercept))
  {
    SetPiece(intercepter_type,where_to_intercept,white[blocker_index].flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king with a white piece on a square
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void white_piece_on(square where_to_intercept,
                           boolean is_diagonal,
                           void (*go_on)(void))
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
    if (white[blocker_index].usage==piece_is_unused)
    {
      white[blocker_index].usage = piece_intercepts;

      switch (white[blocker_index].type)
      {
        case db:
          break;

        case tb:
          if (is_diagonal)
            white_officer(blocker_index,where_to_intercept,go_on);
          break;

        case fb:
          if (!is_diagonal)
            white_officer(blocker_index,where_to_intercept,go_on);
          break;

        case cb:
          white_officer(blocker_index,where_to_intercept,go_on);
          break;

        case pb:
          promoted_white_pawn(blocker_index,where_to_intercept,is_diagonal,go_on);
          intelligent_place_unpromoted_white_pawn(blocker_index,
                                                  where_to_intercept,
                                                  go_on);
          break;

        default:
          assert(0);
          break;
      }

      white[blocker_index].usage = piece_is_unused;
    }

  e[where_to_intercept] = vide;
  spec[where_to_intercept] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the black king with a white piece
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void white_piece(int current_dir, void (*go_on)(void))
{
  square const start = king_square[Black]-current_dir;
  boolean const is_diagonal = SquareCol(start)==SquareCol(king_square[Black]);
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (where_to_intercept = start;
       e[where_to_intercept]==vide;
       where_to_intercept -= current_dir)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      white_piece_on(where_to_intercept,is_diagonal,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the black king
 * @param dir_from_rider direction from rider giving check to white king
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_check_by_white(int dir_from_rider, void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    black_piece(dir_from_rider,go_on);
    intelligent_unreserve();
  }

  if (white[index_of_king].usage==piece_is_unused)
    white_king(dir_from_rider,go_on);

  if (intelligent_reserve_masses(White,1))
  {
    white_piece(dir_from_rider,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
