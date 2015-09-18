#include "optimisations/intelligent/place_white_piece.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

void intelligent_place_unpromoted_white_pawn(slice_index si,
                                             unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(slice_index si))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!TSTFLAGMASK(sq_spec[placed_on],BIT(WhBaseSq)|BIT(WhPromSq))
      && GuardDir[Pawn-Pawn][placed_on].dir<guard_dir_guard_uninterceptable
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   placed_on))
  {
    occupy_square(placed_on,Pawn,placed_flags);
    (*go_on)(si);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_white_rider(slice_index si,
                                            piece_walk_type promotee_type,
                                            unsigned int placed_index,
                                            square placed_on,
                                            void (*go_on)(slice_index si))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;
  int const dir = GuardDir[promotee_type-Pawn][placed_on].dir;
  square const target = GuardDir[promotee_type-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceWalk(promotee_type);
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
    occupy_square(placed_on,promotee_type,placed_flags);

    if (dir==0 || TSTFLAG(being_solved.spec[target],Black) || !is_line_empty(placed_on,target,dir))
      (*go_on)(si);
    else
      intelligent_intercept_guard_by_white(si,target,dir,go_on);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_promoted_white_knight(slice_index si,
                                             unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(slice_index si))
{
  square const placed_comes_from = white[placed_index].diagram_square;
  Flags const placed_flags = white[placed_index].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (GuardDir[Knight-Pawn][placed_on].dir<guard_dir_guard_uninterceptable
      && intelligent_reserve_promoting_white_pawn_moves_from_to(placed_comes_from,
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

void intelligent_place_promoted_white_pawn(slice_index si,
                                           unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          intelligent_place_promoted_white_rider(si,
                                                 pp,
                                                 placed_index,
                                                 placed_on,
                                                 go_on);
          break;

        case Knight:
          intelligent_place_promoted_white_knight(si,
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

typedef struct stack_elmt_type
{
    unsigned int const index_queen;
    square const placed_on;
    void (*const go_on)(slice_index si);
    struct stack_elmt_type const * const next;
} stack_elmt_type;

static stack_elmt_type const *stack_top = 0;

static void intercept_queen_diag(slice_index si)
{
  square const placed_on = stack_top->placed_on;
  int const dir_diag = GuardDir[Bishop-Pawn][placed_on].dir;
  square const target_diag = GuardDir[Bishop-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (dir_diag==0 || TSTFLAG(being_solved.spec[target_diag],Black) || !is_line_empty(placed_on,target_diag,dir_diag))
    (*stack_top->go_on)(si);
  else
    intelligent_intercept_guard_by_white(si,target_diag,dir_diag,stack_top->go_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_queen(slice_index si,
                                   unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(slice_index si))
{
  piece_walk_type const placed_type = white[placed_index].type;
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
      && intelligent_reserve_officer_moves_from_to(White,
                                                   placed_comes_from,
                                                   placed_type,
                                                   placed_on))
  {
    square const target_ortho = GuardDir[Rook-Pawn][placed_on].target;

    stack_elmt_type const new_top = { placed_index, placed_on, go_on, stack_top };
    stack_top = &new_top;

    occupy_square(placed_on,placed_type,placed_flags);

    if (dir_ortho==0 || TSTFLAG(being_solved.spec[target_ortho],Black) || !is_line_empty(placed_on,target_ortho,dir_ortho))
      intercept_queen_diag(si);
    else
      intelligent_intercept_guard_by_white(si,target_ortho,dir_ortho,&intercept_queen_diag);

    assert(stack_top==&new_top);
    stack_top = stack_top->next;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_rider(slice_index si,
                                   unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(slice_index si))
{
  piece_walk_type const placed_type = white[placed_index].type;
  Flags const placed_flags = white[placed_index].flags;
  square const placed_comes_from = white[placed_index].diagram_square;
  int const dir = GuardDir[placed_type-Pawn][placed_on].dir;
  square const target = GuardDir[placed_type-Pawn][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  TraceValue("%d",dir);
  TraceValue("%d",guard_dir_guard_uninterceptable);
  TraceValue("%u",index_of_guarding_piece);
  TraceEOL();

  switch (dir)
  {
    case guard_dir_check_uninterceptable:
      break;

    case guard_dir_guard_uninterceptable:
      if (placed_index>index_of_guarding_piece
          && intelligent_reserve_officer_moves_from_to(White,
                                                       placed_comes_from,
                                                       placed_type,
                                                       placed_on))
      {
        occupy_square(placed_on,placed_type,placed_flags);
        (*go_on)(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    placed_comes_from,
                                                    placed_type,
                                                    placed_on))
      {
        occupy_square(placed_on,placed_type,placed_flags);

        if (placed_index>index_of_guarding_piece
            || dir==0
            || TSTFLAG(being_solved.spec[target],Black)
            || !is_line_empty(placed_on,target,dir))
          (*go_on)(si);
        else
          intelligent_intercept_guard_by_white(si,target,dir,go_on);

        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_place_white_knight(slice_index si,
                                    unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(slice_index si))
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
          && intelligent_reserve_officer_moves_from_to(White,
                                                       placed_comes_from,
                                                       Knight,
                                                       placed_on))
      {
        occupy_square(placed_on,Knight,placed_flags);
        (*go_on)(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    placed_comes_from,
                                                    Knight,
                                                    placed_on))
      {
        occupy_square(placed_on,Knight,placed_flags);
        (*go_on)(si);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place a white piece
 * @param placed_index index of the piece in array white[]
 * @param placed_on where to place the white piece
 * @param go_on what to do with piece placed_index on square placed_on?
 * @note will leave placed_on occupied by the last piece tried
 */
void intelligent_place_white_piece(slice_index si,
                                   unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  switch (white[placed_index].type)
  {
    case Queen:
      intelligent_place_white_queen(si,placed_index,placed_on,go_on);
      break;

    case Rook:
    case Bishop:
      intelligent_place_white_rider(si,placed_index,placed_on,go_on);
      break;

    case Knight:
      intelligent_place_white_knight(si,placed_index,placed_on,go_on);
      break;

    case Pawn:
      intelligent_place_unpromoted_white_pawn(si,placed_index,placed_on,go_on);
      intelligent_place_promoted_white_pawn(si,placed_index,placed_on,go_on);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
