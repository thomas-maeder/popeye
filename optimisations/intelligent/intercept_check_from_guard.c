#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static square guards_black_flight(piece as_piece, square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TracePiece(as_piece);
  TraceSquare(from);
  TraceSquare(king_square[Black]);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && guards(king_square[Black]+vec[i],as_piece,from))
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

  if (!uninterceptably_attacks_king(Black,to_be_intercepted,intercepter_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(intercepter_type,
                                                                             intercepter_diagram_square,
                                                                             intercepter_type,
                                                                             to_be_intercepted);
    if (time<=Nr_remaining_white_moves
        /* avoid duplicate: if intercepter has already been used as guarding
         * piece, it shouldn't guard now again */
        && !(index_of_intercepting_piece<index_of_next_guarding_piece
             && guards_black_flight(intercepter_type,to_be_intercepted)))
    {
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(intercepter_type,to_be_intercepted,intercepter_flags);
      intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
      e[to_be_intercepted] = vide;
      spec[to_be_intercepted] = EmptySpec;
      Nr_remaining_white_moves += time;
    }
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

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int const min_nr_moves_by_p = (to_be_intercepted<=square_h7
                                            ? moves_to_white_prom[index_of_intercepting_piece]+1
                                            : moves_to_white_prom[index_of_intercepting_piece]);
    if (Nr_remaining_white_moves>=min_nr_moves_by_p)
    {
      square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
      Flags const intercepter_flags = white[index_of_intercepting_piece].flags;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!uninterceptably_attacks_king(Black,to_be_intercepted,pp))
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(intercepter_diagram_square,
                                                                                         pp,
                                                                                         to_be_intercepted);
          if (time<=Nr_remaining_white_moves
              && !(index_of_intercepting_piece<index_of_next_guarding_piece
                   && guards_black_flight(pp,to_be_intercepted)))
          {
            Nr_remaining_white_moves -= time;
            TraceValue("%u\n",Nr_remaining_white_moves);
            SetPiece(pp,to_be_intercepted,intercepter_flags);
            intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
            e[to_be_intercepted] = vide;
            spec[to_be_intercepted] = EmptySpec;
            Nr_remaining_white_moves += time;
          }
        }
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

  if (!uninterceptably_attacks_king(Black,to_be_intercepted,pb))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                                      intercepter_diagram_square,
                                                                                      to_be_intercepted);
    if (time<=Nr_remaining_white_moves)
    {
      unsigned int const diffcol = abs(intercepter_diagram_square % onerow
                                       - to_be_intercepted % onerow);
      if (diffcol<=Max_nr_allowed_captures_by_white)
      {
        Max_nr_allowed_captures_by_white -= diffcol;
        Nr_remaining_white_moves -= time;
        TraceValue("%u",Max_nr_allowed_captures_by_white);
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(pb,to_be_intercepted,intercepter_flags);
        intelligent_continue_guarding_flights(n,index_of_next_guarding_piece);
        e[to_be_intercepted] = vide;
        spec[to_be_intercepted] = EmptySpec;
        Nr_remaining_white_moves += time;
        Max_nr_allowed_captures_by_white += diffcol;
      }
    }
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
