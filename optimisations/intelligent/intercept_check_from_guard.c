#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static square white_pawn_guards_flight(square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && white_pawn_attacks_king_region(from,vec[i]))
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

static void place_officer(stip_length_type n,
                          unsigned int index_of_next_guarding_piece,
                          piece officer_type,
                          square to_be_intercepted,
                          unsigned int index_of_intercepting_piece)
{
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TracePiece(officer_type);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (/* avoid duplicate: if intercepter has already been used as guarding
       * piece, it shouldn't guard now again */
      !(index_of_intercepting_piece<index_of_next_guarding_piece
        && white_officer_guards_flight(officer_type,to_be_intercepted)))
  {
    SetPiece(officer_type,to_be_intercepted,intercepter_flags);
    intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
    e[to_be_intercepted] = vide;
    spec[to_be_intercepted] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_promotee(stip_length_type n,
                           unsigned int index_of_next_guarding_piece,
                           piece promotee_type,
                           square to_be_intercepted,
                           unsigned int index_of_intercepting_piece)
{
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TracePiece(promotee_type);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_promoting_white_pawn_moves_from_to(intercepter_diagram_square,
                                                             promotee_type,
                                                             to_be_intercepted))
  {
    place_officer(n,
                  index_of_next_guarding_piece,
                  promotee_type,
                  to_be_intercepted,
                  index_of_intercepting_piece);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          unsigned int index_of_next_guarding_piece,
                          square to_be_intercepted,
                          unsigned int index_of_intercepting_piece,
                          boolean is_diagonal)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
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
            place_promotee(n,
                           index_of_next_guarding_piece,
                           tb,
                           to_be_intercepted,
                           index_of_intercepting_piece);
          break;

        case fb:
          if (!is_diagonal)
            place_promotee(n,
                           index_of_next_guarding_piece,
                           fb,
                           to_be_intercepted,
                           index_of_intercepting_piece);
          break;

        case cb:
          place_promotee(n,
                         index_of_next_guarding_piece,
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

static void unpromoted_pawn(stip_length_type n,
                            unsigned int index_of_next_guarding_piece,
                            square to_be_intercepted,
                            unsigned int index_of_intercepting_piece)
{
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king_region(to_be_intercepted,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(intercepter_diagram_square,
                                                                   to_be_intercepted))
  {
    if (/* avoid duplicate: if intercepter has already been used as guarding
         * piece, it shouldn't guard now again */
        !(index_of_intercepting_piece<index_of_next_guarding_piece
          && white_pawn_guards_flight(to_be_intercepted)))
    {
      SetPiece(pb,to_be_intercepted,intercepter_flags);
      intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
      e[to_be_intercepted] = vide;
      spec[to_be_intercepted] = EmptySpec;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    unsigned int index_of_next_guarding_piece,
                    square to_be_intercepted,
                    unsigned int index_of_intercepting_piece)
{
  square const officer_diagram_square = white[index_of_intercepting_piece].diagram_square;
  piece const officer_type = white[index_of_intercepting_piece].type;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(officer_diagram_square,
                                                to_be_intercepted,
                                                officer_type))
  {
    place_officer(n,
                  index_of_next_guarding_piece,
                  officer_type,
                  to_be_intercepted,
                  index_of_intercepting_piece);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intercept_check_on_guarded_square(stip_length_type n,
                                       unsigned int index_of_next_guarding_piece,
                                       square to_be_intercepted)
{
  unsigned int intercepter_index;
  boolean const is_diagonal = SquareCol(to_be_intercepted)==SquareCol(king_square[Black]);

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
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
              officer(n,
                      index_of_next_guarding_piece,
                      to_be_intercepted,
                      intercepter_index);
              break;

          case fb:
            if (!is_diagonal)
              officer(n,
                      index_of_next_guarding_piece,
                      to_be_intercepted,
                      intercepter_index);
            break;

          case cb:
            officer(n,
                    index_of_next_guarding_piece,
                    to_be_intercepted,
                    intercepter_index);
            break;

          case pb:
            if (to_be_intercepted>=square_a2 && to_be_intercepted<=square_h7)
              unpromoted_pawn(n,
                              index_of_next_guarding_piece,
                              to_be_intercepted,
                              intercepter_index);
            promoted_pawn(n,
                          index_of_next_guarding_piece,
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

square where_to_intercept_check_from_guard(piece guard_type, square guard_from)
{
  int const diff = king_square[Black]-guard_from;
  square result = initsquare;
  PieNam const guard_type_Nam = abs(guard_type);;
	int const dir = CheckDir[guard_type_Nam][diff];

  TraceFunctionEntry(__func__);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (guard_type_Nam)
  {
    case Queen:
    {
      /* don't intercept wQc8 guarding the flight b7 (but not b8!) of bKa8 */
      if (diff!=2*dir && rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    case Rook:
    case Bishop:
    {
      if (rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    default:
      assert(0);
      break;
  }


  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}
