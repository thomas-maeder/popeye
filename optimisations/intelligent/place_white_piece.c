#include "optimisations/intelligent/place_white_piece.h"
#include "pydata.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>

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
    PieNam pp;
    for (pp = promotee_chain[promotee_chain_orthodox][Empty]; pp!=Empty; pp = promotee_chain[promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          intelligent_place_promoted_white_rider(pp,
                                                 placed_index,
                                                 placed_on,
                                                 go_on);
          break;

        case Knight:
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

typedef struct stack_elmt_type
{
    unsigned int const index_queen;
    square const placed_on;
    void (*const go_on)(void);
    struct stack_elmt_type const * const next;
} stack_elmt_type;

static stack_elmt_type const *stack_top = 0;

static void intercept_queen_diag(void)
{
  square const placed_on = stack_top->placed_on;
  int const dir_diag = GuardDir[Bishop-Pawn][placed_on].dir;
  square const target_diag = GuardDir[Bishop-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (dir_diag==0 || e[target_diag]<vide || !is_line_empty(placed_on,target_diag,dir_diag))
    (*stack_top->go_on)();
  else
    intelligent_intercept_guard_by_white(target_diag,dir_diag,stack_top->go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_queen(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void))
{
  piece const placed_type = white[placed_index].type;
  Flags const placed_flags = white[placed_index].flags;
  square const placed_comes_from = white[placed_index].diagram_square;
  int const dir_ortho = GuardDir[Rook-Pawn][placed_on].dir;
  int const dir_diag = GuardDir[Bishop-Pawn][placed_on].dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (dir_ortho<guard_dir_guard_uninterceptable
      && dir_diag<guard_dir_guard_uninterceptable
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                   placed_type,
                                                   placed_on))
  {
    square const target_ortho = GuardDir[Rook-Pawn][placed_on].target;

    stack_elmt_type const new_top = { placed_index, placed_on, go_on, stack_top };
    stack_top = &new_top;

    SetPiece(placed_type,placed_on,placed_flags);

    if (dir_ortho==0 || e[target_ortho]<vide || !is_line_empty(placed_on,target_ortho,dir_ortho))
      intercept_queen_diag();
    else
      intelligent_intercept_guard_by_white(target_ortho,dir_ortho,&intercept_queen_diag);

    assert(stack_top==&new_top);
    stack_top = stack_top->next;

    intelligent_unreserve();
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

  switch (dir)
  {
    case guard_dir_check_uninterceptable:
      break;

    case guard_dir_guard_uninterceptable:
      if (placed_index>index_of_guarding_piece
          && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                       placed_type,
                                                       placed_on))
      {
        SetPiece(placed_type,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                    placed_type,
                                                    placed_on))
      {
        SetPiece(placed_type,placed_on,placed_flags);

        if (placed_index>index_of_guarding_piece
            || dir==0
            || e[target]<vide
            || !is_line_empty(placed_on,target,dir))
          (*go_on)();
        else
          intelligent_intercept_guard_by_white(target,dir,go_on);

        intelligent_unreserve();
      }
      break;
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

  switch (GuardDir[Knight-Pawn][placed_on].dir)
  {
    case guard_dir_check_uninterceptable:
      break;

    case guard_dir_guard_uninterceptable:
      if (placed_index>index_of_guarding_piece
          && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                       cb,
                                                       placed_on))
      {
        SetPiece(cb,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                    cb,
                                                    placed_on))
      {
        SetPiece(cb,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
      break;
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
      intelligent_place_white_queen(placed_index,placed_on,go_on);
      break;

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
