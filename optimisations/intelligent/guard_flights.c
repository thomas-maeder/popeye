#include "optimisations/intelligent/guard_flights.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/block_flights.h"
#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "options/maxsolutions/maxsolutions.h"
#include "platform/maxtime.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Determine whether there would be king contact if the white king were placed
 * on a particular square
 * @param white_king_square square where white king would be placed
 * @return true iff there would be king contact if the white king were placed
 *              on white_king_square
 */
static boolean would_there_be_king_contact(square white_king_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_diff_code[abs(white_king_square-king_square[Black])]<3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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

static void remember_to_keep_rider_guard_line_open(square from, square to,
                                                   piece type, int delta)
{
  int const dir = CheckDirQueen[to-from];
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

static void fix_white_king_on_diagram_square(unsigned int nr_remaining_white_moves,
                                             unsigned int nr_remaining_black_moves,
                                             unsigned int min_nr_captures_by_white,
                                             stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0
      && !would_white_king_guard_from(king_diagram_square))
  {
    king_square[White] = king_diagram_square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    intelligent_block_flights(nr_remaining_white_moves,
                              nr_remaining_black_moves,
                              min_nr_captures_by_white,
                              n);

    white[index_of_king].usage = piece_is_unused;
    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void FinaliseGuarding(unsigned int nr_remaining_white_moves,
                             unsigned int nr_remaining_black_moves,
                             unsigned int min_nr_captures_by_white,
                             stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused
      && white[index_of_king].diagram_square!=square_e1
      && nr_remaining_white_moves==0)
    fix_white_king_on_diagram_square(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     min_nr_captures_by_white,
                                     n);
  else
    intelligent_block_flights(nr_remaining_white_moves,
                              nr_remaining_black_moves,
                              min_nr_captures_by_white,
                              n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(unsigned int nr_remaining_white_moves,
                                         unsigned int nr_remaining_black_moves,
                                         stip_length_type n,
                                         unsigned int index,
                                         unsigned int min_nr_captures_by_white)
{
  Flags const pawn_flags = white[index].flags;
  square const starts_from = white[index].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,pb))
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                            starts_from,
                                                                            *bnp);
      if (time<=nr_remaining_white_moves)
      {
        square const guarded = guards_black_flight(pb,*bnp);
        if (guarded!=initsquare)
        {
          unsigned int const diffcol = abs(starts_from % onerow - *bnp % onerow);
          SetPiece(pb,*bnp,pawn_flags);
          intelligent_continue_guarding_flights(nr_remaining_white_moves-time,
                                                nr_remaining_black_moves,
                                                n,
                                                index+1,
                                                min_nr_captures_by_white+diffcol);

          e[*bnp] = vide;
          spec[*bnp] = EmptySpec;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider_from(unsigned int nr_remaining_white_moves,
                       unsigned int nr_remaining_black_moves,
                       stip_length_type n,
                       unsigned int index_of_rider,
                       piece guard_type,
                       square guard_from,
                       unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  {
    square const guarded = guards_black_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      square const to_be_intercepted = where_to_intercept_check_from_guard(guard_type,guard_from);
      SetPiece(guard_type,guard_from,white[index_of_rider].flags);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,+1);
      if (to_be_intercepted==initsquare)
        intelligent_continue_guarding_flights(nr_remaining_white_moves,
                                              nr_remaining_black_moves,
                                              n,
                                              index_of_rider+1,
                                              min_nr_captures_by_white);
      else
        intercept_check_on_guarded_square(nr_remaining_white_moves,
                                          nr_remaining_black_moves,
                                          n,
                                          index_of_rider+1,
                                          to_be_intercepted,
                                          min_nr_captures_by_white);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,-1);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void leaper_from(unsigned int nr_remaining_white_moves,
                        unsigned int nr_remaining_black_moves,
                        stip_length_type n,
                        unsigned int index_of_leaper,
                        piece guard_type,
                        square guard_from,
                        unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  {
    square const guarded = guards_black_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      SetPiece(guard_type,guard_from,white[index_of_leaper].flags);
      intelligent_continue_guarding_flights(nr_remaining_white_moves,
                                            nr_remaining_black_moves,
                                            n,
                                            index_of_leaper+1,
                                            min_nr_captures_by_white);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(unsigned int nr_remaining_white_moves,
                          unsigned int nr_remaining_black_moves,
                          stip_length_type n,
                          unsigned int index_of_pawn,
                          unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_pawn);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && nr_reasons_for_staying_empty[*bnp]==0)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_white_prom[index_of_pawn]+1
                                              : moves_to_white_prom[index_of_pawn]);
      if (nr_remaining_white_moves>=min_nr_moves_by_p)
      {
        piece pp;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
          if (!uninterceptably_attacks_king(Black,*bnp,pp))
          {
            unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(white[index_of_pawn].diagram_square,
                                                                               pp,
                                                                               *bnp);
            if (time<=nr_remaining_white_moves)
              switch (pp)
              {
                case db:
                case tb:
                case fb:
                  rider_from(nr_remaining_white_moves-time,
                             nr_remaining_black_moves,
                             n,
                             index_of_pawn,
                             pp,
                             *bnp,
                             min_nr_captures_by_white);
                  break;

                case cb:
                  leaper_from(nr_remaining_white_moves-time,
                              nr_remaining_black_moves,
                              n,
                              index_of_pawn,
                              pp,
                              *bnp,
                              min_nr_captures_by_white);
                  break;

                default:
                  assert(0);
                  break;
              }
          }

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider(unsigned int nr_remaining_white_moves,
                  unsigned int nr_remaining_black_moves,
                  stip_length_type n,
                  unsigned int index_of_rider,
                  piece guard_type,
                  unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,guard_type))
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   white[index_of_rider].diagram_square,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves)
        rider_from(nr_remaining_white_moves-time,
                   nr_remaining_black_moves,
                   n,
                   index_of_rider,
                   guard_type,
                   *bnp,
                   min_nr_captures_by_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void leaper(unsigned int nr_remaining_white_moves,
                   unsigned int nr_remaining_black_moves,
                   stip_length_type n,
                   unsigned int index_of_leaper,
                   piece guard_type,
                   unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,guard_type))
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   white[index_of_leaper].diagram_square,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves)
        leaper_from(nr_remaining_white_moves-time,
                    nr_remaining_black_moves,
                    n,
                    index_of_leaper,
                    guard_type,
                    *bnp,
                    min_nr_captures_by_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_continue_guarding_flights(unsigned int nr_remaining_white_moves,
                                           unsigned int nr_remaining_black_moves,
                                           stip_length_type n,
                                           unsigned int index_of_next_guarding_piece,
                                           unsigned int min_nr_captures_by_white)
{
  unsigned int index_of_current_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",index_of_next_guarding_piece);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  assert(index_of_next_guarding_piece>index_of_king);

  if (!max_nr_solutions_found_in_phase()
      && min_nr_captures_by_white<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
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
            unpromoted_pawn(nr_remaining_white_moves,
                            nr_remaining_black_moves,
                            n,
                            index_of_current_guarding_piece,
                            min_nr_captures_by_white);
            promoted_pawn(nr_remaining_white_moves,
                          nr_remaining_black_moves,
                          n,
                          index_of_current_guarding_piece,
                          min_nr_captures_by_white);
            break;

          case db:
          case tb:
          case fb:
            rider(nr_remaining_white_moves,
                  nr_remaining_black_moves,
                  n,
                  index_of_current_guarding_piece,
                  guard_type,
                  min_nr_captures_by_white);
            break;

          case cb:
            leaper(nr_remaining_white_moves,
                   nr_remaining_black_moves,
                   n,
                   index_of_current_guarding_piece,
                   guard_type,
                   min_nr_captures_by_white);
            break;

          default:
            assert(0);
            break;
        }

        white[index_of_current_guarding_piece].usage = piece_is_unused;
      }
    }

    FinaliseGuarding(nr_remaining_white_moves,
                     nr_remaining_black_moves,
                     min_nr_captures_by_white,
                     n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_guard_flights(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               stip_length_type n,
                               unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  if (!max_nr_solutions_found_in_phase()
      && min_nr_captures_by_white<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
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
            && !would_there_be_king_contact(*bnp))
        {
          unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                                   guard_from,
                                                                   *bnp);
          TraceSquare(*bnp);TraceText("\n");
          if (time<=nr_remaining_white_moves)
          {
            square const guarded = guards_black_flight(roib,*bnp);
            if (guarded!=initsquare)
            {
              king_square[White]= *bnp;
              SetPiece(roib,*bnp,white[index_of_king].flags);
              intelligent_continue_guarding_flights(nr_remaining_white_moves-time,
                                                    nr_remaining_black_moves,
                                                    n,
                                                    1,
                                                    min_nr_captures_by_white);
              e[*bnp] = vide;
              spec[*bnp] = EmptySpec;
            }
          }
        }

      king_square[White] = initsquare;
      white[index_of_king].usage = piece_is_unused;
    }

    /* try not using white king for guarding */
    intelligent_continue_guarding_flights(nr_remaining_white_moves,
             nr_remaining_black_moves,
             n,
             1,
             min_nr_captures_by_white);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
