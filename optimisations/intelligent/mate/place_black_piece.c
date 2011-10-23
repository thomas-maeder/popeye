#include "optimisations/intelligent/mate/place_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void promoted_black_pawn(stip_length_type n,
                                unsigned int placed_index,
                                square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    square const placed_from = black[placed_index].diagram_square;

    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,placed_on,pp)
          && intelligent_reserve_promoting_pawn_moves_from_to(placed_from,
                                                              pp,
                                                              placed_on))
      {
        unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                          pp,
                                                          placed_on);
        SetPiece(pp,placed_on,black[placed_index].flags);
        intelligent_mate_finish(n,nr_of_checks_to_white);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_black_pawn(stip_length_type n,
                                  unsigned int placed_index,
                                  square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(black[placed_index].diagram_square,
                                                                   placed_on))
  {
    SetPiece(pn,placed_on,black[placed_index].flags);
    intelligent_mate_test_target_position(n);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void black_officer(stip_length_type n,
                          unsigned int placed_index,
                          square placed_on)
{
  piece const placed_type = black[placed_index].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,placed_on,placed_type)
      && intelligent_reserve_officer_moves_from_to(black[placed_index].diagram_square,
                                                   placed_on,
                                                   placed_type))
  {
    unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                      placed_type,
                                                      placed_on);
    SetPiece(placed_type,placed_on,black[placed_index].flags);
    intelligent_mate_finish(n,nr_of_checks_to_white);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_place_any_black_piece_on(stip_length_type n,
                                               square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
      if (black[placed_index].usage==piece_is_unused)
      {
        black[placed_index].usage = piece_intercepts;

        if (black[placed_index].type==pn)
        {
          if (placed_on>=square_a2)
            unpromoted_black_pawn(n,placed_index,placed_on);
          promoted_black_pawn(n,placed_index,placed_on);
        }
        else
          black_officer(n,placed_index,placed_on);

        black[placed_index].usage = piece_is_unused;
      }

    e[placed_on] = vide;
    spec[placed_on] = EmptySpec;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
