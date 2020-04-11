#include "optimisations/intelligent/place_black_piece.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/pin_black_piece.h"
#include "optimisations/intelligent/intercept_black_move.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

square const *where_to_start_placing_black_pieces = boardnum;

enum
{
  checkdir_uninterceptable = INT_MAX
};

static int find_interceptable_check_dir(piece_walk_type rider_type, square placed_on)
{
  int result;
  square const king_pos = being_solved.king_square[White];

  TraceFunctionEntry(__func__);
  TraceWalk(rider_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    result = 0;
  else
  {
    int const diff = king_pos-placed_on;
    result = CheckDir(rider_type)[diff];

    if (result==diff)
      result = checkdir_uninterceptable;
    else if (result!=0)
    {
      square s;
      for (s = placed_on+result; s!=king_pos; s += result)
        if (!is_square_empty(s))
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

void intelligent_place_pinned_promoted_black_rider(slice_index si,
                                                   unsigned int placed_index,
                                                   piece_walk_type promotee_type,
                                                   square placed_on,
                                                   void (*go_on)(slice_index si))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceWalk(promotee_type);
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
        occupy_square(placed_on,promotee_type,placed_flags);
        (*go_on)(si);
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                       promotee_type,
                                                                       placed_on))
    {
     occupy_square(placed_on,promotee_type,placed_flags);
     intelligent_intercept_check_by_black(si,check_dir,go_on);
     intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct rider_placement_stack_elmt_type
{
    square const placed_on;
    void (* const go_on)(slice_index si);
    struct rider_placement_stack_elmt_type const * const next;
} rider_placement_stack_elmt_type;

static rider_placement_stack_elmt_type const *stack_top = 0;

static void rider_placed(slice_index si)
{
  rider_placement_stack_elmt_type const * const save_top = stack_top;

  TraceFunctionEntry(__func__);
  TraceSquare(stack_top->placed_on);
  TraceFunctionParamListEnd();

  stack_top = stack_top->next;

  if (being_solved.king_square[White]==initsquare)
    (*save_top->go_on)(si);
  else
  {
    int const check_diff = being_solved.king_square[White]-save_top->placed_on;
    int const check_dir = CheckDir(get_walk_of_piece_on_square(save_top->placed_on))[check_diff];
    assert(check_dir!=check_diff);
    if (check_dir!=0
        && is_line_empty(save_top->placed_on,being_solved.king_square[White],check_dir))
      intelligent_intercept_check_by_black(si,check_dir,save_top->go_on);
    else
      (*save_top->go_on)(si);
  }

  assert(stack_top==save_top->next);
  stack_top = save_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  rider_doesnt_disturb,
  rider_requires_interception,
  rider_requires_pin
} rider_disturbance_type;

static rider_disturbance_type how_does_rider_disturb(piece_walk_type placed_type,
                                                     square placed_on)
{
  rider_disturbance_type result = rider_doesnt_disturb;
  unsigned int const start = disturbance_by_rider_index_ranges[placed_type-Queen].start;
  unsigned int const end = disturbance_by_rider_index_ranges[placed_type-Queen].end;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceWalk(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (i = start; i<=end; ++i)
  {
    int const disturbance_dir = DisturbMateDirRider[i][placed_on].dir;
    if (disturbance_dir==disturbance_by_rider_uninterceptable)
    {
      result = rider_requires_pin;
      break;
    }
    else if (disturbance_dir!=0)
      result = rider_requires_interception;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct rider_interception_stack_elmt_type
{
    unsigned int next;
    unsigned int const end;
    square const placed_on;
    struct rider_interception_stack_elmt_type * const succ;
} rider_interception_stack_elmt_type;

static rider_interception_stack_elmt_type *rider_interception_top = 0;

static void next_rider_interception(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)rider_interception_top);
  TraceFunctionParam("%u",rider_interception_top->next);
  TraceFunctionParam("%u",rider_interception_top->end);
  TraceSquare(rider_interception_top->placed_on);
  TraceFunctionParamListEnd();

  if (rider_interception_top->next<=rider_interception_top->end)
  {
    square const placed_on = rider_interception_top->placed_on;
    int const dir = DisturbMateDirRider[rider_interception_top->next][placed_on].dir;
    square const target = DisturbMateDirRider[rider_interception_top->next][placed_on].target;

    assert(dir!=disturbance_by_rider_uninterceptable);
    ++rider_interception_top->next;

    if (dir!=0 && is_line_empty(placed_on,target,dir))
      intelligent_intercept_black_move(si,placed_on,target,&next_rider_interception);
    else
      next_rider_interception(si);

    assert(rider_interception_top->next>0);
    --rider_interception_top->next;
  }
  else
  {
    rider_interception_stack_elmt_type * const save_top = rider_interception_top;
    rider_interception_top = rider_interception_top->succ;
    rider_placed(si);
    rider_interception_top = save_top;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_rider(slice_index si,
                        unsigned int placed_index,
                        piece_walk_type placed_type,
                        square placed_on,
                        void (*go_on)(slice_index si))
{
  rider_placement_stack_elmt_type const elmt = { placed_on, go_on, stack_top };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceWalk(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  stack_top = &elmt;

  occupy_square(placed_on,placed_type,black[placed_index].flags);

  switch (how_does_rider_disturb(placed_type,placed_on))
  {
    case rider_doesnt_disturb:
      rider_placed(si);
      break;

    case rider_requires_interception:
    {
      rider_interception_stack_elmt_type interception_elmt = {
          disturbance_by_rider_index_ranges[placed_type-Queen].start,
          disturbance_by_rider_index_ranges[placed_type-Queen].end,
          placed_on,
          rider_interception_top
      };
      rider_interception_top = &interception_elmt;
      next_rider_interception(si);
      assert(rider_interception_top==&interception_elmt);
      rider_interception_top = interception_elmt.succ;

      intelligent_pin_black_piece(si,placed_on,&rider_placed);
      break;
    }

    case rider_requires_pin:
      intelligent_pin_black_piece(si,placed_on,&rider_placed);
      break;

    default:
      assert(0);
      break;
  }

  assert(stack_top==&elmt);
  stack_top = elmt.next;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_black_rider(slice_index si,
                                            unsigned int placed_index,
                                            piece_walk_type promotee_type,
                                            square placed_on,
                                            void (*go_on)(slice_index si))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  int const check_diff = being_solved.king_square[White]-placed_on;
  int const check_dir = CheckDir(promotee_type)[check_diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceWalk(promotee_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (check_dir!=check_diff
      && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                promotee_type,
                                                                placed_on))
  {
    place_rider(si,placed_index,promotee_type,placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_promoted_black_knight(slice_index si,
                                                    unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(slice_index si))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((being_solved.king_square[White]==initsquare
       || CheckDir(Knight)[being_solved.king_square[White]-placed_on]==0)
      && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                Knight,
                                                                placed_on))
  {
    occupy_square(placed_on,Knight,placed_flags);
    (*go_on)(si);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_black_knight(slice_index si,
                                             unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(slice_index si))
{
  square const placed_comes_from = black[placed_index].diagram_square;
  Flags const placed_flags = black[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((being_solved.king_square[White]==initsquare
       || CheckDir(Knight)[being_solved.king_square[White]-placed_on]==0)
      && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                Knight,
                                                                placed_on))
  {
    occupy_square(placed_on,Knight,placed_flags);
    if (DisturbMateDirKnight[placed_on]==0)
      (*go_on)(si);
    else
      intelligent_pin_black_piece(si,placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_promoted_black_pawn(slice_index si,
                                                  unsigned int placed_index,
                                                  square placed_on,
                                                  void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          intelligent_place_pinned_promoted_black_rider(si,
                                                        placed_index,
                                                        pp,
                                                        placed_on,
                                                        go_on);
          break;

        case Knight:
          intelligent_place_pinned_promoted_black_knight(si,
                                                         placed_index,
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

void intelligent_place_promoted_black_pawn(slice_index si,
                                           unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          intelligent_place_promoted_black_rider(si,
                                                 placed_index,
                                                 pp,
                                                 placed_on,
                                                 go_on);
          break;

        case Knight:
          intelligent_place_promoted_black_knight(si,
                                                  placed_index,
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

void intelligent_place_pinned_unpromoted_black_pawn(slice_index si,
                                                    unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(slice_index si))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!TSTFLAGMASK(sq_spec(placed_on),BIT(BlBaseSq)|BIT(BlPromSq))
      && !black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   placed_on))
  {
    occupy_square(placed_on,Pawn,placed_flags);
    (*go_on)(si);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_unpromoted_black_pawn(slice_index si,
                                             unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(slice_index si))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!TSTFLAGMASK(sq_spec(placed_on),BIT(BlBaseSq)|BIT(BlPromSq))
      && !black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   placed_on))
  {
    occupy_square(placed_on,Pawn,placed_flags);

    switch (DisturbMateDirPawn[placed_on])
    {
      case disturbance_by_pawn_capture:
      case disturbance_by_pawn_interception_single:
        intelligent_pin_black_piece(si,placed_on,go_on);
        break;

      case disturbance_by_pawn_interception_double:
      {
        square const target = placed_on+2*dir_down;
        assert(is_square_empty(target));
        if (is_square_empty(placed_on+dir_down))
        {
          intelligent_intercept_black_move(si,placed_on,target,go_on);
          intelligent_pin_black_piece(si,placed_on,go_on);
        }
        else
          (*go_on)(si);
        break;
      }

      default:
        (*go_on)(si);
        break;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_rider(slice_index si,
                                          unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(slice_index si))
{
  piece_walk_type const intercepter_type = black[placed_index].type;
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
      if (intelligent_reserve_officer_moves_from_to(Black,
                                                    placed_comes_from,
                                                    intercepter_type,
                                                    placed_on))
      {
        occupy_square(placed_on,intercepter_type,placed_flags);
        (*go_on)(si);
        intelligent_unreserve();
      }
    }
    else if (check_dir!=checkdir_uninterceptable
             && intelligent_reserve_officer_moves_from_to(Black,
                                                          placed_comes_from,
                                                          intercepter_type,
                                                          placed_on))
    {
      occupy_square(placed_on,intercepter_type,placed_flags);
      intelligent_intercept_check_by_black(si,check_dir,go_on);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_black_rider(slice_index si,
                                   unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(slice_index si))
{
  piece_walk_type const intercepter_type = black[placed_index].type;
  square const placed_comes_from = black[placed_index].diagram_square;
  int const check_diff = being_solved.king_square[White]-placed_on;
  int const check_dir = CheckDir(intercepter_type)[check_diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (check_dir!=check_diff
      && intelligent_reserve_officer_moves_from_to(Black,
                                                   placed_comes_from,
                                                   intercepter_type,
                                                   placed_on))
  {
    place_rider(si,placed_index,intercepter_type,placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_knight(slice_index si,
                                           unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(slice_index si))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((being_solved.king_square[White]==initsquare
       || CheckDir(Knight)[being_solved.king_square[White]-placed_on]==0)
      && intelligent_reserve_officer_moves_from_to(Black,
                                                   placed_comes_from,
                                                   Knight,
                                                   placed_on))
  {
    occupy_square(placed_on,Knight,placed_flags);
    (*go_on)(si);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_black_knight(slice_index si,
                                    unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(slice_index si))
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if ((being_solved.king_square[White]==initsquare
       || CheckDir(Knight)[being_solved.king_square[White]-placed_on]==0)
      && intelligent_reserve_officer_moves_from_to(Black,
                                                   placed_comes_from,
                                                   Knight,
                                                   placed_on))
  {
    occupy_square(placed_on,Knight,placed_flags);
    if (DisturbMateDirKnight[placed_on]==0)
      (*go_on)(si);
    else
      intelligent_pin_black_piece(si,placed_on,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place a black piece
 * @param placed_index index of the piece in array black[]
 * @param placed_on where to place the black piece
 * @param go_on what to do with piece placed_index on square placed_on?
 * @note will leave placed_on occupied by the last piece tried
 */
void intelligent_place_black_piece(slice_index si,
                                   unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (black[placed_index].type)
  {
    case Queen:
    case Rook:
    case Bishop:
      intelligent_place_black_rider(si,placed_index,placed_on,go_on);
      break;

    case Knight:
      intelligent_place_black_knight(si,placed_index,placed_on,go_on);
      break;

    case Pawn:
      intelligent_place_promoted_black_pawn(si,placed_index,placed_on,go_on);
      intelligent_place_unpromoted_black_pawn(si,placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_pinned_black_piece(slice_index si,
                                          unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (black[placed_index].type)
  {
    case Queen:
    case Rook:
    case Bishop:
      intelligent_place_pinned_black_rider(si,placed_index,placed_on,go_on);
      break;

    case Knight:
      intelligent_place_pinned_black_knight(si,placed_index,placed_on,go_on);
      break;

    case Pawn:
      intelligent_place_pinned_promoted_black_pawn(si,placed_index,placed_on,go_on);
      intelligent_place_pinned_unpromoted_black_pawn(si,placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
