#include "optimisations/intelligent/guard_flights.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/block_flights.h"
#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "options/maxsolutions/maxsolutions.h"
#include "platform/maxtime.h"
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

static boolean white_king_guards_flight(square from)
{
  int const diff = move_diff_code[abs(king_square[Black]-from)];
  boolean const result = diff>3 && diff<=8;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

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

static void remember_to_keep_rider_guard_line_open(square from, square to,
                                                   piece type, int delta)
{
  int const dir = CheckDir[Queen][to-from];
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  TraceValue("%d\n",dir);

  /* the guard line only needs to be kept open up to the flight closest to
   * from; e.g. reset to to c1 with from:a1 to:e1 king_square[Black]:d2
   */
  for (s = to-dir; s!=from && move_diff_code[abs(king_square[Black]-s)]<=2; s -= dir)
    to = s;

  remember_to_keep_rider_line_open(from,to,dir,delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_to_keep_guard_line_open(square from, square to,
                                             piece type, int delta)
{
  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  switch (type)
  {
    case Queen:
      if (move_diff_code[abs(king_square[Black]-from)]>10)
        remember_to_keep_rider_guard_line_open(from,to,type,delta);
      else
      {
        /* queen may be guarding on two lines*/
      }
      break;

    case Rook:
      if (move_diff_code[abs(king_square[Black]-from)]>2)
        remember_to_keep_rider_guard_line_open(from,to,type,delta);
      else
      {
        /* rook may be guarding on two lines*/
      }
      break;

    case Bishop:
      remember_to_keep_rider_guard_line_open(from,to,type,delta);
      break;

    case Knight:
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fix_white_king_on_diagram_square(stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0
      && !would_white_king_guard_from(king_diagram_square))
  {
    king_square[White] = king_diagram_square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    intelligent_find_and_block_flights(n);

    white[index_of_king].usage = piece_is_unused;
    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void FinaliseGuarding(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused
      && white[index_of_king].diagram_square!=square_e1
      && intelligent_get_nr_remaining_moves(White)==0)
    fix_white_king_on_diagram_square(n);
  else
    intelligent_find_and_block_flights(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n, unsigned int index)
{
  Flags const pawn_flags = white[index].flags;
  square const starts_from = white[index].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (*bnp>=square_a2 && *bnp<=square_h7 && e[*bnp]==vide
        && nr_reasons_for_staying_empty[*bnp]==0
        && !white_pawn_attacks_king_region(*bnp,0)
        && intelligent_reserve_white_pawn_moves_from_to_no_promotion(starts_from,
                                                                     *bnp))
    {
      square const guarded = white_pawn_guards_flight(*bnp);
      if (guarded!=initsquare)
      {
        SetPiece(pb,*bnp,pawn_flags);
        intelligent_continue_guarding_flights(n,index+1);
        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }

      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider_from(stip_length_type n,
                       unsigned int index_of_rider,
                       piece guard_type,
                       square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  {
    square const guarded = white_officer_guards_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      square const to_be_intercepted = where_to_intercept_check_from_guard(guard_type,guard_from);
      SetPiece(guard_type,guard_from,white[index_of_rider].flags);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,+1);
      if (to_be_intercepted==initsquare)
        intelligent_continue_guarding_flights(n,index_of_rider+1);
      else
        intercept_check_on_guarded_square(n,index_of_rider+1,to_be_intercepted);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,-1);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void leaper_from(stip_length_type n,
                        unsigned int index_of_leaper,
                        piece guard_type,
                        square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  {
    square const guarded = white_officer_guards_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      SetPiece(guard_type,guard_from,white[index_of_leaper].flags);
      intelligent_continue_guarding_flights(n,index_of_leaper+1);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n, unsigned int index_of_pawn)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_pawn);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && nr_reasons_for_staying_empty[*bnp]==0
        && intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_pawn,
                                                                     *bnp))
    {
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!officer_uninterceptably_attacks_king(Black,*bnp,pp)
            && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_pawn].diagram_square,
                                                                      pp,
                                                                      *bnp))
        {
          switch (pp)
          {
            case db:
            case tb:
            case fb:
              rider_from(n,index_of_pawn,pp,*bnp);
              break;

            case cb:
              leaper_from(n,index_of_pawn,pp,*bnp);
              break;

            default:
              assert(0);
              break;
          }

          intelligent_unreserve();
        }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider(stip_length_type n,
                  unsigned int index_of_rider,
                  piece guard_type)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !officer_uninterceptably_attacks_king(Black,*bnp,guard_type)
        && intelligent_reserve_officer_moves_from_to(white[index_of_rider].diagram_square,
                                                     *bnp,
                                                     guard_type))
    {
      rider_from(n,index_of_rider,guard_type,*bnp);
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void leaper(stip_length_type n,
                   unsigned int index_of_leaper,
                   piece guard_type)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !officer_uninterceptably_attacks_king(Black,*bnp,guard_type)
        && intelligent_reserve_officer_moves_from_to(white[index_of_leaper].diagram_square,
                                                     *bnp,
                                                     guard_type))
    {
      leaper_from(n,index_of_leaper,guard_type,*bnp);
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_continue_guarding_flights(stip_length_type n,
                                           unsigned int index_of_next_guarding_piece)
{
  unsigned int index_of_current_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",index_of_next_guarding_piece);
  TraceFunctionParamListEnd();

  assert(index_of_next_guarding_piece>index_of_king);

  if (!max_nr_solutions_found_in_phase() && !hasMaxtimeElapsed())
  {
    if (intelligent_reserve_masses(White,1))
    {
      TraceValue("%u\n",MaxPiece[White]);
      for (index_of_current_guarding_piece = index_of_next_guarding_piece;
           index_of_current_guarding_piece<MaxPiece[White];
           ++index_of_current_guarding_piece)
      {
        TraceValue("%u",index_of_current_guarding_piece);
        TraceEnumerator(piece_usage,white[index_of_current_guarding_piece].usage,"\n");
        if (white[index_of_current_guarding_piece].usage==piece_is_unused)
        {
          piece const guard_type = white[index_of_current_guarding_piece].type;
          white[index_of_current_guarding_piece].usage = piece_guards;

          switch (guard_type)
          {
            case pb:
              unpromoted_pawn(n,index_of_current_guarding_piece);
              promoted_pawn(n,index_of_current_guarding_piece);
              break;

            case db:
            case tb:
            case fb:
              rider(n,index_of_current_guarding_piece,guard_type);
              break;

            case cb:
              leaper(n,index_of_current_guarding_piece,guard_type);
              break;

            default:
              assert(0);
              break;
          }

          white[index_of_current_guarding_piece].usage = piece_is_unused;
        }
      }

      intelligent_unreserve();
    }

    FinaliseGuarding(n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_guard_flights(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!max_nr_solutions_found_in_phase() && !hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused)
    {
      square const guard_from = white[index_of_king].diagram_square;
      square const *bnp;

      white[index_of_king].usage = piece_guards;

      /* try using white king for guarding from every square */
      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (e[*bnp]==vide
            && nr_reasons_for_staying_empty[*bnp]==0
            && white_king_guards_flight(*bnp)
            && intelligent_reserve_white_king_moves_from_to(guard_from,*bnp))
        {
          king_square[White]= *bnp;
          SetPiece(roib,*bnp,white[index_of_king].flags);
          intelligent_continue_guarding_flights(n,1);
          e[*bnp] = vide;
          spec[*bnp] = EmptySpec;
          intelligent_unreserve();
        }

      king_square[White] = initsquare;
      white[index_of_king].usage = piece_is_unused;
    }

    TraceText("try not using white king for guarding\n");
    intelligent_continue_guarding_flights(n,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
