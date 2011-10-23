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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king(placed_on)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(white[placed_index].diagram_square,
                                                                   placed_on))
  {
    SetPiece(pb,placed_on,white[placed_index].flags);
    intelligent_mate_test_target_position(n);
    intelligent_unreserve();
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

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    square const placed_from = white[placed_index].diagram_square;
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,placed_on,pp)
          && intelligent_reserve_promoting_pawn_moves_from_to(placed_from,
                                                              pp,
                                                              placed_on))
      {
        SetPiece(pp,placed_on,white[placed_index].flags);
        intelligent_mate_test_target_position(n);
        intelligent_unreserve();
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

  if (!officer_uninterceptably_attacks_king(Black,placed_on,placed_type)
      && intelligent_reserve_officer_moves_from_to(white[placed_index].diagram_square,
                                                   placed_on,
                                                   placed_type))
  {
    SetPiece(placed_type,placed_on,white[placed_index].flags);
    intelligent_mate_test_target_position(n);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_place_any_white_piece_on(stip_length_type n,
                                               square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    unsigned int placed_index;
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

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
