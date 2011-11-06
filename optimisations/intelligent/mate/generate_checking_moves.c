#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

void remember_to_keep_checking_line_open(square from, square to,
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
    case Rook:
    case Queen:
      remember_to_keep_rider_line_open(from,to,CheckDir[type][diff],delta);
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

static void by_promoted_pawn(unsigned int index_of_checker)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide
        && intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_checker,
                                                                     *bnp))
    {
      square const pawn_origin = white[index_of_checker].diagram_square;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (intelligent_reserve_promoting_white_pawn_moves_from_to(pawn_origin,
                                                                   pp,
                                                                   *bnp))
        {
          if (officer_guards(king_square[Black],pp,*bnp))
          {
            SetPiece(pp,*bnp,checker_flags);
            remember_to_keep_checking_line_open(*bnp,king_square[Black],pp,+1);
            intelligent_guard_flights();
            remember_to_keep_checking_line_open(*bnp,king_square[Black],pp,-1);
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

static void by_unpromoted_pawn(unsigned int index_of_checker)
{
  square const checker_from = white[index_of_checker].diagram_square;
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);TracePiece(e[*bnp]);TraceText("\n");
    if (*bnp>=square_a2 && *bnp<=square_h7 && e[*bnp]==vide
        && GuardDir[Pawn-Pawn][*bnp].dir==guard_dir_check_uninterceptable
        && intelligent_reserve_white_pawn_moves_from_to_checking(checker_from,*bnp))
    {
      SetPiece(pb,*bnp,checker_flags);
      intelligent_guard_flights();
      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_officer(unsigned int index_of_checker, piece checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TracePiece(checker_type);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);TracePiece(e[*bnp]);TraceText("\n");
    if (e[*bnp]==vide)
    {
      if (intelligent_reserve_white_officer_moves_from_to_checking(white[index_of_checker].diagram_square,
                                                                   checker_type,
                                                                   *bnp))
      {
        if (officer_guards(king_square[Black],checker_type,*bnp))
        {
          SetPiece(checker_type,*bnp,checker_flags);
          remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,+1);
          intelligent_guard_flights();
          remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,-1);
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

void intelligent_mate_generate_checking_moves(void)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    for (index = 1; index<MaxPiece[White]; ++index)
    {
      piece const checker_type = white[index].type;

      TraceValue("%u",index);
      TraceSquare(white[index].diagram_square);
      TracePiece(checker_type);
      TraceText("\n");

      white[index].usage = piece_gives_check;

      if (checker_type==pb)
      {
        by_unpromoted_pawn(index);
        by_promoted_pawn(index);
      }
      else
        by_officer(index,checker_type);

      white[index].usage = piece_is_unused;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
