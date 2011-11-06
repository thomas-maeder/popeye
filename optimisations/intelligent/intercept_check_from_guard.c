#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Does a white officer guard any flight?
 * @param officer_type type of officer
 * @param from where might the officer guard from?
 * @return a flight square guarded by the officer; initsquare if it doesn't guard
 */
static square white_officer_guards_flight(piece officer_type, square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TracePiece(officer_type);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && officer_guards(king_square[Black]+vec[i],officer_type,from))
    {
      result = king_square[Black]+vec[i];
      break;
    }

  e[king_square[Black]]= roin;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Place a white officer to intercept a check to the black king
 * @param index_of_guarding_rider identifies the checking rider
 * @param officer_type type of officer
 * @param to_be_intercepted where to intercept
 * @param index_of_intercepting_piece identifies the pawn
 */
static void place_officer(unsigned int index_of_guarding_rider,
                          piece officer_type,
                          square to_be_intercepted,
                          unsigned int index_of_intercepting_piece)
{
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TracePiece(officer_type);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (/* avoid duplicate: if intercepter has already been used as guarding
       * piece, it shouldn't guard now again */
      !(index_of_intercepting_piece<index_of_guarding_rider
        && white_officer_guards_flight(officer_type,to_be_intercepted)))
  {
    SetPiece(officer_type,to_be_intercepted,intercepter_flags);
    intelligent_continue_guarding_flights();
    e[to_be_intercepted] = vide;
    spec[to_be_intercepted] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place a white promotee to intercept a check to the black king
 * @param index_of_guarding_rider identifies the checking rider
 * @param promotee_type type of promotee
 * @param to_be_intercepted where to intercept
 * @param index_of_intercepting_piece identifies the pawn
 */
static void place_promotee(unsigned int index_of_guarding_rider,
                           piece promotee_type,
                           square to_be_intercepted,
                           unsigned int index_of_intercepting_piece)
{
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TracePiece(promotee_type);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(intercepter_diagram_square,
                                                             promotee_type,
                                                             to_be_intercepted))
  {
    place_officer(index_of_guarding_rider,
                  promotee_type,
                  to_be_intercepted,
                  index_of_intercepting_piece);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check by a white rider white an promoted pawn
 * @param index_of_guarding_rider identifies the rider
 * @param to_be_intercepted where to intercept
 * @param index_of_intercepting_piece identifies the pawn
 * @param is_diagonal true iff the check is on a diagonal
 */
static void promoted_pawn(unsigned int index_of_guarding_rider,
                          square to_be_intercepted,
                          unsigned int index_of_intercepting_piece,
                          boolean is_diagonal)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceValue("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_intercepting_piece,
                                                                to_be_intercepted))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      switch (pp)
      {
        case db:
          break;

        case tb:
          if (is_diagonal)
            place_promotee(index_of_guarding_rider,
                           tb,
                           to_be_intercepted,
                           index_of_intercepting_piece);
          break;

        case fb:
          if (!is_diagonal)
            place_promotee(index_of_guarding_rider,
                           fb,
                           to_be_intercepted,
                           index_of_intercepting_piece);
          break;

        case cb:
          place_promotee(index_of_guarding_rider,
                         cb,
                         to_be_intercepted,
                         index_of_intercepting_piece);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check by a white rider white an unpromoted pawn
 * @param index_of_guarding_rider identifies the rider
 * @param to_be_intercepted where to intercept
 * @param index_of_intercepting_piece identifies the pawn
 */
static void unpromoted_pawn(unsigned int index_of_guarding_rider,
                            square to_be_intercepted,
                            unsigned int index_of_intercepting_piece)
{
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;
  numvec const guard_dir = GuardDir[Pawn-Pawn][to_be_intercepted].dir;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (guard_dir!=guard_dir_check_uninterceptable
      /* avoid duplicate: if intercepter has already been used as guarding
       * piece, it shouldn't guard now again */
      && !(index_of_intercepting_piece<index_of_guarding_rider
           && guard_dir==guard_dir_guard_uninterceptable)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(intercepter_diagram_square,
                                                                   to_be_intercepted))
  {
    SetPiece(pb,to_be_intercepted,intercepter_flags);
    intelligent_continue_guarding_flights();
    e[to_be_intercepted] = vide;
    spec[to_be_intercepted] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check by a white rider white a white officer
 * @param index_of_guarding_rider identifies the rider
 * @param to_be_intercepted where to intercept
 * @param index_of_intercepting_piece identifies the intercepting officer
 */
static void officer(unsigned int index_of_guarding_rider,
                    square to_be_intercepted,
                    unsigned int index_of_intercepting_piece)
{
  square const officer_diagram_square = white[index_of_intercepting_piece].diagram_square;
  piece const officer_type = white[index_of_intercepting_piece].type;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(officer_diagram_square,
                                                officer_type,
                                                to_be_intercepted))
  {
    place_officer(index_of_guarding_rider,
                  officer_type,
                  to_be_intercepted,
                  index_of_intercepting_piece);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a check by a white rider on the flight guarded by the rider
 * @param index_of_guarding_rider identifies the rider
 * @param to_be_intercepted where to intercept
 */
void intercept_check_on_guarded_square(unsigned int index_of_guarding_rider,
                                       square to_be_intercepted)
{
  unsigned int intercepter_index;
  boolean const is_diagonal = SquareCol(to_be_intercepted)==SquareCol(king_square[Black]);

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_guarding_rider);
  TraceSquare(to_be_intercepted);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    for (intercepter_index = 1;
         intercepter_index<MaxPiece[White];
         ++intercepter_index)
    {
      TraceValue("%u",intercepter_index);
      TraceEnumerator(piece_usage,white[intercepter_index].usage,"\n");
      if (white[intercepter_index].usage==piece_is_unused)
      {
        piece const intercepter_type = white[intercepter_index].type;
        white[intercepter_index].usage = piece_intercepts;

        switch (intercepter_type)
        {
          case db:
            break;

          case tb:
            if (is_diagonal)
              officer(index_of_guarding_rider,
                      to_be_intercepted,
                      intercepter_index);
              break;

          case fb:
            if (!is_diagonal)
              officer(index_of_guarding_rider,
                      to_be_intercepted,
                      intercepter_index);
            break;

          case cb:
            officer(index_of_guarding_rider,
                    to_be_intercepted,
                    intercepter_index);
            break;

          case pb:
            if (to_be_intercepted>=square_a2 && to_be_intercepted<=square_h7)
              unpromoted_pawn(index_of_guarding_rider,
                              to_be_intercepted,
                              intercepter_index);
            promoted_pawn(index_of_guarding_rider,
                          to_be_intercepted,
                          intercepter_index,
                          is_diagonal);
            break;

          default:
            assert(0);
            break;
        }

        white[intercepter_index].usage = piece_is_unused;
      }
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept an orthogonal check with a pinned promoted black piece
 * @param placed_on where to place the pinned black piece
 */
static void place_promoted_black_pawn(square placed_on,
                                      unsigned int placed_index)
{
  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",placed_index);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      switch (pp)
      {
        case dn:
        case tn:
          break;

        case fn:
          intelligent_place_promoted_black_rider(placed_index,
                                                 fn,
                                                 placed_on,
                                                 &intelligent_continue_guarding_flights);
          break;

        case cn:
          intelligent_place_promoted_black_knight(placed_index,
                                                  placed_on,
                                                  &intelligent_continue_guarding_flights);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept an orthogonal check with a pinned black piece
 * @param placed_on where to place the pinned black piece
 */
void intelligent_intercept_orthogonal_check_by_pin(square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    unsigned int placed_index;
    for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
      if (black[placed_index].usage==piece_is_unused)
      {
        black[placed_index].usage = piece_intercepts;

        switch (black[placed_index].type)
        {
          case dn:
          case tn:
            break;

          case fn:
            intelligent_place_black_rider(placed_index,
                                          placed_on,
                                          &intelligent_continue_guarding_flights);
            break;

          case cn:
            intelligent_place_black_knight(placed_index,
                                           placed_on,
                                           &intelligent_continue_guarding_flights);
            break;

          case pn:
            intelligent_place_unpromoted_black_pawn(placed_index,
                                                    placed_on,
                                                    &intelligent_continue_guarding_flights);
            place_promoted_black_pawn(placed_on,placed_index);
            break;

          default:
            assert(0);
            break;
        }

        black[placed_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Where to intercept a check using a white piece
 * @param guard_type type of guarding white piece
 * @param guard_from where does the piece guard from?
 * @return square where to intercept
 */
square where_to_intercept_check_from_guard(piece guard_type, square guard_from)
{
  int const diff = king_square[Black]-guard_from;
  PieNam const guard_type_Nam = abs(guard_type);
	int const dir = CheckDir[guard_type_Nam][diff];
  square result;

  TraceFunctionEntry(__func__);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (rider_guards(king_square[Black],guard_from,dir))
    result = king_square[Black]-dir;
  else
    result = initsquare;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}
