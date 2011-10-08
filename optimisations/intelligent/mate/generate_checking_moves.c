#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void generated(unsigned int const min_nr_captures_by_white,
                      unsigned int nr_checking_moves,
                      unsigned int prev_index,
                      stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",nr_checking_moves);
  TraceFunctionParam("%u",prev_index);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nr_checking_moves>=1);

  if (nr_checking_moves==1)
    intelligent_guard_flights(n,min_nr_captures_by_white);
  else
    intelligent_mate_generate_checking_moves(nr_checking_moves-1,prev_index,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_to_keep_checking_line_open(square from, square to,
                                                piece type, int delta)
{
  int const diff = to-from;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  assert(type>vide);

  switch (type)
  {
    case Bishop:
      remember_to_keep_rider_line_open(from,to,CheckDirBishop[diff],delta);
      break;

    case Rook:
      remember_to_keep_rider_line_open(from,to,CheckDirRook[diff],delta);
      break;

    case Queen:
      remember_to_keep_rider_line_open(from,to,CheckDirQueen[diff],delta);
      break;

    case King:
    case Knight:
    case Pawn:
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_pawn(unsigned int nr_checking_moves,
                             unsigned int index_of_checker,
                             stip_length_type n)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_checking_moves);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_white_prom[index_of_checker]+1
                                              : moves_to_white_prom[index_of_checker]);
      if (Nr_remaining_white_moves>=min_nr_moves_by_p)
      {
        piece pp;
        square const pawn_origin = white[index_of_checker].diagram_square;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
          if (!(nr_checking_moves==2
                && uninterceptably_attacks_king(White,*bnp,pp)))
          {
            unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(pawn_origin,
                                                                                           pp,
                                                                                           *bnp);
            if (time<=Nr_remaining_white_moves
                && guards(king_square[Black],pp,*bnp))
            {
              unsigned int const min_nr_captures_by_white = 0;
              Nr_remaining_white_moves -= time;
              TraceValue("%u\n",Nr_remaining_white_moves);
              SetPiece(pp,*bnp,checker_flags);
              remember_to_keep_checking_line_open(*bnp,king_square[Black],pp,+1);
              generated(min_nr_captures_by_white,
                        nr_checking_moves,
                        index_of_checker,
                        n);
              remember_to_keep_checking_line_open(*bnp,king_square[Black],pp,-1);
              Nr_remaining_white_moves += time;
            }
          }
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_unpromoted_pawn(unsigned int nr_checking_moves,
                               unsigned int index_of_checker,
                               stip_length_type n)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_checking_moves);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide)
    {
      square const pawn_origin = white[index_of_checker].diagram_square;
      unsigned int const time = intelligent_count_nr_of_moves_from_to_checking(pb,
                                                                               pawn_origin,
                                                                               pb,
                                                                               *bnp);
      if (time<=Nr_remaining_white_moves
          && guards(king_square[Black],pb,*bnp))
      {
        unsigned int const min_nr_captures_by_white = abs(pawn_origin%onerow - *bnp%onerow);
        Nr_remaining_white_moves -= time;
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(pb,*bnp,checker_flags);
        generated(min_nr_captures_by_white,
                  nr_checking_moves,
                  index_of_checker,
                  n);
        Nr_remaining_white_moves += time;
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_officer(unsigned int nr_checking_moves,
                       unsigned int index_of_checker,
                       stip_length_type n,
                       piece checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_checking_moves);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TracePiece(checker_type);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide
        && !(nr_checking_moves==2
             && uninterceptably_attacks_king(White,*bnp,checker_type)))
    {
      square const checker_origin = white[index_of_checker].diagram_square;
      unsigned int const time = intelligent_count_nr_of_moves_from_to_checking(checker_type,
                                                                               checker_origin,
                                                                               checker_type,
                                                                               *bnp);
      if (time<=Nr_remaining_white_moves
          && guards(king_square[Black],checker_type,*bnp))
      {
        unsigned int const min_nr_captures_by_white = 0;
        Nr_remaining_white_moves -= time;
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(checker_type,*bnp,checker_flags);
        remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,+1);
        generated(min_nr_captures_by_white,
                  nr_checking_moves,
                  index_of_checker,
                  n);
        remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,-1);
        Nr_remaining_white_moves += time;
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_checking_moves(unsigned int nr_checking_moves,
                                              unsigned int prev_index,
                                              stip_length_type n)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_checking_moves);
  TraceFunctionParam("%u",prev_index);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (index = prev_index+1; index<MaxPiece[White]; ++index)
  {
    piece const checker_type = white[index].type;

    TraceValue("%u",index);
    TraceSquare(white[index].diagram_square);
    TracePiece(checker_type);
    TraceText("\n");

    white[index].usage = piece_gives_check;

    if (checker_type==pb)
    {
      by_unpromoted_pawn(nr_checking_moves,index,n);
      by_promoted_pawn(nr_checking_moves,index,n);
    }
    else
      by_officer(nr_checking_moves,index,n,checker_type);

    white[index].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
