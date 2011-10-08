#include "optimisations/intelligent/mate/place_white_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void unpromoted_white_pawn(stip_length_type n,
                                  unsigned int placed_index,
                                  square placed_on)
{
  square const placed_from = white[placed_index].diagram_square;
  unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (diffcol<=Nr_unused_black_masses
      && !uninterceptably_attacks_king(Black,placed_on,pb))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                                      placed_from,
                                                                                      placed_on);
    if (time<=Nr_remaining_white_moves)
    {
      Nr_unused_black_masses -= diffcol;
      Nr_remaining_white_moves -= time;
      TraceValue("%u",Nr_unused_black_masses);
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(pb,placed_on,white[placed_index].flags);
      intelligent_mate_test_target_position(n);
      Nr_remaining_white_moves += time;
      Nr_unused_black_masses += diffcol;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_white_pawn(stip_length_type n,
                                unsigned int placed_index,
                                square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = moves_to_white_prom[placed_index];
    if (placed_on<=square_h7)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++time;

    if (time<=Nr_remaining_white_moves)
    {
      square const placed_from = white[placed_index].diagram_square;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!uninterceptably_attacks_king(Black,placed_on,pp))
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                                         pp,
                                                                                         placed_on);
          unsigned int diffcol;
          if (pp==fb && SquareCol(placed_on)==SquareCol(placed_from%onerow))
            diffcol= 1;
          else
            diffcol= 0;
          TracePiece(pp);
          TraceValue("%u",diffcol);
          TraceValue("%u\n",time);

          if (diffcol<=Nr_unused_black_masses
              && time<=Nr_remaining_white_moves)
          {
            Nr_unused_black_masses -= diffcol;
            Nr_remaining_white_moves -= time;
            TraceValue("%u",Nr_unused_black_masses);
            TraceValue("%u\n",Nr_remaining_white_moves);
            SetPiece(pp,placed_on,white[placed_index].flags);
            intelligent_mate_test_target_position(n);
            Nr_remaining_white_moves += time;
            Nr_unused_black_masses += diffcol;
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void white_officer(stip_length_type n,
                          unsigned int placed_index,
                          piece placed_type, square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,placed_on,placed_type))
  {
    square const placed_from = white[placed_index].diagram_square;
    unsigned int const time= intelligent_count_nr_of_moves_from_to_no_check(placed_type,
                                                                            placed_from,
                                                                            placed_type,
                                                                            placed_on);
    if (time<=Nr_remaining_white_moves)
    {
      Flags const placed_flags = white[placed_index].flags;
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(placed_type,placed_on,placed_flags);
      intelligent_mate_test_target_position(n);
      Nr_remaining_white_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_place_any_white_piece_on(stip_length_type n,
                                               square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
    if (white[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = white[placed_index].type;

      white[placed_index].usage = piece_intercepts;

      if (placed_type==pb)
      {
        if (placed_on<=square_h7)
          unpromoted_white_pawn(n,placed_index,placed_on);
        promoted_white_pawn(n,placed_index,placed_on);
      }
      else
        white_officer(n,placed_index,placed_type,placed_on);

      white[placed_index].usage = piece_is_unused;
    }

  e[placed_on]= vide;
  spec[placed_on]= EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
