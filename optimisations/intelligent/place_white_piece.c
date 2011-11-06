#include "optimisations/intelligent/place_white_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "trace.h"

#include <assert.h>


static boolean is_line_empty(square from, square to, int dir)
{
  boolean result = true;
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = from+dir; s!=to; s += dir)
    if (e[s]!=vide)
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
      && GuardDir[Pawn-Pawn][placed_on].dir<guard_dir_guard_uninterceptable
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
  int const dir = GuardDir[promotee_type-Pawn][placed_on].dir;
  square const target = GuardDir[promotee_type-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TracePiece(promotee_type);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (dir>=guard_dir_guard_uninterceptable)
  {
    /* nothing */
  }
  else if (intelligent_reserve_promoting_white_pawn_moves_from_to(placed_comes_from,
                                                                  promotee_type,
                                                                  placed_on))
  {
    SetPiece(promotee_type,placed_on,placed_flags);

    if (dir==0 || e[target]<vide || !is_line_empty(placed_on,target,dir))
      (*go_on)();
    else
      intelligent_intercept_guard_by_white(target,dir,go_on);

    intelligent_unreserve();
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

  if (GuardDir[Knight-Pawn][placed_on].dir<guard_dir_guard_uninterceptable
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
  int const dir = GuardDir[placed_type-Pawn][placed_on].dir;
  square const target = GuardDir[placed_type-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (dir>=guard_dir_guard_uninterceptable)
  {
    /* nothing */
  }
  else if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                     placed_type,
                                                     placed_on))
  {
    SetPiece(placed_type,placed_on,placed_flags);

    if (dir==0 || e[target]<vide || !is_line_empty(placed_on,target,dir))
      (*go_on)();
    else
      intelligent_intercept_guard_by_white(target,dir,go_on);

    intelligent_unreserve();
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

  if (GuardDir[Knight-Pawn][placed_on].dir<guard_dir_guard_uninterceptable
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
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
