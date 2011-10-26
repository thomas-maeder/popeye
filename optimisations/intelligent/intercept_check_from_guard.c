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

static void intercept_check_on_guarded_square_officer(stip_length_type n,
                                                      unsigned int index_of_next_guarding_piece,
                                                      square to_be_intercepted,
                                                      unsigned int index_of_intercepting_piece)
{
  piece const intercepter_type = white[index_of_intercepting_piece].type;
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,to_be_intercepted,intercepter_type)
      && intelligent_reserve_officer_moves_from_to(intercepter_diagram_square,
                                                   to_be_intercepted,
                                                   intercepter_type))
  {
    if (/* avoid duplicate: if intercepter has already been used as guarding
         * piece, it shouldn't guard now again */
        !(index_of_intercepting_piece<index_of_next_guarding_piece
          && white_officer_guards_flight(intercepter_type,to_be_intercepted)))
    {
      SetPiece(intercepter_type,to_be_intercepted,intercepter_flags);
      intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
      e[to_be_intercepted] = vide;
      spec[to_be_intercepted] = EmptySpec;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square_promoted_pawn(stip_length_type n,
                                                            unsigned int index_of_next_guarding_piece,
                                                            square to_be_intercepted,
                                                            unsigned int index_of_intercepting_piece)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_intercepting_piece,
                                                                to_be_intercepted))
  {
    square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
    Flags const intercepter_flags = white[index_of_intercepting_piece].flags;
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,to_be_intercepted,pp)
          && intelligent_reserve_promoting_white_pawn_moves_from_to(intercepter_diagram_square,
                                                                    pp,
                                                                    to_be_intercepted))
      {
        if (/* avoid duplicate: if intercepter has already been used as guarding
             * piece, it shouldn't guard now again */
            !(index_of_intercepting_piece<index_of_next_guarding_piece
              && white_officer_guards_flight(pp,to_be_intercepted)))
        {
          SetPiece(pp,to_be_intercepted,intercepter_flags);
          intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
          e[to_be_intercepted] = vide;
          spec[to_be_intercepted] = EmptySpec;
        }

        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square_unpromoted_pawn(stip_length_type n,
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

void intercept_check_on_guarded_square(stip_length_type n,
                                       unsigned int index_of_next_guarding_piece,
                                       square to_be_intercepted)
{
  unsigned int index_of_intercepting_piece;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    for (index_of_intercepting_piece = 1;
         index_of_intercepting_piece<MaxPiece[White];
         ++index_of_intercepting_piece)
    {
      TraceValue("%u",index_of_intercepting_piece);
      TraceEnumerator(piece_usage,white[index_of_intercepting_piece].usage,"\n");
      if (white[index_of_intercepting_piece].usage==piece_is_unused)
      {
        piece const guard_type = white[index_of_intercepting_piece].type;
        white[index_of_intercepting_piece].usage = piece_intercepts;

        switch (guard_type)
        {
          case db:
            break;

          case tb:
          case fb:
          case cb:
            intercept_check_on_guarded_square_officer(n,
                                                      index_of_next_guarding_piece,
                                                      to_be_intercepted,
                                                      index_of_intercepting_piece);
            break;

          case pb:
            if (to_be_intercepted>=square_a2 && to_be_intercepted<=square_h7)
              intercept_check_on_guarded_square_unpromoted_pawn(n,
                                                                index_of_next_guarding_piece,
                                                                to_be_intercepted,
                                                                index_of_intercepting_piece);
            intercept_check_on_guarded_square_promoted_pawn(n,
                                                            index_of_next_guarding_piece,
                                                            to_be_intercepted,
                                                            index_of_intercepting_piece);
            break;

          default:
            assert(0);
            break;
        }

        white[index_of_intercepting_piece].usage = piece_is_unused;
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

  TraceFunctionEntry(__func__);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (guard_type)
  {
    case db:
    {
      int const dir = CheckDirQueen[diff];
      /* don't intercept wQc8 guarding the flight b7 (but not b8!) of bKa8 */
      if (diff!=2*dir && rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    case tb:
    {
      int const dir = CheckDirRook[diff];
      if (rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    case fb:
    {
      int const dir = CheckDirBishop[diff];
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
