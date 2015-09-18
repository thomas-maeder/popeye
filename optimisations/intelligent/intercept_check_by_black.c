#include "optimisations/intelligent/intercept_check_by_black.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Intercept a check to the white king with a white piece on some square
 * @param where_to_intercept where to intercept
 * @param go_on what to do after each successful interception?
 */
static void white_piece_on(slice_index si,
                           square where_to_intercept,
                           void (*go_on)(slice_index si))
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; ++i)
    if (white[i].usage==piece_is_unused)
    {
      white[i].usage = piece_intercepts;
      intelligent_place_white_piece(si,i,where_to_intercept,go_on);
      white[i].usage = piece_is_unused;
    }

  empty_square(where_to_intercept);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king with a white piece
 * @param dir_from_rider direction from rider giving check to white king
 * @param go_on what to do after each successful interception?
 */
static void white_piece(slice_index si, int dir_from_rider, void (*go_on)(slice_index si))
{
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = being_solved.king_square[White]-dir_from_rider;
       is_square_empty(where_to_intercept);
       where_to_intercept -= dir_from_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
      white_piece_on(si,where_to_intercept,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king with a promoted black pawn
 * @param intercepter_index identifies intercepting pawn
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void promoted_black_pawn(slice_index si,
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

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(intercepter_index,
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
            intelligent_place_promoted_black_rider(si,
                                                   intercepter_index,
                                                   Rook,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case Bishop:
          if (!is_diagonal)
            intelligent_place_promoted_black_rider(si,
                                                   intercepter_index,
                                                   Bishop,
                                                   where_to_intercept,
                                                   go_on);
          break;

        case Knight:
          intelligent_place_promoted_black_knight(si,
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

/* Intercept a check to the white king with a black piece on a square
 * @param where_to_intercept what square
 * @param is_diagonal true iff we are intercepting a check on a diagonal line
 * @param go_on what to do after each successful interception?
 */
static void black_piece_on(slice_index si,
                           square where_to_intercept,
                           boolean is_diagonal,
                           void (*go_on)(slice_index si))
{
  unsigned int intercepter_index;

  TraceFunctionEntry(__func__);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  for (intercepter_index = 1; intercepter_index<MaxPiece[Black]; ++intercepter_index)
    if (black[intercepter_index].usage==piece_is_unused)
    {
      black[intercepter_index].usage = piece_intercepts;

      switch (black[intercepter_index].type)
      {
        case Pawn:
          promoted_black_pawn(si,
                              intercepter_index,where_to_intercept,
                              is_diagonal,
                              go_on);
          if (!TSTFLAGMASK(sq_spec[where_to_intercept],BIT(BlBaseSq)|BIT(BlPromSq)))
            intelligent_place_unpromoted_black_pawn(si,
                                                    intercepter_index,
                                                    where_to_intercept,
                                                    go_on);
          break;

        case Queen:
          break;

        case Rook:
          if (is_diagonal)
            intelligent_place_black_rider(si,
                                          intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case Bishop:
          if (!is_diagonal)
            intelligent_place_black_rider(si,
                                          intercepter_index,
                                          where_to_intercept,
                                          go_on);
          break;

        case Knight:
          intelligent_place_black_knight(si,
                                         intercepter_index,
                                         where_to_intercept,
                                         go_on);
          break;

        default:
          assert(0);
          break;
      }

      black[intercepter_index].usage = piece_is_unused;
    }

  empty_square(where_to_intercept);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king with a black piece
 * @param dir_from_rider direction from rider giving check to white king
 * @param go_on what to do after each successful interception?
 */
static void black_piece(slice_index si,
                        int dir_to_rider,
                        void (*go_on)(slice_index si))
{
  square const start = being_solved.king_square[White]-dir_to_rider;
  boolean const is_diagonal = SquareCol(being_solved.king_square[White])==SquareCol(start);
  square where_to_intercept;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_to_rider);
  TraceFunctionParamListEnd();

  for (where_to_intercept = start;
       is_square_empty(where_to_intercept);
       where_to_intercept -= dir_to_rider)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0
        && *where_to_start_placing_black_pieces<=where_to_intercept)
      black_piece_on(si,where_to_intercept,is_diagonal,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check to the white king
 * @param dir_from_rider direction from rider giving check to white king
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_check_by_black(slice_index si,
                                          int dir_from_rider,
                                          void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_from_rider);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1,piece_intercepts))
  {
    black_piece(si,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  if (intelligent_reserve_masses(White,1,piece_intercepts))
  {
    white_piece(si,dir_from_rider,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
