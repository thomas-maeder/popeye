#include "optimisations/intelligent/place_white_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/intercept_check_by_white.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

enum
{
  checkdir_uninterceptable = INT_MAX
};

static int find_interceptable_check_dir(piece placed_type, square placed_on)
{
  int result;
  int const diff = king_square[Black]-placed_on;

  TraceFunctionEntry(__func__);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  result = CheckDir[abs(placed_type)][diff];

  if (result==diff)
    result = checkdir_uninterceptable;
  else if (result!=0)
  {
    square s;
    for (s = placed_on+result; s!=king_square[Black]; s += result)
      if (e[s]!=vide)
      {
        result = 0;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

void intelligent_place_unpromoted_white_pawn(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (placed_on>=square_a2 && placed_on<=square_h7
      && !white_pawn_attacks_king_region(placed_on,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                     placed_on))
  {
    SetPiece(pb,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_white_rider(piece promotee_type,
                                            unsigned int placed_index,
                                            square placed_on,
                                            void (*go_on)(void))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TracePiece(promotee_type);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(promotee_type,placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(placed_comes_from,
                                                                 promotee_type,
                                                                 placed_on))
      {
        SetPiece(promotee_type,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_promoting_white_pawn_moves_from_to(placed_comes_from,
                                                                       promotee_type,
                                                                       placed_on))
    {
     SetPiece(promotee_type,placed_on,placed_flags);
     intelligent_intercept_check_by_white(check_dir,go_on);
     intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_white_knight(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,placed_on,cb)
      && intelligent_reserve_promoting_white_pawn_moves_from_to(placed_comes_from,
                                                                cb,
                                                                placed_on))
  {
    SetPiece(cb,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_white_pawn(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      switch (pp)
      {
        case db:
        case tb:
        case fb:
          intelligent_place_promoted_white_rider(pp,
                                                 placed_index,
                                                 placed_on,
                                                 go_on);
          break;

        case cb:
          intelligent_place_promoted_white_knight(placed_index,
                                                  placed_on,
                                                  go_on);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_rider(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void))
{
  piece const placed_type = white[placed_index].type;
  Flags const placed_flags = white[placed_index].flags;
  square const placed_comes_from = white[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(placed_type,
                                                       placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                    placed_on,
                                                    placed_type))
      {
        SetPiece(placed_type,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                          placed_on,
                                                          placed_type))
    {
      SetPiece(placed_type,placed_on,placed_flags);
      intelligent_intercept_check_by_white(check_dir,go_on);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_knight(unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(void))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,placed_on,cb)
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                   placed_on,
                                                   cb))
  {
    SetPiece(cb,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_piece(unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (white[placed_index].type)
  {
    case db:
    case tb:
    case fb:
      intelligent_place_white_rider(placed_index,placed_on,go_on);
      break;

    case cb:
      intelligent_place_white_knight(placed_index,placed_on,go_on);
      break;

    case pb:
      intelligent_place_unpromoted_white_pawn(placed_index,placed_on,go_on);
      intelligent_place_promoted_white_pawn(placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
