#include "optimisations/intelligent/place_white_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/intercept_guard_by_white.h"
#include "trace.h"

#include <assert.h>
#include <string.h>


static struct
{
    numvec dir;
    square target;
} GuardDir[4][maxsquare+4];

enum
{
  check_uninterceptable = INT_MAX,
  guard_uninterceptable = INT_MAX-1
};

static void init_guard_dirs_leaper(PieNam guarder,
                                   square target,
                                   unsigned int start, unsigned int end,
                                   numvec value)
{
  unsigned int i;
  for (i = start; i <= end; ++i)
    GuardDir[guarder-Queen][target+vec[i]].dir = value;
}

static void init_guard_dirs_rider(PieNam guarder,
                                  square target,
                                  numvec dir)
{
  square s;
  for (s = target; e[s]==vide; s += dir)
  {
    GuardDir[guarder-Queen][s].dir = -dir;
    GuardDir[guarder-Queen][s].target = target;
  }
}

static void init_guard_dirs_knight(square black_king_pos)
{
  unsigned int i;

  init_guard_dirs_leaper(Knight,
                         black_king_pos,
                         vec_knight_start,vec_knight_end,
                         check_uninterceptable);

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
      init_guard_dirs_leaper(Knight,
                             flight,
                             vec_knight_start,vec_knight_end,
                             guard_uninterceptable);
  }
}

static void init_guard_dirs_queen(square black_king_pos)
{
  unsigned int i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
    {
      unsigned int j;
      for (j = vec_queen_start; j<=vec_queen_end; ++j)
        if (vec[i]!=-vec[j])
          init_guard_dirs_rider(Queen,flight,vec[j]);
    }
  }

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
      init_guard_dirs_leaper(Queen,
                             flight,
                             vec_queen_start,vec_queen_end,
                             guard_uninterceptable);
  }

  init_guard_dirs_leaper(Queen,
                         black_king_pos,
                         vec_queen_start,vec_queen_end,
                         check_uninterceptable);
}

static void init_guard_dirs_rook(square black_king_pos)
{
  unsigned int i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
    {
      unsigned int j;
      for (j = vec_rook_start; j<=vec_rook_end; ++j)
        if (vec[i]!=-vec[j])
          init_guard_dirs_rider(Rook,flight,vec[j]);
    }
  }

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
      init_guard_dirs_leaper(Rook,
                             flight,
                             vec_rook_start,vec_rook_end,
                             guard_uninterceptable);
  }

  init_guard_dirs_leaper(Rook,
                         black_king_pos,
                         vec_rook_start,vec_rook_end,
                         check_uninterceptable);
}

static void init_guard_dirs_bishop(square black_king_pos)
{
  unsigned int i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
    {
      unsigned int j;
      for (j = vec_bishop_start; j<=vec_bishop_end; ++j)
        if (vec[i]!=-vec[j])
          init_guard_dirs_rider(Bishop,flight,vec[j]);
    }
  }

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (e[flight]==vide)
      init_guard_dirs_leaper(Bishop,
                             flight,
                             vec_bishop_start,vec_bishop_end,
                             guard_uninterceptable);
  }

  init_guard_dirs_leaper(Bishop,
                         black_king_pos,
                         vec_bishop_start,vec_bishop_end,
                         check_uninterceptable);
}

/* Initialise the internal structures for fast detection of guards by newly
 * placed white pieces
 * @param black_king_pos position of black king
 */
void init_guard_dirs(square black_king_pos)
{
  memset(GuardDir, 0, sizeof GuardDir);
  init_guard_dirs_queen(black_king_pos);
  init_guard_dirs_rook(black_king_pos);
  init_guard_dirs_bishop(black_king_pos);
  init_guard_dirs_knight(black_king_pos);
}

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
  int const dir = GuardDir[promotee_type-Queen][placed_on].dir;
  square const target = GuardDir[promotee_type-Queen][placed_on].target;

  TraceFunctionEntry(__func__);
  TracePiece(promotee_type);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (dir>=guard_uninterceptable)
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

  if (GuardDir[Knight-Queen][placed_on].dir<guard_uninterceptable
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
  int const dir = GuardDir[placed_type-Queen][placed_on].dir;
  square const target = GuardDir[placed_type-Queen][placed_on].target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (dir>=guard_uninterceptable)
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

  if (GuardDir[Knight-Queen][placed_on].dir<guard_uninterceptable
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
