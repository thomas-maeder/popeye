#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Intercept with a white piece on some square
 * @param where_to_intercept where to intercept
 * @param go_on what to do after each successful interception?
 */
static void black_piece_on(slice_index si,
                           boolean is_check,
                           square where_to_intercept,
                           void (*go_on)(slice_index si))
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_check);
  TraceSquare(where_to_intercept);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      black[i].usage = piece_intercepts;
      if (is_check)
        intelligent_place_pinned_black_piece(si,i,where_to_intercept,go_on);
      else
        intelligent_place_black_piece(si,i,where_to_intercept,go_on);
      black[i].usage = piece_is_unused;
    }

  empty_square(where_to_intercept);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a black piece
 * @param guard of what square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void black_piece(slice_index si,
                        square target,
                        int dir_from_rider,
                        void (*go_on)(slice_index si))
{
  Flags const mask = BIT(Black)|BIT(Royal);
  boolean const is_check = TSTFULLFLAGMASK(being_solved.spec[target+dir_from_rider],mask);
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = target-dir_from_rider;
       is_square_empty(where_to_intercept);
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0
      /* avoid testing the same position twice */
        && *where_to_start_placing_black_pieces<=where_to_intercept)
      black_piece_on(si,is_check,where_to_intercept,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a promoted white pawn
 * @param intercepter_index identifies intercepting pawn
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void promoted_white_pawn(slice_index si,
                                unsigned int intercepter_index,
                                square where_to_intercept,
                                boolean is_diagonal,
                                void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",intercepter_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(intercepter_index,
                                                                where_to_intercept))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
          break;

        case Rook:
          if (is_diagonal)
            intelligent_place_promoted_white_rider(si,
                                                   Rook,
                                                   intercepter_index,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case Bishop:
          if (!is_diagonal)
            intelligent_place_promoted_white_rider(si,
                                                   Bishop,
                                                   intercepter_index,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case Knight:
          intelligent_place_promoted_white_knight(si,
                                                  intercepter_index,
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

/* Intercept with a white piece on a square
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void white_piece_on(slice_index si,
                           square where_to_intercept,
                           boolean is_diagonal,
                           void (*go_on)(slice_index si))
{
  unsigned int intercepter_index;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  for (intercepter_index = 0; intercepter_index<MaxPiece[White]; ++intercepter_index)
    if (white[intercepter_index].usage==piece_is_unused)
    {
      white[intercepter_index].usage = piece_intercepts;

      switch (white[intercepter_index].type)
      {
        case King:
          intelligent_place_white_king(si,
                                       where_to_intercept,
                                       go_on);
          break;

        case Queen:
          break;

        case Rook:
          if (is_diagonal)
            intelligent_place_white_rider(si,
                                          intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case Bishop:
          if (!is_diagonal)
            intelligent_place_white_rider(si,
                                          intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case Knight:
          intelligent_place_white_knight(si,
                                         intercepter_index,
                                         where_to_intercept,
                                         go_on);
          break;

        case Pawn:
          promoted_white_pawn(si,
                              intercepter_index,
                              where_to_intercept,
                              is_diagonal,go_on);
          intelligent_place_unpromoted_white_pawn(si,
                                                  intercepter_index,
                                                  where_to_intercept,
                                                  go_on);
          break;

        case Dummy:
          break;

        default:
          assert(0);
          break;
      }

      white[intercepter_index].usage = piece_is_unused;
    }

  empty_square(where_to_intercept);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept with a white piece
 * @param target guard of what square
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
static void white_piece(slice_index si,
                        square target,
                        int dir_from_rider,
                        void (*go_on)(slice_index si))
{
  square const start = target-dir_from_rider;
  boolean const is_diagonal = SquareCol(start)==SquareCol(target);
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = start;
       is_square_empty(where_to_intercept);
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      white_piece_on(si,where_to_intercept,is_diagonal,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a guard by white
 * @param target guard of what square
 * @param dir_from_rider direction from guarding rider
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_guard_by_white(slice_index si,
                                          square target,
                                          int dir_from_rider,
                                          void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1,piece_intercepts))
  {
    black_piece(si,target,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  if (intelligent_reserve_masses(White,1,piece_intercepts))
  {
    white_piece(si,target,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
