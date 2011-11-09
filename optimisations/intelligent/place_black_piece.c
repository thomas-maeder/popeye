#include "optimisations/intelligent/place_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/pin_black_piece.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

square const *where_to_start_placing_black_pieces = boardnum;

enum
{
  checkdir_uninterceptable = INT_MAX
};

static int find_interceptable_check_dir(piece rider_type, square placed_on)
{
  int result;
  square const king_pos = king_square[White];

  TraceFunctionEntry(__func__);
  TracePiece(rider_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    result = 0;
  else
  {
    int const diff = king_pos-placed_on;
    result = CheckDir[abs(rider_type)][diff];

    if (result==diff)
      result = checkdir_uninterceptable;
    else if (result!=0)
    {
      square s;
      for (s = placed_on+result; s!=king_pos; s += result)
        if (e[s]!=vide)
        {
          result = 0;
          break;
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

void intelligent_place_pinned_promoted_black_rider(unsigned int placed_index,
                                                   piece promotee_type,
                                                   square placed_on,
                                                   void (*go_on)(void))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(promotee_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(promotee_type,placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                 promotee_type,
                                                                 placed_on))
      {
        SetPiece(promotee_type,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                       promotee_type,
                                                                       placed_on))
    {
     SetPiece(promotee_type,placed_on,placed_flags);
     intelligent_intercept_check_by_black(check_dir,go_on);
     intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    int const check_dir;
    void (* const go_on)(void);
} stack_elmt_type;

static stack_elmt_type const *stack_top = 0;

static void rider_placed(void)
{
  assert(stack_top!=0);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",stack_top->check_dir);
  TraceFunctionParamListEnd();

  intelligent_intercept_check_by_black(stack_top->check_dir,stack_top->go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean prevent_checking_rider_from_disturbing(piece placed_type,
                                                      square placed_on,
                                                      int check_dir,
                                                      void (*go_on)(void))
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%d",check_dir);
  TraceFunctionParamListEnd();

  for (i = disturbance_by_rider_index_ranges[abs(placed_type)-Queen].start;
       i<=disturbance_by_rider_index_ranges[abs(placed_type)-Queen].end;
       ++i)
    if (DisturbMateDirRider[i][placed_on].dir!=0)
    {
      int const pin_diff = king_square[Black]-placed_on;
      int const pin_dir = CheckDir[Queen][pin_diff];
      if (pin_dir==-check_dir)
        /* pinner is going to intercept check */
        intelligent_pin_black_piece(placed_on,go_on);
      else
      {
        /* first pin, then intercept check */
        stack_elmt_type const * const save_top = stack_top;
        stack_elmt_type const elmt = { check_dir, go_on };
        stack_top = &elmt;
        intelligent_pin_black_piece(placed_on,&rider_placed);
        assert(stack_top==&elmt);
        stack_top = save_top;
      }

      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void place_unpinned_rider_with_check(unsigned int placed_index,
                                            piece placed_type,
                                            square placed_on,
                                            int check_dir,
                                            void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%d",check_dir);
  TraceFunctionParamListEnd();

  SetPiece(placed_type,placed_on,black[placed_index].flags);

  if (!prevent_checking_rider_from_disturbing(placed_type,
                                              placed_on,
                                              check_dir,
                                              go_on))
    intelligent_intercept_check_by_black(check_dir,go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean prevent_nonchecking_rider_from_disturbing(piece placed_type,
                                                         square placed_on,
                                                         void (*go_on)(void))
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (i = disturbance_by_rider_index_ranges[abs(placed_type)-Queen].start;
       i<=disturbance_by_rider_index_ranges[abs(placed_type)-Queen].end;
       ++i)
    if (DisturbMateDirRider[i][placed_on].dir!=0)
    {
      intelligent_pin_black_piece(placed_on,go_on);
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void place_unpinned_rider_without_check(unsigned int placed_index,
                                               piece placed_type,
                                               square placed_on,
                                               void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  SetPiece(placed_type,placed_on,black[placed_index].flags);

  if (!prevent_nonchecking_rider_from_disturbing(placed_type,placed_on,go_on))
    (*go_on)();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_black_rider(unsigned int placed_index,
                                            piece promotee_type,
                                            square placed_on,
                                            void (*go_on)(void))
{
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(promotee_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(promotee_type,placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                 promotee_type,
                                                                 placed_on))
      {
        place_unpinned_rider_without_check(placed_index,promotee_type,placed_on,go_on);
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                       promotee_type,
                                                                       placed_on))
    {
      place_unpinned_rider_with_check(placed_index,promotee_type,placed_on,check_dir,go_on);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_promoted_black_knight(unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(void))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((king_square[White]==initsquare
       || CheckDir[Knight][king_square[White]-placed_on]==0)
      && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                cn,
                                                                placed_on))
  {
    SetPiece(cn,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_black_knight(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((king_square[White]==initsquare
       || CheckDir[Knight][king_square[White]-placed_on]==0)
      && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                cn,
                                                                placed_on))
  {
    SetPiece(cn,placed_on,placed_flags);
    if (DisturbMateDirKnight[placed_on]==0)
      (*go_on)();
    else
      intelligent_pin_black_piece(placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_promoted_black_pawn(unsigned int placed_index,
                                                  square placed_on,
                                                  void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      switch (pp)
      {
        case dn:
        case tn:
        case fn:
          intelligent_place_pinned_promoted_black_rider(placed_index,
                                                        pp,
                                                        placed_on,
                                                        go_on);
          break;

        case cn:
          intelligent_place_pinned_promoted_black_knight(placed_index,placed_on,go_on);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_black_pawn(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      switch (pp)
      {
        case dn:
        case tn:
        case fn:
          intelligent_place_promoted_black_rider(placed_index,
                                                 pp,
                                                 placed_on,
                                                 go_on);
          break;

        case cn:
          intelligent_place_promoted_black_knight(placed_index,placed_on,go_on);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_unpromoted_black_pawn(unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(void))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (placed_on>=square_a2 && placed_on<=square_h7
      && !black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   placed_on))
  {
    SetPiece(pn,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_unpromoted_black_pawn(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (placed_on>=square_a2 && placed_on<=square_h7
      && !black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   placed_on))
  {
    SetPiece(pn,placed_on,placed_flags);
    if (DisturbMateDirPawn[placed_on]==disturbance_by_pawn_capture
        || DisturbMateDirPawn[placed_on]==disturbance_by_pawn_interception_single)
      intelligent_pin_black_piece(placed_on,go_on);
    else
      (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_rider(unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(void))
{
  piece const intercepter_type = black[placed_index].type;
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(intercepter_type,
                                                       placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                    intercepter_type,
                                                    placed_on))
      {
        SetPiece(intercepter_type,placed_on,placed_flags);
        (*go_on)();
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                          intercepter_type,
                                                          placed_on))
    {
      SetPiece(intercepter_type,placed_on,placed_flags);
      intelligent_intercept_check_by_black(check_dir,go_on);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_black_rider(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void))
{
  piece const intercepter_type = black[placed_index].type;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    int const check_dir = find_interceptable_check_dir(intercepter_type,
                                                       placed_on);
    if (check_dir==0)
    {
      if (intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                    intercepter_type,
                                                    placed_on))
      {
        place_unpinned_rider_without_check(placed_index,intercepter_type,placed_on,go_on);
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                          intercepter_type,
                                                          placed_on))
    {
      place_unpinned_rider_with_check(placed_index,intercepter_type,placed_on,check_dir,go_on);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_knight(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((king_square[White]==initsquare
       || CheckDir[Knight][king_square[White]-placed_on]==0)
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                   cn,
                                                   placed_on))
  {
    SetPiece(cn,placed_on,placed_flags);
    (*go_on)();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_black_knight(unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(void))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((king_square[White]==initsquare
       || CheckDir[Knight][king_square[White]-placed_on]==0)
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                   cn,
                                                   placed_on))
  {
    SetPiece(cn,placed_on,placed_flags);
    if (DisturbMateDirKnight[placed_on]==0)
      (*go_on)();
    else
      intelligent_pin_black_piece(placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_black_piece(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (black[placed_index].type)
  {
    case dn:
    case tn:
    case fn:
      intelligent_place_black_rider(placed_index,placed_on,go_on);
      break;

    case cn:
      intelligent_place_black_knight(placed_index,placed_on,go_on);
      break;

    case pn:
      intelligent_place_promoted_black_pawn(placed_index,placed_on,go_on);
      intelligent_place_unpromoted_black_pawn(placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_piece(unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (black[placed_index].type)
  {
    case dn:
    case tn:
    case fn:
      intelligent_place_pinned_black_rider(placed_index,placed_on,go_on);
      break;

    case cn:
      intelligent_place_pinned_black_knight(placed_index,placed_on,go_on);
      break;

    case pn:
      intelligent_place_pinned_promoted_black_pawn(placed_index,placed_on,go_on);
      intelligent_place_pinned_unpromoted_black_pawn(placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
