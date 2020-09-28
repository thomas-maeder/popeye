#include "optimisations/intelligent/guard_flights.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "position/move_diff_code.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>
#include <string.h>

/* index of guarding piece currently being placed */
unsigned int index_of_guarding_piece;

guard_dir_struct GuardDir[5][maxsquare+4];

static void init_guard_dirs_leaper(piece_walk_type guarder,
                                   square target,
                                   vec_index_type start, vec_index_type end,
                                   numvec value)
{
  vec_index_type i;
  for (i = start; i <= end; ++i)
    GuardDir[guarder-Pawn][target+vec[i]].dir = value;
}

static void init_guard_dirs_rider(piece_walk_type guarder,
                                  square flight,
                                  numvec dir)
{
  square const start = flight+dir;
  if (move_diff_code[abs(being_solved.king_square[Black]-start)]<=2)
  {
    /* start is a flight, too.
     * GuardDir will be initialised from start in this dir */
  }
  else
  {
    square s;
    for (s = start; is_square_empty(s); s += dir)
    {
      GuardDir[guarder-Pawn][s].dir = -dir;
      GuardDir[guarder-Pawn][s].target = flight;
    }
  }
}

static void init_guard_dirs_queen(square black_king_pos)
{
  vec_index_type i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
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
    if (is_square_empty(flight))
      init_guard_dirs_leaper(Queen,
                             flight,
                             vec_queen_start,vec_queen_end,
                             guard_dir_guard_uninterceptable);
  }

  init_guard_dirs_leaper(Queen,
                         black_king_pos,
                         vec_queen_start,vec_queen_end,
                         guard_dir_check_uninterceptable);
}

static void init_guard_dirs_rook(square black_king_pos)
{
  vec_index_type i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
    {
      vec_index_type j;
      for (j = vec_rook_start; j<=vec_rook_end; ++j)
        if (vec[i]!=-vec[j])
          init_guard_dirs_rider(Rook,flight,vec[j]);
    }
  }

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
      init_guard_dirs_leaper(Rook,
                             flight,
                             vec_rook_start,vec_rook_end,
                             guard_dir_guard_uninterceptable);
  }

  init_guard_dirs_leaper(Rook,
                         black_king_pos,
                         vec_rook_start,vec_rook_end,
                         guard_dir_check_uninterceptable);
}

static void init_guard_dirs_bishop(square black_king_pos)
{
  vec_index_type i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
    {
      vec_index_type j;
      for (j = vec_bishop_start; j<=vec_bishop_end; ++j)
        if (vec[i]!=-vec[j])
          init_guard_dirs_rider(Bishop,flight,vec[j]);
    }
  }

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
      init_guard_dirs_leaper(Bishop,
                             flight,
                             vec_bishop_start,vec_bishop_end,
                             guard_dir_guard_uninterceptable);
  }

  init_guard_dirs_leaper(Bishop,
                         black_king_pos,
                         vec_bishop_start,vec_bishop_end,
                         guard_dir_check_uninterceptable);
}

static void init_guard_dirs_knight(square black_king_pos)
{
  vec_index_type i;

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
      init_guard_dirs_leaper(Knight,
                             flight,
                             vec_knight_start,vec_knight_end,
                             guard_dir_guard_uninterceptable);
  }

  /* only now - check trumps guard */
  init_guard_dirs_leaper(Knight,
                         black_king_pos,
                         vec_knight_start,vec_knight_end,
                         guard_dir_check_uninterceptable);
}

static void init_guard_dir_pawn(square flight, numvec dir)
{
  GuardDir[Pawn-Pawn][flight+dir_down+dir_left].dir = dir;
  GuardDir[Pawn-Pawn][flight+dir_down+dir_left].target = flight;
  GuardDir[Pawn-Pawn][flight+dir_down+dir_right].dir = dir;
  GuardDir[Pawn-Pawn][flight+dir_down+dir_right].target = flight;
}

static void init_guard_dirs_pawn(square black_king_pos)
{
  vec_index_type i;
  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = black_king_pos+vec[i];
    if (is_square_empty(flight))
      init_guard_dir_pawn(flight,guard_dir_guard_uninterceptable);
  }

  init_guard_dir_pawn(black_king_pos,guard_dir_check_uninterceptable);
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
  init_guard_dirs_pawn(black_king_pos);
}

/* Does the white king guard a flight
 * @param from where might the king guard?
 * @return true iff the king guards a flight
 */
static boolean white_king_guards_flight(square from)
{
  move_diff_type const diff = move_diff_code[abs(being_solved.king_square[Black]-from)];
  boolean const result = diff>3 && diff<=8;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* find out whether to remember to keep a guard line open, and do it
 * @param from start of line
 * @param to end of line
 * @param type type of rider
 * @param delta [+-1] remember resp. forget to keep the line open
 */
static void remember_to_keep_guard_line_open(square from, square to,
                                             int delta)
{
  int const dir = CheckDir(Queen)[to-from];
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%d",dir);
  TraceEOL();

  /* the guard line only needs to be kept open up to the flight closest to
   * from; e.g. reset to to c1 with from:a1 to:e1 being_solved.king_square[Black]:d2
   */
  for (s = to-dir; s!=from && move_diff_code[abs(being_solved.king_square[Black]-s)]<=2; s -= dir)
    to = s;

  remember_to_keep_rider_line_open(from,to,dir,delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* fix the white king on its diagram square
 */
static void fix_white_king_on_diagram_square(slice_index si)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_square_empty(king_diagram_square)
      && nr_reasons_for_staying_empty[king_diagram_square]==0)
  {
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;
    intelligent_place_white_king(si,king_diagram_square,&pipe_solve_delegate);
    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* continue after guarding
 */
static void guarding_done(slice_index si)
{
  unsigned int const save_index_of_guarding_piece = index_of_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  index_of_guarding_piece = UINT_MAX;

  if (white[index_of_king].usage==piece_is_unused
      && white[index_of_king].diagram_square!=square_e1
      && intelligent_get_nr_remaining_moves(White)==0)
    fix_white_king_on_diagram_square(si);
  else
    pipe_solve_delegate(si);

  index_of_guarding_piece = save_index_of_guarding_piece;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) queen in opposition to the black king
 * @param guard_from from what square should the queen guard
 */
static void place_queen_opposition(slice_index si, square guard_from)
{
  square const to_be_intercepted = (being_solved.king_square[Black]+guard_from)/2;

  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  occupy_square(guard_from,Queen,white[index_of_guarding_piece].flags);
  if (is_square_empty(to_be_intercepted))
  {
    assert(nr_reasons_for_staying_empty[to_be_intercepted]==0);
    intercept_check_on_guarded_square(si,to_be_intercepted);
    intelligent_intercept_check_by_pin(si,to_be_intercepted);
    empty_square(to_be_intercepted);
  }
  else
    /* there is already a guard between queen and king */
    intelligent_continue_guarding_flights(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) rider
 * @param rider_type type of rider
 * @param guard_from from what square should the rider guard
 */
static void place_rider(slice_index si,
                        piece_walk_type rider_type,
                        square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceWalk(rider_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  {
    int const dir = GuardDir[rider_type-Pawn][guard_from].dir;

    TraceValue("%d",dir);
    TraceValue("%d",guard_dir_check_uninterceptable);
    TraceEOL();

    switch (dir)
    {
      case guard_dir_check_uninterceptable:
      case 0:
        break;

      case guard_dir_guard_uninterceptable:
      {
        square const guarded = GuardDir[rider_type-Pawn][guard_from].target;
        TraceSquare(guarded);
        TraceValue("%u",TSTFLAG(being_solved.spec[guarded],Black));
        TraceWalk(get_walk_of_piece_on_square(guarded));
        TraceEOL();
        if (!TSTFLAG(being_solved.spec[guarded],Black))
        {
          occupy_square(guard_from,rider_type,white[index_of_guarding_piece].flags);
          if (CheckDir(rider_type)[being_solved.king_square[Black]-guard_from]!=0
              && is_square_empty(guarded))
          {
            assert(nr_reasons_for_staying_empty[guarded]==0);
            intercept_check_on_guarded_square(si,guarded);
            intelligent_intercept_check_by_pin(si,guarded);
            empty_square(guarded);
          }
          else
            intelligent_continue_guarding_flights(si);
        }
        break;
      }

      default:
      {
        square const guarded = GuardDir[rider_type-Pawn][guard_from].target;
        TraceSquare(guarded);
        TraceValue("%u",TSTFLAG(being_solved.spec[guarded],Black));
        TraceEOL();
        if (!TSTFLAG(being_solved.spec[guarded],Black) && is_line_empty(guard_from,guarded,dir))
        {
          occupy_square(guard_from,rider_type,white[index_of_guarding_piece].flags);
          remember_to_keep_guard_line_open(guard_from,guarded,+1);
          if (CheckDir(rider_type)[being_solved.king_square[Black]-guard_from]!=0)
          {
            if (is_square_empty(guarded))
            {
              assert(nr_reasons_for_staying_empty[guarded]==0);
              intercept_check_on_guarded_square(si,guarded);
              intelligent_intercept_check_by_pin(si,guarded);
              empty_square(guarded);
            }
            else
            {
              PieceIdType const id = GetPieceId(being_solved.spec[guarded]);
              if (white[PieceId2index[id]].usage==piece_guards)
              {
                white[PieceId2index[id]].usage = piece_intercepts_check_from_guard;
                intelligent_continue_guarding_flights(si);
                white[PieceId2index[id]].usage = piece_guards;
              }
              else
                intelligent_continue_guarding_flights(si);
            }
          }
          else
            intelligent_continue_guarding_flights(si);
          remember_to_keep_guard_line_open(guard_from,guarded,-1);
        }
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) knight
 * @param guard_from from what square should the knight guard
 */
static void place_knight(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (GuardDir[Knight-Pawn][guard_from].dir==guard_dir_guard_uninterceptable)
  {
    occupy_square(guard_from,Knight,white[index_of_guarding_piece].flags);
    intelligent_continue_guarding_flights(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with an unpromoted pawn
 * @param guard_from from what square should the pawn guard
 */
static void unpromoted_pawn(slice_index si, square guard_from)
{
  Flags const pawn_flags = white[index_of_guarding_piece].flags;
  square const starts_from = white[index_of_guarding_piece].diagram_square;

  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (!TSTFLAGMASK(sq_spec(guard_from),BIT(WhBaseSq)|BIT(WhPromSq))
      && GuardDir[Pawn-Pawn][guard_from].dir==guard_dir_guard_uninterceptable
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(starts_from,
                                                                   guard_from))
  {
    occupy_square(guard_from,Pawn,pawn_flags);
    intelligent_continue_guarding_flights(si);
    empty_square(guard_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted queen
 * @param guard_from from what square should the queen guard
 */
static void promoted_queen(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(being_solved.king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Qb2 */
    case 1+1: /* e.g. Ka2 Qb3 */
      /* uninterceptable check */
      break;

    case 4+0: /* e.g. Ka2 Qc2 */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                 Queen,
                                                                 guard_from))
      {
        place_queen_opposition(si,guard_from);
        intelligent_unreserve();
      }
      break;

    case 4+1: /* e.g. Ka2 Qc3 - no lines to be kept open */
    case 9+1: /* e.g. Ka2 Qd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                 Queen,
                                                                 guard_from))
      {
        occupy_square(guard_from,Queen,white[index_of_guarding_piece].flags);
        intelligent_continue_guarding_flights(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                 Queen,
                                                                 guard_from))
      {
        place_rider(si,Queen,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a rook
 * @param guard_from from what square should the rook guard
 */
static void promoted_rook(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(being_solved.king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Rb2 */
      /* uninterceptable check */
      break;

    case 1+1: /* e.g. Kc2 Rd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                 Rook,
                                                                 guard_from))
      {
        occupy_square(guard_from,Rook,white[index_of_guarding_piece].flags);
        intelligent_continue_guarding_flights(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                 Rook,
                                                                 guard_from))
      {
        place_rider(si,Rook,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted bishop
 * @param guard_from from what square should the bishop guard
 */
static void promoted_bishop(slice_index si, square guard_from)
{
  move_diff_type const diff = move_diff_code[abs(being_solved.king_square[Black]-guard_from)];

  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (diff==1+1) /* e.g. Ka2 Bb3 */
  {
    /* uninterceptable check */
  }
  else if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                  Bishop,
                                                                  guard_from))
  {
    if (diff<=4+0)
    {
      occupy_square(guard_from,Bishop,white[index_of_guarding_piece].flags);
      intelligent_continue_guarding_flights(si);
    }
    else
      place_rider(si,Bishop,guard_from);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted knight
 * @param guard_from from what square should the knight guard
 */
static void promoted_knight(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (CheckDir(Knight)[being_solved.king_square[Black]-guard_from]==0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_guarding_piece].diagram_square,
                                                                Knight,
                                                                guard_from))
  {
    place_knight(si,guard_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted pawn
 * @param guard_from from what square should the promotee guard
 */
static void promoted_pawn(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_guarding_piece,
                                                                guard_from))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
          promoted_queen(si,guard_from);
          break;

        case Rook:
          promoted_rook(si,guard_from);
          break;

        case Bishop:
          promoted_bishop(si,guard_from);
          break;

        case Knight:
          promoted_knight(si,guard_from);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a queen
 * @param guard_from from what square should the queen guard
 */
static void queen(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(being_solved.king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Qb2 */
    case 1+1: /* e.g. Ka2 Qb3 */
      /* uninterceptable check */
      break;

    case 4+0: /* e.g. Ka2 Qc2 */
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    white[index_of_guarding_piece].diagram_square,
                                                    Queen,
                                                    guard_from))
      {
        place_queen_opposition(si,guard_from);
        intelligent_unreserve();
      }
      break;

    case 4+1: /* e.g. Ka2 Qc3 - no lines to be kept open */
    case 9+1: /* e.g. Ka2 Qd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    white[index_of_guarding_piece].diagram_square,
                                                    Queen,
                                                    guard_from))
      {
        occupy_square(guard_from,Queen,white[index_of_guarding_piece].flags);
        intelligent_continue_guarding_flights(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    white[index_of_guarding_piece].diagram_square,
                                                    Queen,
                                                    guard_from))
      {
        place_rider(si,Queen,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a rook
 * @param guard_from from what square should the rook guard
 */
static void rook(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(being_solved.king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Rb2 */
      /* uninterceptable check */
      break;

    case 1+1: /* e.g. Kc2 Rd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    white[index_of_guarding_piece].diagram_square,
                                                    Rook,
                                                    guard_from))
      {
        occupy_square(guard_from,Rook,white[index_of_guarding_piece].flags);
        intelligent_continue_guarding_flights(si);
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(White,
                                                    white[index_of_guarding_piece].diagram_square,
                                                    Rook,
                                                    guard_from))
      {
        place_rider(si,Rook,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a bishop
 * @param guard_from from what square should the bishop guard
 */
static void bishop(slice_index si, square guard_from)
{
  move_diff_type const diff = move_diff_code[abs(being_solved.king_square[Black]-guard_from)];

  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (diff==1+1) /* e.g. Ka2 Bb3 */
  {
    /* uninterceptable check */
  }
  else if (intelligent_reserve_officer_moves_from_to(White,
                                                     white[index_of_guarding_piece].diagram_square,
                                                     Bishop,
                                                     guard_from))
  {
    if (diff<=4+0)
    {
      occupy_square(guard_from,Bishop,white[index_of_guarding_piece].flags);
      intelligent_continue_guarding_flights(si);
    }
    else
      place_rider(si,Bishop,guard_from);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a white knight
 * @param guard_from from what square should the knight guard
 */
static void knight(slice_index si, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (CheckDir(Knight)[being_solved.king_square[Black]-guard_from]==0
      && intelligent_reserve_officer_moves_from_to(White,
                                                   white[index_of_guarding_piece].diagram_square,
                                                   Knight,
                                                   guard_from))
  {
    place_knight(si,guard_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_next_flight(slice_index si)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (is_square_empty(*bnp) && nr_reasons_for_staying_empty[*bnp]==0)
    {
      switch (white[index_of_guarding_piece].type)
      {
        case Queen:
          queen(si,*bnp);
          break;

        case Rook:
          rook(si,*bnp);
          break;

        case Bishop:
          bishop(si,*bnp);
          break;

        case Knight:
          knight(si,*bnp);
          break;

        case Pawn:
          unpromoted_pawn(si,*bnp);
          promoted_pawn(si,*bnp);
          break;

        default:
          assert(0);
          break;
      }

      empty_square(*bnp);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard more king flights */
void intelligent_continue_guarding_flights(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1,piece_guards))
  {
    unsigned int const save_index_guarder = index_of_guarding_piece;

    for (++index_of_guarding_piece; index_of_guarding_piece<MaxPiece[White]; ++index_of_guarding_piece)
      if (white[index_of_guarding_piece].usage==piece_is_unused)
      {
        white[index_of_guarding_piece].usage = piece_guards;
        guard_next_flight(si);
        white[index_of_guarding_piece].usage = piece_is_unused;
      }

    index_of_guarding_piece = save_index_guarder;

    intelligent_unreserve();
  }

  guarding_done(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with the white king */
static void king(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    if (intelligent_reserve_masses(White,1,piece_guards))
    {
      square const comes_from = white[index_of_king].diagram_square;
      square const *bnp;

      white[index_of_king].usage = piece_guards;

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (is_square_empty(*bnp)
            && nr_reasons_for_staying_empty[*bnp]==0
            && white_king_guards_flight(*bnp)
            && intelligent_reserve_white_king_moves_from_to(comes_from,*bnp))
        {
          being_solved.king_square[White]= *bnp;
          occupy_square(*bnp,King,white[index_of_king].flags);
          intelligent_continue_guarding_flights(si);
          empty_square(*bnp);
          intelligent_unreserve();
        }

      being_solved.king_square[White] = initsquare;
      white[index_of_king].usage = piece_is_unused;

      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard the king flights */
void intelligent_guard_flights(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king(si);

  TraceText("try not using white king for guarding\n");
  intelligent_continue_guarding_flights(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
