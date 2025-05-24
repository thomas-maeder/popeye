/******************** MODIFICATIONS to pyint.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/06/14 TLi  bug fix in function guards_black_flight()
 **
 ** 2007/12/27 TLi  bug fix in function stalemate_immobilise_black()
 **
 **************************** End of List ******************************/

#include "optimisations/intelligent/intelligent.h"

#include "pieces/walks/pawns/en_passant.h"
#include "solving/proofgames.h"
#include "solving/machinery/solve.h"
#include "solving/castling.h"
#include "solving/check.h"
#include "solving/temporary_hacks.h"
#include "solving/pipe.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/fork.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "optimisations/intelligent/proof.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/mate/finish.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "optimisations/intelligent/mate/generate_doublechecking_moves.h"
#include "optimisations/intelligent/piece_usage.h"
#include "optimisations/intelligent/proof.h"
#include "options/maxsolutions/guard.h"
#include "options/maxtime.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "platform/maxtime.h"
#include "options/options.h"

#include "debugging/assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef JUST_OUTPUT_TARGET_POSITIONS
#include <ctype.h>
#endif

typedef unsigned int index_type;

goal_type goal_to_be_reached;

unsigned int MaxPiece[nr_sides];

PIECE white[nr_squares_on_board];
PIECE black[nr_squares_on_board];
unsigned int moves_to_white_prom[nr_squares_on_board];

PIECE target_position[MaxPieceId+1];

boolean solutions_found;

boolean testcastling;

unsigned int MovesRequired[nr_sides][maxply+1];
unsigned int CapturesLeft[maxply+1];

unsigned int PieceId2index[MaxPieceId+1];

unsigned int nr_reasons_for_staying_empty[maxsquare+4];

typedef struct
{
  Flags       spec[nr_squares_on_board];
  piece_walk_type      e[nr_squares_on_board];
  square      rn_sic, rb_sic;
} stored_position_type;

static stored_position_type initial_position;

static void StorePosition(stored_position_type *store)
{
  store->rn_sic = being_solved.king_square[Black];
  store->rb_sic = being_solved.king_square[White];

  {
    unsigned int i;
    for (i = 0; i<nr_squares_on_board; i++)
    {
      store->e[i] = get_walk_of_piece_on_square(boardnum[i]);
      store->spec[i] = being_solved.spec[boardnum[i]];
    }
  }
}

static void ResetPosition(stored_position_type const *store)
{
  {
    piece_walk_type p;
    for (p = King; p<nr_piece_walks; ++p)
    {
      being_solved.number_of_pieces[White][p] = 0;
      being_solved.number_of_pieces[Black][p] = 0;
    }
  }

  being_solved.king_square[Black] = store->rn_sic;
  being_solved.king_square[White] = store->rb_sic;

  {
    unsigned int i;
    for (i = 0; i<nr_squares_on_board; i++)
      switch (store->e[i])
      {
        case Empty:
          empty_square(boardnum[i]);
          break;

        case Invalid:
          block_square(boardnum[i]);
          break;

        default:
        {
          Side const side = TSTFLAG(store->spec[i],White) ? White : Black;
          ++being_solved.number_of_pieces[side][store->e[i]];
          occupy_square(boardnum[i],store->e[i],store->spec[i]);
          break;
        }
      }
  }
}

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta)
{
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  for (s = from+dir; s!=to; s+=dir)
  {
    /*assert(is_square_empty(s)); doesn't work if there are holes! */
    // TODO does this overflow work on all implementations?
    assert(abs(delta)==1);
    assert(nr_reasons_for_staying_empty[s]>0 || delta>0);
    nr_reasons_for_staying_empty[s] += (unsigned int)delta;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
/* Detrmine whether some line is empty
 * @param start start of line
 * @param end end of line
 * @param dir direction from start to end
 * @return true iff the line is empty
 */
boolean is_line_empty(square start, square end, int dir)
{
  boolean result = true;
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = start+dir; s!=end; s += dir)
    if (!is_square_empty(s))
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean black_pawn_attacks_king(square from)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(sq_spec(from),BlPromSq));
  assert(!TSTFLAG(sq_spec(from),BlBaseSq));

  if (being_solved.king_square[White]==initsquare)
    result = false;
  else
  {
    int const diff = being_solved.king_square[White]-from;
    result = diff==dir_down+dir_right || diff==dir_down+dir_left;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/*#define DETAILS*/
#if defined(DETAILS)
static void trace_target_position(PIECE const position[MaxPieceId+1],
                                  unsigned int nr_required_captures)
{
  unsigned int moves_per_side[nr_sides] = { 0, 0 };
  square const *bnp;

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (!is_square_empty(*bnp))
    {
      Flags const sp = being_solved.spec[*bnp];
      PieceIdType const id = GetPieceId(sp);
      PIECE const * const target = &position[id];
      if (target->diagram_square!=/* vide */ Empty /* TODO: Is Empty the correct value here? */)
      {
        Side const cur_side = TSTFLAG(being_solved.spec[*bnp],White) ? White : Black;
        unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(cur_side,
                                                                     get_walk_of_piece_on_square(*bnp),
                                                                     *bnp,
                                                                     target->type,
                                                                     target->diagram_square);
        moves_per_side[cur_side] += time;
        TraceWalk(get_walk_of_piece_on_square(*bnp));
        TraceSquare(*bnp);
        TraceWalk(target->type);
        TraceSquare(target->diagram_square);
        TraceEnumerator(piece_usage,target->usage);
        TraceValue("%u",time);
        TraceEOL();
      }
    }

  TraceValue("%u",nr_required_captures);
  TraceValue("%u",moves_per_side[Black]);
  TraceValue("%u",moves_per_side[White]);
  TraceEOL();
}

static piece_usage find_piece_usage(PieceIdType id)
{
  piece_usage result = piece_is_unused;

  unsigned int i;
  for (i = 0; i<MaxPiece[White]; ++i)
    if (id==GetPieceId(white[i].flags))
    {
      result = white[i].usage;
      break;
    }
  for (i = 0; i<MaxPiece[Black]; ++i)
    if (id==GetPieceId(black[i].flags))
    {
      result = black[i].usage;
      break;
    }

  assert(result!=piece_is_unused);

  return result;
}
#endif

static square PositionInDiagram_to_square(int pos)
{
  pos -= square_a1;
  return (square) (((pos / onerow) * nr_files_on_board) +
                    (pos % onerow));
}

#if 0
static int square_to_PositionInDiagram(square sq)
{
  int const pos = (((sq / nr_files_on_board) * onerow) + (sq % nr_files_on_board));
  return (pos + square_a1);
}
#endif

static square orig_square_of_piece(Flags const flags)
{
  return PositionInDiagram_to_square(GetPositionInDiagram(flags));
}

typedef struct {
  piece_walk_type piece;
  Side color;
  square orig_square;
} piece_on_square;

typedef struct {
  square blocking_square[max_nr_straight_rider_steps - 1];
  int num_blocking_squares;
  square checking_square;
} line_check;

static line_check get_line_check_up(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (square_checked + nr_files_on_board);
  for (int sq = adjacent; sq < nr_squares_on_board; sq += nr_files_on_board)
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Rook) ||
          ((p == King) && (sq == adjacent)))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_down(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (((int) square_checked) - nr_files_on_board);
  for (int sq = adjacent; sq >= 0; sq -= nr_files_on_board)
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Rook) ||
          ((p == King) && (sq == adjacent)))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_left(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (((int) square_checked) - 1);
  for (int sq = square_checked; (sq % nr_files_on_board);)
  {
    --sq;
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Rook) ||
          ((p == King) && (sq == adjacent)))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_right(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (square_checked + 1);
  for (int sq = adjacent; (sq % nr_files_on_board); ++sq)
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Rook) ||
          ((p == King) && (sq == adjacent)))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_upper_left(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (square_checked + (nr_files_on_board - 1));
  for (int sq = adjacent; ((sq < nr_squares_on_board) && ((sq % nr_files_on_board) != (nr_files_on_board - 1))); sq += (nr_files_on_board - 1))
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Bishop) ||
          ((sq == adjacent) && ((p == King) ||
                                ((p == Pawn) && (color_checked == White)))))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_upper_right(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};
  int const adjacent = (square_checked + (nr_files_on_board + 1));
  for (int sq = adjacent; ((sq < nr_squares_on_board) && (sq % nr_files_on_board)); sq += (nr_files_on_board + 1))
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Bishop) ||
          ((sq == adjacent) && ((p == King) ||
                                ((p == Pawn) && (color_checked == White)))))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_lower_left(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (square_checked - (nr_files_on_board + 1));
  for (int sq = adjacent; ((sq >= 0) && ((sq % nr_files_on_board) != (nr_files_on_board - 1))); sq -= (nr_files_on_board + 1))
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Bishop) ||
          ((sq == adjacent) && ((p == King) ||
                                ((p == Pawn) && (color_checked == Black)))))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static line_check get_line_check_lower_right(piece_on_square const * const final, square const square_checked, Side const color_checked)
{
  line_check returnVal = {{0}, 0, nr_squares_on_board};

  int const adjacent = (square_checked - (nr_files_on_board - 1));
  for (int sq = adjacent; ((sq >= 0) && (sq % nr_files_on_board)); sq -= (nr_files_on_board - 1))
  {
    if (final[sq].color == color_checked)
    {
      returnVal.num_blocking_squares = 0;
      break;
    }
    const piece_walk_type p = final[sq].piece;
    if (p == Empty)
      returnVal.blocking_square[returnVal.num_blocking_squares++] = sq;
    else
    {
      if ((p == Queen) ||
          (p == Bishop) ||
          ((sq == adjacent) && ((p == King) ||
                                ((p == Pawn) && (color_checked == Black)))))
        returnVal.checking_square = sq;
      else
        returnVal.num_blocking_squares = 0;
      break;
    }
  }
  return returnVal;
}

static int get_knight_checks(piece_on_square const * const final, square const square_checked, Side const color_checked, square * const checking_squares)
{
  int const row = (square_checked / nr_files_on_board);
  int const col = (square_checked % nr_files_on_board);
  int num_checks = 0;
  if (row)
  {
    if (col > 1)
    {
      int const knight_square = (square_checked - (nr_files_on_board + 2));
      if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
        checking_squares[num_checks++] = knight_square;
    }
    if (col < (nr_files_on_board - 2))
    {
      int const knight_square = (square_checked - (nr_files_on_board - 2));
      if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
        checking_squares[num_checks++] = knight_square;
    }
    if (row > 1)
    {
      if (col)
      {
        int const knight_square = (square_checked - ((2 * nr_files_on_board) + 1));
        if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
          checking_squares[num_checks++] = knight_square;
      }
      if (col < (nr_files_on_board - 1))
      {
        int const knight_square = (square_checked - ((2 * nr_files_on_board) - 1));
        if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
          checking_squares[num_checks++] = knight_square;
      }
    }
  }
  if (row < (nr_rows_on_board - 1))
  {
    if (col > 1)
    {
      int const knight_square = (square_checked + (nr_files_on_board - 2));
      if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
        checking_squares[num_checks++] = knight_square;
    }
    if (col < (nr_files_on_board - 2))
    {
      int const knight_square = (square_checked + (nr_files_on_board + 2));
      if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
        checking_squares[num_checks++] = knight_square;
    }
    if (row < (nr_rows_on_board - 2))
    {
      if (col)
      {
        int const knight_square = (square_checked + ((2 * nr_files_on_board) - 1));
        if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
          checking_squares[num_checks++] = knight_square;
      }
      if (col < (nr_files_on_board - 1))
      {
        int const knight_square = (square_checked + ((2 * nr_files_on_board) + 1));
        if ((final[knight_square].piece == Knight) && (final[knight_square].color != color_checked))
          checking_squares[num_checks++] = knight_square;
      }
    }
  }
  return num_checks;
}

enum
{
  ensure_unsigned_long_long_has_enough_bits = 1/(nr_squares_on_board <= 64) // TODO: replace 64 with the number of value bits in unsigned long long
};

static piece_on_square target_before_white_move[nr_squares_on_board];
static square pieces_not_in_target[nr_squares_on_board];
static int num_pieces_not_in_target;
static boolean castle_kingside;
static boolean castle_queenside;
boolean white_must_capture;
static square white_capture_square;
static square final_square_of_piece[nr_squares_on_board];
static unsigned long long square_must_remain_open;
static unsigned int problem_orig_num_moves;
static boolean maybe_series_mover;
static unsigned long long black_pawn_double_step;
static unsigned long long white_pawn_double_step;

typedef struct {
  square blocking_squares[((nr_files_on_board > nr_rows_on_board) ? nr_files_on_board : nr_rows_on_board) - 2];
  square checking_square;
  int num_blocking_squares;
} LineCheck;

#ifdef JUST_OUTPUT_TARGET_POSITIONS
static void output_target_position(void) {
  char cur_fen[nr_squares_on_board + (nr_rows_on_board - 1) + 1];
  int cur_fen_index = 0;
  int num_spaces = 0;
  for (int row = (nr_rows_on_board - 1); row >= 0; --row)
  {
    for (int col = 0; col < nr_files_on_board; ++col)
    {
      int cur_index = ((row * nr_files_on_board) + col);
      int cur_piece = target_before_white_move[cur_index].piece;
      if (cur_piece == Empty)
        ++num_spaces;
      else
      {
        if (num_spaces)
        {
          cur_fen[cur_fen_index++] = (char) (num_spaces + '0');
          num_spaces = 0;
        }
        switch (cur_piece)
        {
          case Pawn:
            cur_piece = 'p';
            break;
          case Knight:
            cur_piece = 's';
            break;
          case Bishop:
            cur_piece = 'b';
            break;
          case Rook:
            cur_piece = 'r';
            break;
          case Queen:
            cur_piece = 'q';
            break;
          case King:
            cur_piece = 'k';
            break;
          case nr_piece_walks:
            cur_piece = '*';
            break;
          default:
            cur_piece = '?';
        }
        if (target_before_white_move[cur_index].color == White)
          cur_piece = toupper(cur_piece);
        cur_fen[cur_fen_index++] = (char) cur_piece;
      }
    }
    if (num_spaces)
    {
      cur_fen[cur_fen_index++] = (char) (num_spaces + '0');
      num_spaces = 0;
    }
    if (row)
      cur_fen[cur_fen_index++] = '/';
  }
  cur_fen[cur_fen_index] = '\0';
  printf("\ntarget position FEN: %s\n", cur_fen);
}
#endif

static boolean get_target_before_white_move(stored_position_type const * const store)
{
  enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
  };

  if (MovesLeft[White] != 1) // TODO: replace with a more precise test
  {
    maybe_series_mover = false;
    return true;
  }

  // Convert the position to a more convenient form.
  unsigned long long seen_black_pieces = 0;
  square bKPosition = nr_squares_on_board;
//  a8RFinalPosition = nr_squares_on_board;
//  h8RFinalPosition = nr_squares_on_board;
  piece_on_square initial[nr_squares_on_board];
  black_pawn_double_step = 0;
  white_pawn_double_step = 0;
  for (int index = a1; index <= h8; ++index)
  {
    target_before_white_move[index].piece = Empty;
    target_before_white_move[index].color = no_side;
    target_before_white_move[index].orig_square = nr_squares_on_board;
    final_square_of_piece[index] = nr_squares_on_board;
    square const cur_square = boardnum[index];
    if (TSTFLAG(sq_spec(cur_square), BlPawnDoublestepSq))
      black_pawn_double_step |= (1ULL << index);
    if (TSTFLAG(sq_spec(cur_square), WhPawnDoublestepSq))
      white_pawn_double_step |= (1ULL << index);
    piece_walk_type p = get_walk_of_piece_on_square(cur_square);
    if (p == Invalid)
      p = nr_piece_walks;
    initial[index].piece = p;
    if (p == Empty)
    {
      initial[index].color = no_side;
      initial[index].orig_square = nr_squares_on_board;
    }
    else
    {
      Flags const flags = being_solved.spec[cur_square];
      if (TSTFLAG(flags, White))
        initial[index].color = White;
      else
        initial[index].color = Black;
      initial[index].orig_square = orig_square_of_piece(flags);
    }
  }

  int moved_white_piece_orig_square[2];
  int moved_white_piece_new_square[2];
  piece_walk_type moved_white_piece_type[2];
  int num_moved_white_pieces = 0;
  for (PieceIdType id = 0; id <= MaxPieceId; ++id)
  {
    square const diagram_square = target_position[id].diagram_square;
    if (diagram_square != initsquare)
    {
      Flags const flags = target_position[id].flags;
      square const orig_square = orig_square_of_piece(flags);
      if ((orig_square < a1) || (orig_square > h8))
        return false;
      piece_walk_type p = target_position[id].type;
      if (p == Invalid)
        p = nr_piece_walks;
      int const index = PositionInDiagram_to_square(diagram_square);
      target_before_white_move[index].piece = p;
      target_before_white_move[index].orig_square = orig_square;
      if (TSTFLAG(flags, White))
      {
        target_before_white_move[index].color = White;
        if (orig_square != index)
        {
          if (num_moved_white_pieces > 1)
            return false;
          moved_white_piece_orig_square[num_moved_white_pieces] = orig_square;
          moved_white_piece_new_square[num_moved_white_pieces] = index;
          moved_white_piece_type[num_moved_white_pieces++] = initial[orig_square].piece;
        }
      }
      else
      {
        switch (p)
        {
          case King:
            bKPosition = index;
            break;
          case Pawn:
            {
              int const orig_row = (orig_square / nr_files_on_board);
              int const final_row = (index / nr_files_on_board);
              if (orig_row < final_row)
                return false;
              int const orig_col = (orig_square % nr_files_on_board);
              int const final_col = (index % nr_files_on_board);
              if ((orig_row - final_row) < abs(orig_col - final_col))
                return false;
            }
            break;
          default:
            ; // do nothing
        }
        target_before_white_move[index].color = Black;
        seen_black_pieces |= (1ULL << orig_square);
        final_square_of_piece[orig_square] = index;
      }
    }
  }

  castle_kingside = false;
  castle_queenside = false;
  if (num_moved_white_pieces == 2)
  {
    if (moved_white_piece_orig_square[0] == e1)
    {
      if (moved_white_piece_type[0] != King)
        return false;
      if (moved_white_piece_type[1] != Rook)
        return false;
      if ((moved_white_piece_new_square[0] == c1) &&
          (moved_white_piece_orig_square[1] == a1) &&
          (moved_white_piece_new_square[1] == d1))
        castle_queenside = true;
      else if ((moved_white_piece_new_square[0] == g1) &&
               (moved_white_piece_orig_square[1] == h1) &&
               (moved_white_piece_new_square[1] == f1))
        castle_kingside = true;
      else
        return false;
      // Put the Rook -- which can deliver check -- in slot 0.
      int tmp = moved_white_piece_orig_square[0];
      moved_white_piece_orig_square[0] = moved_white_piece_orig_square[1];
      moved_white_piece_orig_square[1] = tmp;
      tmp = moved_white_piece_new_square[0];
      moved_white_piece_new_square[0] = moved_white_piece_new_square[1];
      moved_white_piece_new_square[1] = tmp;
      moved_white_piece_type[0] = Rook;
      moved_white_piece_type[1] = King;
    }
    else if (moved_white_piece_orig_square[1] == e1)
    {
      if (moved_white_piece_type[1] != King)
        return false;
      if (moved_white_piece_type[0] != Rook)
        return false;
      if ((moved_white_piece_new_square[1] == c1) &&
          (moved_white_piece_orig_square[0] == a1) &&
          (moved_white_piece_new_square[0] == d1))
        castle_queenside = true;
      else if ((moved_white_piece_new_square[1] == g1) &&
               (moved_white_piece_orig_square[0] == h1) &&
               (moved_white_piece_new_square[0] == f1))
        castle_kingside = true;
      else
        return false;
    }
    else
      return false;
  }
  else if (num_moved_white_pieces == 1)
    if (moved_white_piece_type[0] == King)
    {
      if (moved_white_piece_orig_square[0] == e1)
      {
        if (moved_white_piece_new_square[0] == c1)
        {
          if ((target_before_white_move[d1].piece != Empty) ||
              (target_before_white_move[b1].piece != Empty))
            return false;
          if ((initial[a1].piece != Rook) ||
              (initial[a1].color != White))
            return false;
          castle_queenside = true;
          num_moved_white_pieces = 2;
          // Put the Rook -- which can deliver check -- in slot 0.
          moved_white_piece_type[0] = Rook;
          moved_white_piece_orig_square[0] = a1;
          moved_white_piece_new_square[0] = d1;
          moved_white_piece_type[1] = King;
          moved_white_piece_orig_square[1] = e1;
          moved_white_piece_new_square[1] = c1;
          target_before_white_move[d1].piece = Rook;
          target_before_white_move[d1].color = White;
          target_before_white_move[d1].orig_square = a1;
        }
        else if (moved_white_piece_new_square[0] == g1)
        {
          if (target_before_white_move[f1].piece != Empty)
            return false;
          if ((initial[h1].piece != Rook) ||
              (initial[h1].color != White))
            return false;
          castle_kingside = true;
          num_moved_white_pieces = 2;
          // Put the Rook -- which can deliver check -- in slot 0.
          moved_white_piece_type[0] = Rook;
          moved_white_piece_orig_square[0] = h1;
          moved_white_piece_new_square[0] = f1;
          moved_white_piece_type[1] = King;
          moved_white_piece_orig_square[1] = e1;
          moved_white_piece_new_square[1] = g1;
          target_before_white_move[f1].piece = Rook;
          target_before_white_move[f1].color = White;
          target_before_white_move[f1].orig_square = h1;
        }
      }
    }

  for (int i = 0; i < num_moved_white_pieces; ++i)
    if (target_before_white_move[moved_white_piece_orig_square[i]].piece != Empty)
      return false;

  // determine if we're looking for a checkmate or stalemate
  square knight_checks[8];
  int const num_knight_checks = get_knight_checks(target_before_white_move, bKPosition, Black, knight_checks);
  if (num_knight_checks > 1)
    return false;
  if (num_knight_checks == 1)
  {
    if (num_moved_white_pieces != 1)
      return false;
    if (moved_white_piece_new_square[0] != knight_checks[0])
      return false;
  }
  line_check all_line_checks[8];
  int num_line_checks = 0;
  line_check tmp_line_check = get_line_check_up(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_down(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_left(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_right(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_upper_left(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_upper_right(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_lower_left(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;
  tmp_line_check = get_line_check_lower_right(target_before_white_move, bKPosition, Black);
  if (tmp_line_check.checking_square < nr_squares_on_board)
    all_line_checks[num_line_checks++] = tmp_line_check;

  if ((num_line_checks + num_knight_checks) > 2)
    return false;
  square_must_remain_open = 0;
  if (num_line_checks > 0)
  {
    if (num_knight_checks > 0)
    {
      // Did the Knight discover check?
      for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
        if (all_line_checks[0].blocking_square[i] == moved_white_piece_orig_square[0])
          goto LEGAL_CHECK;
    }
    else if (num_line_checks == 2)
    {
      // Did either piece discover check?
      if (num_moved_white_pieces != 1)
        return false;
      if (moved_white_piece_type[0] != Queen)
      {
        if (all_line_checks[0].checking_square == moved_white_piece_new_square[0])
        {
          for (int i = 0; i < all_line_checks[1].num_blocking_squares; ++i)
            if (all_line_checks[1].blocking_square[i] == moved_white_piece_orig_square[0])
              goto LEGAL_CHECK;
        }
        else if (all_line_checks[1].checking_square == moved_white_piece_new_square[0])
        {
          for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
            if (all_line_checks[0].blocking_square[i] == moved_white_piece_orig_square[0])
              goto LEGAL_CHECK;
        }
        else if ((moved_white_piece_type[0] == Pawn) &&
                 (moved_white_piece_orig_square[0] >= a5) &&
                 (moved_white_piece_orig_square[0] <= h5) &&
                 ((moved_white_piece_new_square[0] - moved_white_piece_orig_square[0]) % nr_files_on_board))
        {
          // Handle the possibility of an en passant capture that discovers check from two other pieces.
          int ep_block_index = -1;
          for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
            if (all_line_checks[0].blocking_square[i] == moved_white_piece_orig_square[0])
            {
              ep_block_index = 1;
              break;
            }
          if (ep_block_index < 0)
            for (int i = 0; i < all_line_checks[1].num_blocking_squares; ++i)
              if (all_line_checks[1].blocking_square[i] == moved_white_piece_orig_square[0])
              {
                ep_block_index = 0;
                break;
              }
          if (ep_block_index >= 0)
            for (int i = 0; i < all_line_checks[ep_block_index].num_blocking_squares; ++i)
              if (all_line_checks[ep_block_index].blocking_square[i] == (moved_white_piece_new_square[0] - nr_files_on_board))
                goto LEGAL_CHECK;
        }
      }
    }
    else
    {
      // exactly one line check
      if (num_moved_white_pieces > 0)
      {
        if (all_line_checks[0].checking_square == moved_white_piece_new_square[0])
        {
          // If it's not a pawn promotion, it couldn't have come from a blocking square.
          if ((moved_white_piece_type[0] != Pawn) || ((moved_white_piece_new_square[0] / nr_files_on_board) != (nr_rows_on_board - 1)))
            for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
              if (all_line_checks[0].blocking_square[i] == moved_white_piece_orig_square[0])
                return false;
          goto LEGAL_CHECK;
        }
        else
        {
          // The moved piece must have discovered check.
          for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
            if (all_line_checks[0].blocking_square[i] == moved_white_piece_orig_square[0])
              goto LEGAL_CHECK;
          if ((moved_white_piece_type[0] == Pawn) &&
              (moved_white_piece_orig_square[0] >= a5) &&
              (moved_white_piece_orig_square[0] <= h5) &&
              ((moved_white_piece_new_square[0] - moved_white_piece_orig_square[0]) % nr_files_on_board))
            // Maybe it discovered check by capturing a block pawn en passant.
            for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
              if (all_line_checks[0].blocking_square[i] == (moved_white_piece_new_square[0] - nr_files_on_board))
                goto LEGAL_CHECK;
        }
      }
      else
      {
        // check must have been discovered by a piece no longer on the board
        // TODO TODO TODO: determine the actual starting squares of the pieces.
        for (int i = 0; i < all_line_checks[0].num_blocking_squares; ++i)
        {
          square const b = all_line_checks[0].blocking_square[i];
          square const cur_square = boardnum[b];
          piece_walk_type const p = get_walk_of_piece_on_square(cur_square);
          if (initial[b].color == White)
          {
            if (num_moved_white_pieces > 0)
            {
              num_moved_white_pieces = 0;
              goto LEGAL_CHECK;
            }
            num_moved_white_pieces = 1;
            moved_white_piece_type[0] = p;
            moved_white_piece_orig_square[0] = b;
            moved_white_piece_new_square[0] = nr_squares_on_board;
          }
        }
        if (num_moved_white_pieces > 0)
          goto LEGAL_CHECK;
      }
    }
    return false;
LEGAL_CHECK:
    for (int i = 0; i < num_line_checks; ++i)
      for (int j = 0; j < all_line_checks[i].num_blocking_squares; ++j)
        square_must_remain_open |= (1ULL << all_line_checks[i].blocking_square[j]);
  }
  else if (num_moved_white_pieces <= 0)
  {
    // anything not appearing could have moved
    for (int i = 0; i < nr_squares_on_board; ++i)
      if (target_before_white_move[i].piece == Empty)
      {
        if (initial[i].color == White)
        {
          if (num_moved_white_pieces > 0)
          {
            num_moved_white_pieces = 0;
            goto FOUND_POSSIBLE_MOVE;
          }
          num_moved_white_pieces = 1;
          moved_white_piece_type[0] = initial[i].piece;
          moved_white_piece_orig_square[0] = initial[i].orig_square;
          moved_white_piece_new_square[0] = nr_squares_on_board;
        }
      }
    if (num_moved_white_pieces <= 0)
      return false;
FOUND_POSSIBLE_MOVE:;
  }

  boolean const black_in_check = ((num_line_checks > 0) || (num_knight_checks > 0));

  // ensure that White's move was legal
  if (castle_queenside)
  {
    if ((target_before_white_move[a1].piece != Empty) ||
        (target_before_white_move[b1].piece != Empty) ||
        (target_before_white_move[e1].piece != Empty))
      return false;
    square_must_remain_open |= ((1ULL << b1) | (1ULL << c1) | (1ULL << d1));
  }
  else if (castle_kingside)
  {
    if ((target_before_white_move[e1].piece != Empty) ||
        (target_before_white_move[h1].piece != Empty))
      return false;
    square_must_remain_open |= ((1ULL << f1) | (1ULL << g1));
  }
  else
    for (int i = 0; i < num_moved_white_pieces; ++i)
    {
      // the square white moved from must be empty
      int orig_square = moved_white_piece_orig_square[i];
      if (target_before_white_move[orig_square].piece != Empty)
        return false;
      // a White line piece can't have jumped over anything
      int const new_square = moved_white_piece_new_square[i];
      if ((new_square < nr_squares_on_board) && (moved_white_piece_type[i] != Knight))
      {
        // line pieces cannot have jumped over anything
        int orig_rank = (orig_square / nr_files_on_board);
        int orig_file = (orig_square % nr_files_on_board);
        int const new_rank = (new_square / nr_files_on_board);
        int const new_file = (new_square % nr_files_on_board);
        int step;
        if (new_rank > orig_rank)
          step = nr_files_on_board;
        else if (new_rank < orig_rank)
          step = -((int) nr_files_on_board);
        else
          step = 0;
        if (new_file > orig_file)
          ++step;
        else if (new_file < orig_file)
          --step;
        while ((orig_square += step) != new_square)
        {
          if (target_before_white_move[orig_square].piece != Empty)
            return false;
          square_must_remain_open |= (1ULL << orig_square);
        }
      }
    }

  // retract White's last move
  white_must_capture = false;
  for (int i = 0; i < num_moved_white_pieces; ++i)
  {
    int const orig_square = moved_white_piece_orig_square[i];
    piece_walk_type const p = moved_white_piece_type[i];
    target_before_white_move[orig_square].piece = p;
    target_before_white_move[orig_square].color = White;
    target_before_white_move[orig_square].orig_square = orig_square;
    int const new_square = moved_white_piece_new_square[i];
    if (new_square < nr_squares_on_board)
    {
      target_before_white_move[new_square].piece = Empty;
      target_before_white_move[new_square].color = no_side;
      target_before_white_move[new_square].orig_square = nr_squares_on_board;
      if ((p == Pawn) &&
          ((new_square - orig_square) % nr_files_on_board))
      {
        white_must_capture = true;
        white_capture_square = new_square;
      }
    }
  }

  num_pieces_not_in_target = 0;
  int index_of_captured_piece;
  for (int index = a1; index <= h8; ++index)
  {
    if ((initial[index].color == Black) &&
        (initial[index].piece < nr_piece_walks))
    {
      square const orig_square = initial[index].orig_square;
      if (!(seen_black_pieces & (1ULL << orig_square)))

      {
        if (!black_in_check)
        {
          // We're looking for stalemate, so we should see every piece except possibly one that is captured.
          if (num_pieces_not_in_target > 0)
            return false;

          // some moves can't be captures
          if (castle_kingside || castle_queenside)
            return false;
          if ((num_moved_white_pieces > 0) &&
              (moved_white_piece_type[0] == Pawn) &&
              (moved_white_piece_new_square[0] < nr_squares_on_board) &&
              !white_must_capture)
            return false;
        }
        index_of_captured_piece = index;
        pieces_not_in_target[num_pieces_not_in_target++] = orig_square;
      }
    }
  }
  if (num_pieces_not_in_target > 0)
  {
    if ((num_moved_white_pieces > 0) && !black_in_check)
    {
      white_must_capture = true;
      white_capture_square = moved_white_piece_new_square[0];
    }
  }
  else
  {
    if (white_must_capture)
      return false;
    for (int i = 0; i < num_moved_white_pieces; ++i)
      if (moved_white_piece_new_square[i] < nr_squares_on_board)
        square_must_remain_open |= (1ULL << moved_white_piece_new_square[i]);
  }

  if (white_must_capture && (white_capture_square < nr_squares_on_board))
  {
    target_before_white_move[white_capture_square].color = Black;
    if (num_pieces_not_in_target == 1)
    {
      piece_walk_type const captured_piece = initial[index_of_captured_piece].piece;
      if (captured_piece == Pawn)
        target_before_white_move[white_capture_square].piece = nr_piece_walks; // The pawn might have promoted.
      else
        target_before_white_move[white_capture_square].piece = captured_piece;
      target_before_white_move[white_capture_square].orig_square = pieces_not_in_target[0];
      final_square_of_piece[pieces_not_in_target[0]] = white_capture_square;
      num_pieces_not_in_target = 0;
      white_must_capture = false;
    }
    else
    {
      target_before_white_move[white_capture_square].piece = nr_piece_walks;
      target_before_white_move[white_capture_square].orig_square = nr_squares_on_board;
    }
  }

  // restore any missing White Kings (which can't have been captured)
  for (int i = 0; i < nr_squares_on_board; ++i)
    if ((initial[i].piece == King) &&
        (initial[i].color == White))
    {
      target_before_white_move[i].piece = King;
      target_before_white_move[i].color = White;
      target_before_white_move[i].orig_square = i;
    }

  // TODO: add any blockers to ensure that Black isn't in check and that White doesn't castle out of check

  problem_orig_num_moves = MovesLeft[Black];
  maybe_series_mover = true;
  return true;
}

struct move_generator
{
  unsigned int (* move_generating_function)(struct move_generator const * mg, unsigned char * destinations, square from);
  unsigned long long possible_froms;
  unsigned long long possible_tos;
};
typedef struct move_generator move_generator;

typedef unsigned int (* get_legal_moves_function_ptr)(move_generator const * mg, unsigned char * destinations, square from);

typedef struct
{
  move_generator mg;
  unsigned long long obstructions;
} rider_move_generator;

typedef struct
{
  rider_move_generator rg;
  unsigned long long occupied_squares;
  unsigned long long capturable_pieces;
  unsigned long long white_pawn_locations;
  square orig_square;
  EnPassantLegality ep;
  boolean original_problem_position;
} pawn_move_generator;

static unsigned int get_legal_black_pawn_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  pawn_move_generator const generator = *((pawn_move_generator const *) (rider_move_generator const *) mg);
  unsigned long long const landing_squares = generator.rg.mg.possible_tos & ~generator.rg.obstructions;
  unsigned int num_destinations = 0;
  if ((from >= nr_files_on_board) && (generator.rg.mg.possible_froms & (1ULL << from)))
  {
    int const row = (from / nr_files_on_board);
    int const col = (from % nr_files_on_board);
    if (landing_squares & (1ULL << (from - nr_files_on_board)))
      destinations[num_destinations++] = (unsigned char) (from - nr_files_on_board);
    if ((row >= 2) &&
        (black_pawn_double_step & (1ULL << from)) &&
        (landing_squares & (1ULL << (from - (nr_files_on_board * 2)))) &&
        !(generator.rg.obstructions & (1ULL << (from - nr_files_on_board))))
      destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board * 2));
    if (col &&
        (landing_squares & (1ULL << (from - (nr_files_on_board + 1)))))
    {
      if (generator.capturable_pieces & (1ULL << (from - (nr_files_on_board + 1))))
        destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board + 1));
      else if ((row >= 2) &&
               (white_pawn_double_step & (1ULL << (from - ((2 * nr_files_on_board) + 1)))) &&
               ((generator.white_pawn_locations & generator.capturable_pieces) & (1ULL << (from - 1))))
      {
        if (generator.ep == EP_CONSEQUENT)
        {
          if (!(generator.rg.obstructions & ((1ULL | (1ULL << nr_files_on_board)) << (from - ((2 * nr_files_on_board) + 1)))))
            destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board + 1));
        }
        else if (generator.ep == EP_ON_FIRST_MOVE)
          if (generator.original_problem_position &&
              (from == generator.orig_square) &&
              !(generator.occupied_squares & ((1ULL | (1ULL << nr_files_on_board)) << (from - ((2 * nr_files_on_board) + 1)))))
            destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board + 1));
      }
    }
    if ((col < (nr_files_on_board - 1)) &&
        (landing_squares & (1ULL << (from - (nr_files_on_board - 1)))))
    {
      if (generator.capturable_pieces & (1ULL << (from - (nr_files_on_board - 1))))
        destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board - 1));
      else if ((row >= 2) &&
               (white_pawn_double_step & (1ULL << (from - ((2 * nr_files_on_board) - 1)))) &&
               ((generator.white_pawn_locations & generator.capturable_pieces) & (1ULL << (from + 1))))
      {
        if (generator.ep == EP_CONSEQUENT)
        {
          if (!(generator.rg.obstructions & ((1ULL | (1ULL << nr_files_on_board)) << (from - ((2 * nr_files_on_board) - 1)))))
            destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board - 1));
        }
        else if (generator.ep == EP_ON_FIRST_MOVE)
          if (generator.original_problem_position &&
              (from == generator.orig_square) &&
              !(generator.occupied_squares & ((1ULL | (1ULL << nr_files_on_board)) << (from - ((2 * nr_files_on_board) - 1)))))
            destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board - 1));
      }
    }
  }

  return num_destinations;
}

static unsigned int get_legal_knight_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  move_generator const generator = *mg;
  unsigned int num_destinations = 0;
  if (generator.possible_froms & (1ULL << from))
  {
    int const row = (from / nr_files_on_board);
    int const col = (from % nr_files_on_board);
    if (row)
    {
      if (col > 1)
        if (generator.possible_tos & (1ULL << (from - (nr_files_on_board + 2))))
          destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board + 2));
      if (col < (nr_files_on_board - 2))
        if (generator.possible_tos & (1ULL << (from - (nr_files_on_board - 2))))
          destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board - 2));
      if (row > 1)
      {
        if (col)
          if (generator.possible_tos & (1ULL << (from - ((2 * nr_files_on_board) + 1))))
            destinations[num_destinations++] = (unsigned char) (from - ((2 * nr_files_on_board) + 1));
        if (col < (nr_files_on_board - 1))
          if (generator.possible_tos & (1ULL << (from - ((2 * nr_files_on_board) - 1))))
            destinations[num_destinations++] = (unsigned char) (from - ((2 * nr_files_on_board) - 1));
      }
    }
    if (row < (nr_rows_on_board - 1))
    {
      if (col > 1)
        if (generator.possible_tos & (1ULL << (from + (nr_files_on_board - 2))))
          destinations[num_destinations++] = (unsigned char) (from + (nr_files_on_board - 2));
      if (col < (nr_files_on_board - 2))
        if (generator.possible_tos & (1ULL << (from + (nr_files_on_board + 2))))
          destinations[num_destinations++] = (unsigned char) (from + (nr_files_on_board + 2));
      if (row < (nr_rows_on_board - 2))
      {
        if (col)
          if (generator.possible_tos & (1ULL << (from + ((2 * nr_files_on_board) - 1))))
            destinations[num_destinations++] = (unsigned char) (from + ((2 * nr_files_on_board) - 1));
        if (col < (nr_files_on_board - 1))
          if (generator.possible_tos & (1ULL << (from + ((2 * nr_files_on_board) + 1))))
            destinations[num_destinations++] = (unsigned char) (from + ((2 * nr_files_on_board) + 1));
      }
    }
  }

  return num_destinations;
}

static unsigned int get_legal_bishop_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  rider_move_generator const generator = *((rider_move_generator const *) mg);
  unsigned int num_destinations = 0;
  if (generator.mg.possible_froms & (1ULL << from))
  {
    for (int poss_move = (from + (nr_files_on_board - 1)); ((poss_move < (int) nr_squares_on_board) && ((poss_move % nr_files_on_board) != (nr_files_on_board - 1))); poss_move += (nr_files_on_board - 1))
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = (from + (nr_files_on_board + 1)); ((poss_move < (int) nr_squares_on_board) && (poss_move % nr_files_on_board)); poss_move += (nr_files_on_board + 1))
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = (from - (nr_files_on_board - 1)); ((poss_move >= 0) && (poss_move % nr_files_on_board)); poss_move -= (nr_files_on_board - 1))
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = (from - (nr_files_on_board + 1)); ((poss_move >= 0) && ((poss_move % nr_files_on_board) != (nr_files_on_board - 1))); poss_move -= (nr_files_on_board + 1))
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
  }

  return num_destinations;
}

static unsigned int get_legal_rook_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  rider_move_generator const generator = *((rider_move_generator const *) mg);
  unsigned int num_destinations = 0;
  if (generator.mg.possible_froms & (1ULL << from))
  {
    for (int poss_move = (from + 1); (poss_move % nr_files_on_board); ++poss_move)
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL <<  poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = (from + nr_files_on_board); poss_move < (int) nr_squares_on_board; poss_move += nr_files_on_board)
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = from; (poss_move % nr_files_on_board);)
    {
      --poss_move;
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
    for (int poss_move = (from - nr_files_on_board); poss_move >= 0; poss_move -= nr_files_on_board)
    {
      if (generator.obstructions & (1ULL << poss_move))
        break;
      if (generator.mg.possible_tos & (1ULL << poss_move))
        destinations[num_destinations++] = (unsigned char) poss_move;
    }
  }

  return num_destinations;
}

static unsigned int get_legal_queen_moves(move_generator const * const mg, unsigned char * const destinations, square const from) {
  unsigned int num_moves = get_legal_bishop_moves(mg, destinations, from);
  num_moves += get_legal_rook_moves(mg, (destinations + num_moves), from);
  return num_moves;
}

static unsigned int get_legal_king_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  move_generator const generator = *mg;
  unsigned int num_destinations = 0;
  if (generator.possible_froms & (1ULL << from))
  {
    int const row = (from / nr_files_on_board);
    int const col = (from % nr_files_on_board);
    if (row)
    {
      if (generator.possible_tos & (1ULL << (from - nr_files_on_board)))
        destinations[num_destinations++] = (unsigned char) (from - nr_files_on_board);
      if (col)
        if (generator.possible_tos & (1ULL << (from - (nr_files_on_board + 1))))
          destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board + 1));
      if (col < (nr_files_on_board - 1))
        if (generator.possible_tos & (1ULL << (from - (nr_files_on_board - 1))))
          destinations[num_destinations++] = (unsigned char) (from - (nr_files_on_board - 1));
    }
    if (col)
      if (generator.possible_tos & (1ULL << (from - 1)))
        destinations[num_destinations++] = (unsigned char) (from - 1);
    if (col < (nr_files_on_board - 1))
      if (generator.possible_tos & (1ULL << (from + 1)))
        destinations[num_destinations++] = (unsigned char) (from + 1);
    if (row < (nr_rows_on_board - 1))
    {
      if (generator.possible_tos & (1ULL << (from + nr_files_on_board)))
        destinations[num_destinations++] = (unsigned char) (from + nr_files_on_board);
      if (col)
        if (generator.possible_tos & (1ULL << (from + (nr_files_on_board - 1))))
          destinations[num_destinations++] = (unsigned char) (from + (nr_files_on_board - 1));
      if (col < (nr_files_on_board - 1))
        if (generator.possible_tos & (1ULL << (from + (nr_files_on_board + 1))))
          destinations[num_destinations++] = (unsigned char) (from + (nr_files_on_board + 1));
    }
  }

  return num_destinations;
}

static unsigned int get_legal_dummy_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  return 0;
}

static unsigned int get_legal_unknown_moves(move_generator const * const mg, unsigned char * const destinations, square const from)
{
  unsigned int num_destinations = 0;
  move_generator const generator = *mg;
  if (generator.possible_froms & (1ULL << from))
    for (int i = 0; i < nr_squares_on_board; ++i)
      if (generator.possible_tos & (1ULL << i))
        destinations[num_destinations++] = (unsigned char) i;
  return num_destinations;
}

static unsigned int get_length_of_shortest_path(move_generator const * const mg, int const from, unsigned long long const target_tos)
{
  if (target_tos & (1ULL << from))
    return 0;
  unsigned long long seen_squares = (1ULL << from);
  unsigned int next_num_steps = 1;
  unsigned int num_in_frontier = 1;
  unsigned int cur_frontier = 0;
  unsigned char frontier[2][nr_squares_on_board - 1];
  frontier[0][0] = (unsigned char) from;
  get_legal_moves_function_ptr const move_generating_function_ptr = mg->move_generating_function;
  do {
    unsigned int next_num_in_frontier = 0;
    for (unsigned int viaIndex = 0; viaIndex < num_in_frontier; ++viaIndex)
    {
      unsigned char new_destinations[nr_squares_on_board];
      unsigned int const num_new_destinations = (*move_generating_function_ptr)(mg, new_destinations, frontier[cur_frontier][viaIndex]);
      for (unsigned int d = 0; d < num_new_destinations; ++d)
      {
        unsigned char const cur_dest = new_destinations[d];
        if (!(seen_squares & (1ULL << cur_dest)))
        {
          if (target_tos & (1ULL << cur_dest))
            return next_num_steps;
          seen_squares |= (1ULL << cur_dest);
          frontier[cur_frontier ^ 1][next_num_in_frontier++] = cur_dest;
        }
      }
    }
    ++next_num_steps;
    cur_frontier ^= 1;
    num_in_frontier = next_num_in_frontier;
  } while (num_in_frontier);
  return (maxply + 1);
}

unsigned int series_h_distance_to_target(CastlingLegality const cl, EnPassantLegality const ep)
{
  enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
  };

  if (!maybe_series_mover)
    return 0;

  if (castle_kingside)
  {
    if (!(testcastling && TSTCASTLINGFLAGMASK(White, (rh_cancastle & castling_flags_no_castling))))
      return (maxply + 1);
  }
  else if (castle_queenside)
  {
    if (!(testcastling && TSTCASTLINGFLAGMASK(White, (ra_cancastle & castling_flags_no_castling))))
      return (maxply + 1);
  }

  // Convert the positions to more convenient forms.
  piece_on_square initial[nr_squares_on_board];
  piece_on_square final[nr_squares_on_board + 1];
  square wKPosition = nr_squares_on_board;
  square bKPosition = nr_squares_on_board;
  square cur_square_of_piece[nr_squares_on_board + 1];
  unsigned long long pawn_locations = 0;
  unsigned long long knight_locations = 0;
  unsigned long long bishop_locations = 0;
  unsigned long long rook_locations = 0;
  unsigned long long queen_locations = 0;
  unsigned long long white_pawn_locations = 0;
  unsigned long long occupied_squares = 0;
  for (int index = 0; index < (nr_squares_on_board + 1); ++index)
    cur_square_of_piece[index] = nr_squares_on_board;
  for (int index = a1; index <= h8; ++index)
  {
    square const cur_square = boardnum[index];
    piece_walk_type p = get_walk_of_piece_on_square(cur_square);
    if (p == Invalid)
      p = nr_piece_walks;
    initial[index].piece = p;
    if (p == Empty)
    {
      initial[index].color = no_side;
      initial[index].orig_square = nr_squares_on_board;
    }
    else
    {
      Flags const flags = being_solved.spec[cur_square];
      unsigned long long const cur_bit = (1ULL << index);
      occupied_squares |= cur_bit;
      if (TSTFLAG(flags, White))
      {
        if (p == King)
          wKPosition = index;
        else if (p == Pawn)
          white_pawn_locations |= cur_bit;
        initial[index].color = White;
      }
      else
      {
        switch (p)
        {
          case Pawn:
            pawn_locations |= cur_bit;
            break;
          case Knight:
            knight_locations |= cur_bit;
            break;
          case Bishop:
            bishop_locations |= cur_bit;
            break;
          case Rook:
            rook_locations |= cur_bit;
            break;
          case Queen:
            queen_locations |= cur_bit;
            break;
          case King:
            bKPosition = index;
            break;
          default:
            ; // do nothing
        }
        initial[index].color = Black;
      }
      square const orig_square = orig_square_of_piece(flags);
      initial[index].orig_square = orig_square;
      cur_square_of_piece[orig_square] = index;
    }

    final[index] = target_before_white_move[index];
    if ((final[index].color == White) && (initial[index].color != White))
      return (maxply + 1);
  }
  /* We pretend that pieces that don't appear in the target position are moved to "square" nr_squares on board.
     If they begin as pawns, they could finish as anything. */
  final[nr_squares_on_board].piece = nr_piece_walks;
  final[nr_squares_on_board].color = Black;
  final[nr_squares_on_board].orig_square = nr_squares_on_board;

  if (white_must_capture && (white_capture_square < nr_squares_on_board) && (final[white_capture_square].piece >= nr_piece_walks))
  {
    int const orig_square = final[white_capture_square].orig_square;
    if (orig_square < nr_squares_on_board)
    {
      int const cur_piece = initial[cur_square_of_piece[orig_square]].piece;
      if (cur_piece != Pawn)
        final[white_capture_square].piece = cur_piece;
    }
  }

  // TODO: add any blockers to ensure that Black isn't in check and that White doesn't castle out of check

  move_generator knight_generator = {get_legal_knight_moves, 0xFFFFFFFFFFFFFFFFU, 0xFFFFFFFFFFFFFFFFU};

  // What squares have White's pieces guarded throughout?
  unsigned long long guarded_by_white = 0;
  for (int sq = a1; sq <= h8; ++sq)
    if (final[sq].color == White)
    {
      int const row = (sq / nr_files_on_board);
      int const col = (sq % nr_files_on_board);
      piece_walk_type const p = final[sq].piece;
      switch (final[sq].piece)
      {
        case Pawn:
          if (col)
            guarded_by_white |= (1ULL << (sq + (nr_files_on_board - 1)));
          if (col < (nr_files_on_board - 1))
            guarded_by_white |= (1ULL << (sq + (nr_files_on_board + 1)));
          break;
        case Knight:
          {
            unsigned char knight_dest[8];
            unsigned int const num_knight_moves = get_legal_knight_moves(&knight_generator, knight_dest, sq);
            for (unsigned int i = 0; i < num_knight_moves; ++i)
              guarded_by_white |= (1ULL << knight_dest[i]);
          }
          break;
        case Queen:
        case King:
        case Bishop:
          if (row)
          {
            if (col)
              guarded_by_white |= (1ULL << (sq - (nr_files_on_board + 1)));
            if (col < (nr_files_on_board - 1))
              guarded_by_white |= (1ULL << (sq - (nr_files_on_board - 1)));
          }
          if (row < (nr_rows_on_board - 1))
          {
            if (col)
              guarded_by_white |= (1ULL << (sq + (nr_files_on_board - 1)));
            if (col < 7)
              guarded_by_white |= (1ULL << (sq + (nr_files_on_board + 1)));
          }
          if (p == Bishop)
            break;
          // intentional fall-through
        case Rook:
          if (row)
            guarded_by_white |= (1ULL << (sq - nr_files_on_board));
          if (row < (nr_rows_on_board - 1))
            guarded_by_white |= (1ULL << (sq + nr_files_on_board));
          if (col)
            guarded_by_white |= (1ULL << (sq - 1));
          if (col < (nr_files_on_board - 1))
            guarded_by_white |= (1ULL << (sq + 1));
          break;
        case Dummy:
          break;
        default:
          assert(0);
          break;
      }
    }

  // What squares must attack the wK?
  unsigned long long pawn_checks_white_king = 0;
  unsigned long long knight_checks_white_king = 0;
  unsigned long long bishop_checks_white_king = 0;
  unsigned long long rook_checks_white_king = 0;
  unsigned long long queen_checks_white_king = 0;
  if (wKPosition != nr_squares_on_board)
  {
    int const wKRow = (wKPosition / nr_files_on_board);
    int const wKCol = (wKPosition % nr_files_on_board);
    if (wKRow)
    {
      rook_checks_white_king |= (1ULL << (wKPosition - nr_files_on_board));
      if (wKCol)
      {
        bishop_checks_white_king |= (1ULL << (wKPosition - (nr_files_on_board + 1)));
        if (wKCol > 1)
          knight_checks_white_king |= (1ULL << (wKPosition - (nr_files_on_board + 2)));
      }
      if (wKCol < (nr_files_on_board - 1))
      {
        bishop_checks_white_king |= (1ULL << (wKPosition - (nr_files_on_board - 1)));
        if (wKCol < (nr_files_on_board - 2))
          knight_checks_white_king |= (1ULL << (wKPosition - (nr_files_on_board - 2)));
      }
      if (wKRow > 1)
      {
        if (wKCol)
          knight_checks_white_king |= (1ULL << (wKPosition - ((2 * nr_files_on_board) + 1)));
        if (wKCol < (nr_files_on_board - 1))
          knight_checks_white_king |= (1ULL << (wKPosition - ((2 * nr_files_on_board) - 1)));
      }
    }
    if (wKRow < (nr_rows_on_board - 1))
    {
      rook_checks_white_king |= (1ULL << (wKPosition + nr_files_on_board));
      if (wKCol)
      {
        pawn_checks_white_king |= (1ULL << (wKPosition + (nr_files_on_board - 1)));
        if (wKCol > 1)
          knight_checks_white_king |= (1ULL << (wKPosition + (nr_files_on_board - 2)));
      }
      if (wKCol < (nr_files_on_board - 1))
      {
        pawn_checks_white_king |= (1ULL << (wKPosition + (nr_files_on_board + 1)));
        if (wKCol < (nr_files_on_board - 2))
          knight_checks_white_king |= (1ULL << (wKPosition + (nr_files_on_board + 2)));
      }
      if (wKRow < (nr_rows_on_board - 2))
      {
        if (wKCol)
          knight_checks_white_king |= (1ULL << (wKPosition + ((2 * nr_files_on_board) - 1)));
        if (wKCol < (nr_files_on_board - 1))
          knight_checks_white_king |= (1ULL << (wKPosition + ((2 * nr_files_on_board) + 1)));
      }
    }
    if (wKCol)
    {
      rook_checks_white_king |= (1ULL << (wKPosition - 1));
    }
    if (wKCol < (nr_files_on_board - 1))
    {
      rook_checks_white_king |= (1ULL << (wKPosition + 1));
    }
    bishop_checks_white_king |= pawn_checks_white_king;
    queen_checks_white_king = (bishop_checks_white_king | rook_checks_white_king);
  }

  // Determine which pieces never moved.
  unsigned long long vacatable_squares = 0xFFFFFFFFFFFFFFFFU;
  unsigned long long capturable_pieces = 0;
  unsigned long long squares_occupied_in_target = 0;
  for (int index = a1; index <= h8; ++index)
  {
    if (final[index].orig_square < nr_squares_on_board)
      squares_occupied_in_target |= (1ULL << index);
    switch(initial[index].color)
    {
      case White:
        if (final[index].color != White)
        {
          capturable_pieces |= (1ULL << index);
          break;
        }
        // intentional fall-through
      case Black:
        vacatable_squares ^= (1ULL << index);
        break;
      default:
        ; // do nothing
    }
  }

  pawn_move_generator pawn_generator;
  pawn_generator.rg.mg.move_generating_function = get_legal_black_pawn_moves;
  pawn_generator.occupied_squares = occupied_squares;
  pawn_generator.capturable_pieces = capturable_pieces;
  pawn_generator.white_pawn_locations = white_pawn_locations;
  pawn_generator.ep = ep;
  pawn_generator.original_problem_position = (MovesLeft[Black] == problem_orig_num_moves);

  rider_move_generator bishop_generator;
  bishop_generator.mg.move_generating_function = get_legal_bishop_moves;

  rider_move_generator rook_generator;
  rook_generator.mg.move_generating_function = get_legal_rook_moves;

  rider_move_generator queen_generator;
  queen_generator.mg.move_generating_function = get_legal_queen_moves;

  move_generator king_generator;
  king_generator.move_generating_function = get_legal_king_moves;

  move_generator dummy_generator;
  dummy_generator.move_generating_function = get_legal_dummy_moves;

  move_generator unknown_generator;
  unknown_generator.move_generating_function = get_legal_unknown_moves;

  unsigned long long prev_vacatable_squares;
  do {
    prev_vacatable_squares = vacatable_squares;
    for (int index = a1; index <= h8; ++index)
      if ((initial[index].color == Black) && !(vacatable_squares & (1ULL << index)))
        switch (initial[index].piece)
        {
          case Pawn:
            {
              const square final_square = final_square_of_piece[initial[index].orig_square];
              if ((final_square < nr_squares_on_board) && (final[final_square].piece == Pawn))
              {
                if (final_square == index)
                  continue;
                int const cur_row = (index / nr_files_on_board);
                int const final_row = (final_square / nr_files_on_board);
                if (cur_row < final_row)
                  return (maxply + 1);
                int const cur_col = (index % nr_files_on_board);
                int const final_col = (final_square % nr_files_on_board);
                if ((cur_row - final_row) < abs(cur_col - final_col))
                  return (maxply + 1);
              }
              unsigned char tmp_dest[4];
              pawn_generator.rg.mg.possible_froms = ((pawn_locations | vacatable_squares) & 0x00FFFFFFFFFFFF00U & ~pawn_checks_white_king);
              pawn_generator.rg.mg.possible_tos = (vacatable_squares & 0x0000FFFFFFFFFFFFU & ~pawn_checks_white_king);
              // TODO: remove any holes from pawn_generator.rg.mg.possible_tos
              pawn_generator.rg.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
              pawn_generator.orig_square = index;
              if (get_legal_black_pawn_moves(&pawn_generator.rg.mg, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case Knight:
            {
              unsigned char tmp_dest[8];
              knight_generator.possible_froms = ((knight_locations | vacatable_squares) & ~knight_checks_white_king);
              knight_generator.possible_tos = (vacatable_squares & ~knight_checks_white_king);
              // TODO: remove any holes from knight_generator.possible_tos
              if (get_legal_knight_moves(&knight_generator, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case Bishop:
            {
              unsigned char tmp_dest[2 * (((nr_files_on_board < nr_rows_on_board) ? nr_files_on_board : nr_rows_on_board) - 1)];
              bishop_generator.mg.possible_froms = ((bishop_locations | vacatable_squares) & ~bishop_checks_white_king);
              bishop_generator.mg.possible_tos = (vacatable_squares & ~bishop_checks_white_king);
              // TODO: remove any holes from bishop_generator.mg.possible_tos
              bishop_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
              if (get_legal_bishop_moves(&bishop_generator.mg, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case Rook:
            {
              unsigned char tmp_dest[(nr_files_on_board - 1) + (nr_rows_on_board - 1)];
              rook_generator.mg.possible_froms = ((rook_locations | vacatable_squares) & ~rook_checks_white_king);
              rook_generator.mg.possible_tos = (vacatable_squares & ~rook_checks_white_king);
              // TODO: remove any holes from rook_generator.mg.possible_tos
              rook_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
              if (get_legal_rook_moves(&rook_generator.mg, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case Queen:
            {
              unsigned char tmp_dest[(2 * (((nr_files_on_board < nr_rows_on_board) ? nr_files_on_board : nr_rows_on_board) - 1)) + ((nr_files_on_board - 1) + (nr_rows_on_board - 1))];
              queen_generator.mg.possible_froms = ((queen_locations | vacatable_squares) & ~queen_checks_white_king);
              queen_generator.mg.possible_tos = (vacatable_squares & ~queen_checks_white_king);
              // TODO: remove any holes from queen_generator.mg.possible_tos
              queen_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
              if (get_legal_queen_moves(&queen_generator.mg, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case King:
            {
              unsigned char tmp_dest[8];
              king_generator.possible_froms = ((1ULL << bKPosition) | vacatable_squares);
              king_generator.possible_tos = (vacatable_squares & ~guarded_by_white);
              // TODO: remove any holes from king_generator.possible_tos
              if (get_legal_king_moves(&king_generator, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
            break;
          case Dummy:
            break;
          default:
            if (initial[index].piece < nr_piece_walks)
            {
              assert(0); // We have a piece we don't recognize, so we should figure it out.
              unsigned char tmp_dest[nr_squares_on_board];
              unknown_generator.possible_froms = vacatable_squares;
              unknown_generator.possible_tos = vacatable_squares;
              // TODO: remove any holes from unknown_generator.mg.possible_tos
              if (get_legal_unknown_moves(&unknown_generator, tmp_dest, index))
                vacatable_squares |= (1ULL << index);
            }
        }
  } while (prev_vacatable_squares != vacatable_squares);

  pawn_generator.rg.mg.possible_froms = ((pawn_locations | vacatable_squares) & 0x00FFFFFFFFFFFF00U & ~pawn_checks_white_king);
  pawn_generator.rg.mg.possible_tos = (vacatable_squares & 0x0000FFFFFFFFFFFFU);
  // TODO: remove any holes from pawn_generator.rg.mg.possible_tos
  pawn_generator.rg.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
  knight_generator.possible_froms = ((knight_locations | vacatable_squares) & ~knight_checks_white_king);
  knight_generator.possible_tos = vacatable_squares;
  // TODO: remove any holes from knight_generator.possible_tos
  bishop_generator.mg.possible_froms = ((bishop_locations | vacatable_squares) & ~bishop_checks_white_king);
  bishop_generator.mg.possible_tos = vacatable_squares;
  // TODO: remove any holes from bishop_generator.mg.possible_tos
  bishop_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
  rook_generator.mg.possible_froms = ((rook_locations | vacatable_squares) & ~rook_checks_white_king);
  rook_generator.mg.possible_tos = vacatable_squares;
  // TODO: remove any holes from rook_generator.mg.possible_tos
  rook_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
  queen_generator.mg.possible_froms = ((queen_locations | vacatable_squares) & ~queen_checks_white_king);
  queen_generator.mg.possible_tos = vacatable_squares;
  // TODO: remove any holes from queen_generator.mg.possible_tos
  queen_generator.obstructions = (vacatable_squares ^ 0xFFFFFFFFFFFFFFFFU);
  if (bKPosition < nr_squares_on_board)
    king_generator.possible_froms = ((1ULL << bKPosition) | vacatable_squares);
  else
    king_generator.possible_froms = vacatable_squares;
  king_generator.possible_tos = (vacatable_squares & ~guarded_by_white);
  // TODO: remove any holes from king_generator.possible_tos
  dummy_generator.possible_froms = vacatable_squares;
  dummy_generator.possible_tos = vacatable_squares;
  // TODO: remove any holes from unknown_generator.mg.possible_tos
  unknown_generator.possible_froms = vacatable_squares;
  unknown_generator.possible_tos = vacatable_squares;

  /* TODO: Pieces (but not pawns) already on their final squares may need to move out of the way to let other pieces past.
           Determine how that affects the move estimates. */

  // Could pieces have reached their final squares and, if so, how many moves would it take?
  unsigned int min_num_moves = 0;

  // Consider the possibility that Black saved move(s) by castling.
  unsigned long long const kingside_castling_squares_empty = ((1ULL << f8) | (1ULL << g8));
  unsigned long long const kingside_castling_squares_not_attacked = ((1ULL << e8) | (1ULL << f8) | (1ULL << g8));
  boolean kingside_castling_possible = false;
  if ((cl & CASTLING_KINGSIDE) &&
      !((kingside_castling_squares_empty & ~vacatable_squares) ||
        (kingside_castling_squares_not_attacked & guarded_by_white)))
    if ((cl == CASTLING_CONSEQUENT) ||
        ((bKPosition == e8) &&
         (initial[e8].orig_square == e8) &&
         (initial[h8].orig_square == h8) &&
         (initial[h8].piece == Rook) &&
         (initial[h8].color == Black)))
      kingside_castling_possible = true;
  unsigned long long const queenside_castling_squares_empty = ((1ULL << b8) | (1ULL << c8) | (1ULL << d8));
  unsigned long long const queenside_castling_squares_not_attacked = ((1ULL << c8) | (1ULL << d8) | (1ULL << e8));
  boolean queenside_castling_possible = false;
  if ((cl & CASTLING_QUEENSIDE) &&
      !((queenside_castling_squares_empty & ~vacatable_squares) ||
        (queenside_castling_squares_not_attacked & guarded_by_white)))
    if ((cl == CASTLING_CONSEQUENT) ||
        ((bKPosition == e8) &&
         (initial[e8].orig_square == e8) &&
         (initial[a8].orig_square == a8) &&
         (initial[a8].piece == Rook) &&
         (initial[a8].color == Black)))
      queenside_castling_possible = true;
  unsigned int king_moves_saved_by_castling_kingside = 0;
  unsigned int king_moves_saved_by_castling_queenside = 0;
  unsigned int min_extra_rook_moves_to_castle_kingside = (maxply + 1);
  unsigned int min_extra_rook_moves_to_castle_queenside = (maxply + 1);
  unsigned int min_extra_to_reach_capture_square = (maxply + 1);
  for (int from_square = a1; from_square <= h8; ++from_square)
    if ((initial[from_square].color == Black) &&
        (initial[from_square].piece < nr_piece_walks))
    {
      int const orig_piece = initial[from_square].piece;
      if (orig_piece == Pawn)
        pawn_generator.orig_square = from_square;
      int const to_square = final_square_of_piece[initial[from_square].orig_square];
      unsigned long long to_square_mask;
      int final_piece;
      if (orig_piece == Pawn)
        final_piece = final[to_square].piece;
      else
        final_piece = orig_piece;
      if (to_square < nr_squares_on_board)
        to_square_mask = (1ULL << to_square);
      else
        to_square_mask = ((squares_occupied_in_target | square_must_remain_open) ^ 0xFFFFFFFFFFFFFFFFU);
      move_generator const * mg;
      switch (final_piece)
      {
        case Pawn:
          mg = &pawn_generator.rg.mg;
          break;
        case Knight:
          mg = &knight_generator;
          break;
        case Bishop:
          mg = &bishop_generator.mg;
          break;
        case Rook:
          mg = &rook_generator.mg;
          break;
        case Queen:
          mg = &queen_generator.mg;
          break;
        case King:
          mg = &king_generator;
          break;
        case Dummy:
          mg = &dummy_generator;
          break;
        default:
          mg = &unknown_generator;
      }
      unsigned int prev_cur_moves;
      for (unsigned int aiming_for_capture_square = 0; aiming_for_capture_square < 2; ++aiming_for_capture_square)
      {
        unsigned int cur_moves_needed;
        if (final_piece == nr_piece_walks)
        {
          cur_moves_needed = get_length_of_shortest_path(&pawn_generator.rg.mg, from_square, to_square_mask);
          if (cur_moves_needed > maxply)
          {
            unsigned int min_moves_to_castle_kingside = (maxply + 1);
            unsigned int min_moves_to_castle_queenside = (maxply + 1);
            for (int promote = a1; promote <= h1; ++promote)
            {
              unsigned int const pawn_moves_to_promote = get_length_of_shortest_path(&pawn_generator.rg.mg, from_square, (1ULL << promote));
              if (pawn_moves_to_promote <= maxply)
              {
                unsigned int moves_after_promotion = get_length_of_shortest_path(&queen_generator.mg, promote, to_square_mask);
                unsigned int tmp_min_moves = get_length_of_shortest_path(&rook_generator.mg, promote, to_square_mask);
                if (tmp_min_moves < moves_after_promotion)
                  moves_after_promotion = tmp_min_moves;
                tmp_min_moves = get_length_of_shortest_path(&bishop_generator.mg, promote, to_square_mask);
                if (tmp_min_moves < moves_after_promotion)
                  moves_after_promotion = tmp_min_moves;
                tmp_min_moves = get_length_of_shortest_path(&knight_generator, promote, to_square_mask);
                if (tmp_min_moves < moves_after_promotion)
                  moves_after_promotion = tmp_min_moves;
                moves_after_promotion += pawn_moves_to_promote;
                if (moves_after_promotion < cur_moves_needed)
                  cur_moves_needed = moves_after_promotion;
                if (cl == CASTLING_CONSEQUENT)
                {
                  if (kingside_castling_possible)
                  {
                    tmp_min_moves = (pawn_moves_to_promote +
                                     get_length_of_shortest_path(&rook_generator.mg, promote, (1ULL << h8)) +
                                     1U +
                                     get_length_of_shortest_path(&rook_generator.mg, f8, to_square_mask));
                    if (tmp_min_moves < min_moves_to_castle_kingside)
                      min_moves_to_castle_kingside = tmp_min_moves;
                  }
                  if (queenside_castling_possible)
                  {
                    tmp_min_moves = (pawn_moves_to_promote +
                                     get_length_of_shortest_path(&rook_generator.mg, promote, (1ULL << a8)) +
                                     1U +
                                     get_length_of_shortest_path(&rook_generator.mg, c8, to_square_mask));
                    if (tmp_min_moves < min_moves_to_castle_queenside)
                      min_moves_to_castle_queenside = tmp_min_moves;
                  }
                }
              }
            }
            if (cl == CASTLING_CONSEQUENT)
            {
              unsigned int cur_extra_rook_moves = (min_moves_to_castle_kingside -
                                                   cur_moves_needed);
              if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_kingside)
                min_extra_rook_moves_to_castle_kingside = cur_extra_rook_moves;
              cur_extra_rook_moves = (min_moves_to_castle_queenside -
                                      cur_moves_needed);
              if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_queenside)
                min_extra_rook_moves_to_castle_queenside = cur_extra_rook_moves;
            }
          }
        }
        else if (orig_piece == final_piece)
        {
          cur_moves_needed = get_length_of_shortest_path(mg, from_square, to_square_mask);
          if (orig_piece == King)
          {
            if (kingside_castling_possible || queenside_castling_possible)
            {
              unsigned int const king_moves_to_e8 = get_length_of_shortest_path(&king_generator, from_square, (1ULL << e8));
              if (king_moves_to_e8 <= maxply)
              {
                if (kingside_castling_possible)
                {
                  unsigned int const king_moves_castle_kingside = (king_moves_to_e8 +
                                                                   get_length_of_shortest_path(&king_generator, g8, to_square_mask));
                  if (king_moves_castle_kingside < cur_moves_needed)
                    king_moves_saved_by_castling_kingside = (cur_moves_needed - king_moves_castle_kingside);
                  else
                    kingside_castling_possible = false;
                }
                if (queenside_castling_possible)
                {
                  unsigned int const king_moves_castle_queenside = (king_moves_to_e8 +
                                                                    get_length_of_shortest_path(&king_generator, c8, to_square_mask));
                  if (king_moves_castle_queenside < cur_moves_needed)
                    king_moves_saved_by_castling_queenside = (cur_moves_needed - king_moves_castle_queenside);
                  else
                    queenside_castling_possible = false;
                }
              }
            }
          }
          else if (orig_piece == Rook)
          {
            if (kingside_castling_possible &&
                ((cl == CASTLING_CONSEQUENT) ||
                 ((from_square == h8) && (initial[h8].orig_square == h8))))
            {
              unsigned int const cur_extra_rook_moves = (get_length_of_shortest_path(&rook_generator.mg, from_square, (1ULL << h8)) +
                                                         1U +
                                                         get_length_of_shortest_path(&rook_generator.mg, f8, to_square_mask) -
                                                         cur_moves_needed);
              if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_kingside)
                min_extra_rook_moves_to_castle_kingside = cur_extra_rook_moves;
            }
            if (queenside_castling_possible &&
                ((cl == CASTLING_CONSEQUENT) ||
                 ((from_square == a8) && (initial[a8].orig_square == a8))))
            {
              unsigned int const cur_extra_rook_moves = (get_length_of_shortest_path(&rook_generator.mg, from_square, (1ULL << a8)) +
                                                         1U +
                                                         get_length_of_shortest_path(&rook_generator.mg, d8, to_square_mask) -
                                                         cur_moves_needed);
              if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_queenside)
                min_extra_rook_moves_to_castle_queenside = cur_extra_rook_moves;
            }
          }
        }
        else
        {
          cur_moves_needed = (maxply + 1);
          unsigned int min_moves_to_castle_kingside = (maxply + 1);
          unsigned int min_moves_to_castle_queenside = (maxply + 1);
          for (int promote = a1; promote <= h1; ++promote)
          {
            unsigned int const pawn_moves_to_promote = get_length_of_shortest_path(&pawn_generator.rg.mg, from_square, (1ULL << promote));
            unsigned int promote_and_move = (pawn_moves_to_promote +
                                             get_length_of_shortest_path(mg, promote, to_square_mask));
            if (promote_and_move < cur_moves_needed)
              cur_moves_needed = promote_and_move;
            if ((cl == CASTLING_CONSEQUENT) && (final_piece == Rook))
            {
              if (kingside_castling_possible)
              {
                promote_and_move = (pawn_moves_to_promote +
                                    get_length_of_shortest_path(&rook_generator.mg, promote, (1ULL << h8)) +
                                    1U +
                                    get_length_of_shortest_path(&rook_generator.mg, f8, to_square_mask));
                if (promote_and_move < min_moves_to_castle_kingside)
                  min_moves_to_castle_kingside = promote_and_move;
              }
              if (queenside_castling_possible)
              {
                promote_and_move = (pawn_moves_to_promote +
                                    get_length_of_shortest_path(&rook_generator.mg, promote, (1ULL << a8)) +
                                    1U +
                                    get_length_of_shortest_path(&rook_generator.mg, d8, to_square_mask));
                if (promote_and_move < min_moves_to_castle_queenside)
                  min_moves_to_castle_queenside = promote_and_move;
              }
            }
          }
          if ((cl == CASTLING_CONSEQUENT) && (final_piece == Rook))
          {
            unsigned int cur_extra_rook_moves = (min_moves_to_castle_kingside -
                                                 cur_moves_needed);
            if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_kingside)
              min_extra_rook_moves_to_castle_kingside = cur_extra_rook_moves;
            cur_extra_rook_moves = (min_moves_to_castle_queenside -
                                    cur_moves_needed);
            if (cur_extra_rook_moves < min_extra_rook_moves_to_castle_queenside)
              min_extra_rook_moves_to_castle_queenside = cur_extra_rook_moves;
          }
        }
        if (aiming_for_capture_square)
        {
          if (cur_moves_needed <= maxply)
          {
            unsigned int const cur_extra = (cur_moves_needed - prev_cur_moves);
            if (cur_extra < min_extra_to_reach_capture_square)
              min_extra_to_reach_capture_square = cur_extra;
          }
        }
        else
        {
          if (cur_moves_needed > maxply)
            return (maxply + 1);
          min_num_moves += cur_moves_needed;
          if ((to_square < nr_squares_on_board) ||
              (!white_must_capture) ||
              (white_capture_square >= nr_squares_on_board) ||
              (final[white_capture_square].orig_square < nr_squares_on_board))
            break;
          to_square_mask = (1ULL << white_capture_square);
          prev_cur_moves = cur_moves_needed;
        }
      }
    }
  // TODO: add any extra moves needed for required captures

  if (white_must_capture &&
      (white_capture_square < nr_squares_on_board) &&
      (final[white_capture_square].orig_square >= nr_squares_on_board))
  {
    if (min_extra_to_reach_capture_square > maxply)
      return (maxply + 1);
    min_num_moves += min_extra_to_reach_capture_square;
  }

  unsigned int savings_from_castling = 0;
  if (king_moves_saved_by_castling_kingside > min_extra_rook_moves_to_castle_kingside)
    savings_from_castling = (king_moves_saved_by_castling_kingside - min_extra_rook_moves_to_castle_kingside);
  if (king_moves_saved_by_castling_queenside > min_extra_rook_moves_to_castle_queenside)
  {
    unsigned int const tmp_moves = (king_moves_saved_by_castling_queenside - min_extra_rook_moves_to_castle_queenside);
    if (tmp_moves > savings_from_castling)
      savings_from_castling = tmp_moves;
  }

  if (min_num_moves > savings_from_castling)
    min_num_moves -= savings_from_castling;
  else
    min_num_moves = 0;

  return min_num_moves;
}

void solve_target_position(slice_index si)
{
#if (defined(_WIN32) && !defined(_MSC_VER))|| defined(__CYGWIN__)
  /* Windows executables generated with gcc (both cross-compiling from Linux and
   * using cygwin) appear to have an optimiser error which may cause the value
   * of the expression save_king_square[Black] (but not the underlying memory!!)
   * to be modified from here to the end of the function (where
   * the value of being_solved.king_square[Black] is to be restored).
   *
   * This error doesn't manifest itself if save_king_square is volatile.
   *
   * Cf. bug report 3489394, which gives this problem as an example:
   * AnfangProblem
   * Steine Weiss  kh1 lh3 bh2
   * Steine Schwarz  ka2 dh8 ta1h6 la5f1 sg7a8 bc2e2f2b3c3a4h4h5
   * Forderung H=8
   * Option Intelligent
   * EndeProblem
   */
  volatile
#endif
  square const save_king_square[nr_sides] = { being_solved.king_square[White],
                                              being_solved.king_square[Black] };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      target_position[id].diagram_square = initsquare;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
    {
      piece_walk_type const type = get_walk_of_piece_on_square(*bnp);
      if (type!=Empty && type!=Invalid)
      {
        Flags const flags = being_solved.spec[*bnp];
        PieceIdType const id = GetPieceId(flags);
        target_position[id].type = type;
        target_position[id].flags = flags;
        target_position[id].diagram_square = *bnp;
#if defined(DETAILS)
        target_position[id].usage = find_piece_usage(id);
#endif
      }
    }
  }

  /* solve the problem */
  ResetPosition(&initial_position);
  if (get_target_before_white_move(&initial_position))
  {
    unsigned int const min_num_moves = series_h_distance_to_target(CASTLING_EITHER, EP_ON_FIRST_MOVE);
    if (min_num_moves <= MovesLeft[Black])
    {

#if defined(DETAILS)
      TraceText("target position:\n");
      trace_target_position(target_position,CapturesLeft[1]);
#endif

#ifdef JUST_OUTPUT_TARGET_POSITIONS
      output_target_position();
#else
      pipe_solve_delegate(si);

      if (solve_result<=MOVE_HAS_SOLVED_LENGTH())
        solutions_found = true;
#endif

    }
  }

  /* reset the old mating position */
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
      if (!is_square_blocked(*bnp))
        empty_square(*bnp);
  }

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      if (target_position[id].diagram_square != initsquare)
        occupy_square(target_position[id].diagram_square,
                      target_position[id].type,
                      target_position[id].flags);
  }

  {
    piece_walk_type p;

    being_solved.number_of_pieces[White][King] = 1;
    being_solved.number_of_pieces[Black][King] = 1;

    for (p = King+1; p<=Bishop; ++p)
    {
      being_solved.number_of_pieces[White][p] = 2;
      being_solved.number_of_pieces[Black][p] = 2;
    }
  }

  being_solved.king_square[White] = save_king_square[White];
  being_solved.king_square[Black] = save_king_square[Black];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void GenerateBlackKing(slice_index si)
{
  Flags const king_flags = black[index_of_king].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(black[index_of_king].type==King);

  intelligent_init_reservations(MovesLeft[White],MovesLeft[Black],
                                MaxPiece[White],MaxPiece[Black]-1);

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (is_square_empty(*bnp) /* *bnp isn't a hole*/
        && intelligent_reserve_black_king_moves_from_to(black[index_of_king].diagram_square,
                                                        *bnp))
    {

      {
        square s;
        for (s = 0; s!=maxsquare+4; ++s)
        {
          if (nr_reasons_for_staying_empty[s]>0)
            WriteSquare(&output_plaintext_engine,stdout,s);
          assert(nr_reasons_for_staying_empty[s]==0);
        }
      }

      occupy_square(*bnp,King,king_flags);
      being_solved.king_square[Black] = *bnp;
      black[index_of_king].usage = piece_is_king;

      init_guard_dirs(*bnp);

      if (goal_to_be_reached==goal_mate)
      {
        intelligent_mate_generate_checking_moves(si);
        intelligent_mate_generate_doublechecking_moves(si);
      }
      else
        pipe_solve_delegate(si);

      empty_square(*bnp);

      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void IntelligentRegulargoal_types(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[Black]!=initsquare)
  {
    testcastling = (   TSTCASTLINGFLAGMASK(White,q_castling)==q_castling
                    || TSTCASTLINGFLAGMASK(White,k_castling)==k_castling
                    || TSTCASTLINGFLAGMASK(Black,q_castling)==q_castling
                    || TSTCASTLINGFLAGMASK(Black,k_castling)==k_castling);

    assert(where_to_start_placing_black_pieces==boardnum);

    MaxPiece[Black] = 0;
    MaxPiece[White] = 0;

    black[index_of_king].type= get_walk_of_piece_on_square(being_solved.king_square[Black]);
    black[index_of_king].flags= being_solved.spec[being_solved.king_square[Black]];
    black[index_of_king].diagram_square= being_solved.king_square[Black];
    PieceId2index[GetPieceId(being_solved.spec[being_solved.king_square[Black]])] = index_of_king;
    ++MaxPiece[Black];

    if (being_solved.king_square[White]==initsquare)
      white[index_of_king].usage = piece_is_missing;
    else
    {
      white[index_of_king].usage = piece_is_unused;
      white[index_of_king].type = get_walk_of_piece_on_square(being_solved.king_square[White]);
      white[index_of_king].flags = being_solved.spec[being_solved.king_square[White]];
      white[index_of_king].diagram_square = being_solved.king_square[White];
      PieceId2index[GetPieceId(being_solved.spec[being_solved.king_square[White]])] = index_of_king;
      assert(white[index_of_king].type==King);
    }

    ++MaxPiece[White];

    {
      square const *bnp;

      nextply(White);

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (being_solved.king_square[White]!=*bnp && TSTFLAG(being_solved.spec[*bnp],White))
        {
          white[MaxPiece[White]].type = get_walk_of_piece_on_square(*bnp);
          white[MaxPiece[White]].flags = being_solved.spec[*bnp];
          white[MaxPiece[White]].diagram_square = *bnp;
          white[MaxPiece[White]].usage = piece_is_unused;
          if (get_walk_of_piece_on_square(*bnp)==Pawn)
            moves_to_white_prom[MaxPiece[White]] = intelligent_count_moves_to_white_promotion(*bnp);
          PieceId2index[GetPieceId(being_solved.spec[*bnp])] = MaxPiece[White];
          ++MaxPiece[White];
        }

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (being_solved.king_square[Black]!=*bnp && TSTFLAG(being_solved.spec[*bnp],Black))
        {
          black[MaxPiece[Black]].type = get_walk_of_piece_on_square(*bnp);
          black[MaxPiece[Black]].flags = being_solved.spec[*bnp];
          black[MaxPiece[Black]].diagram_square = *bnp;
          black[MaxPiece[Black]].usage = piece_is_unused;
          PieceId2index[GetPieceId(being_solved.spec[*bnp])] = MaxPiece[Black];
          ++MaxPiece[Black];
        }

      finply();
    }

    StorePosition(&initial_position);

    /* clear board */
    {
      square const *bnp;
      for (bnp= boardnum; *bnp!=initsquare; ++bnp)
        if (!is_square_blocked(*bnp))
          empty_square(*bnp);
    }

    {
      piece_walk_type p;
      for (p = King; p<=Bishop; ++p)
      {
        being_solved.number_of_pieces[White][p] = 2;
        being_solved.number_of_pieces[Black][p] = 2;
      }
    }

    /* generate final positions */
    GenerateBlackKing(si);

    ResetPosition(&initial_position);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void goal_to_be_reached_goal(slice_index si,
                                    stip_structure_traversal *st)
{
  goal_type * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*goal==no_goal);
  *goal = SLICE_U(si).goal_handler.goal.type;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the variable holding the goal to be reached
 */
static goal_type determine_goal_to_be_reached(slice_index si)
{
  stip_structure_traversal st;
  goal_type result = no_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &goal_to_be_reached_goal);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STAnd,
                                           &stip_traverse_structure_binary_operand1);
  stip_traverse_structure(si,&st);

  TraceValue("%u",goal_to_be_reached);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise a STGoalReachableGuardFilter slice
 * @return identifier of allocated slice
 */
static slice_index alloc_goalreachable_guard_filter(goal_type goal)
{
  slice_index result;
  slice_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
      type = STGoalReachableGuardFilterMate;
      break;

    case goal_stale:
      type = STGoalReachableGuardFilterStalemate;
      break;

    case goal_proofgame:
    case goal_atob:
      type = proof_make_goal_reachable_type();
      break;

    default:
      assert(0);
      type = no_slice_type;
      break;
  }

  if (type!=no_slice_type)
    result = alloc_pipe(type);
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
void goalreachable_guards_inserter_help_move(slice_index si,
                                             stip_structure_traversal *st)
{
  goal_type const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_goalreachable_guard_filter(*goal);
    if (prototype!=no_slice)
      help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void
goalreachable_guards_duplicate_avoider_inserter(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (SLICE_U(si).goal_handler.goal.type==goal_mate
      || SLICE_U(si).goal_handler.goal.type==goal_stale)
  {
    slice_index const prototypes[] = {
        alloc_pipe(STIntelligentDuplicateAvoider),
        alloc_pipe(STIntelligentSolutionRememberer)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    help_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goalreachable_guards_inserters[] =
{
  { STReadyForHelpMove,  &goalreachable_guards_inserter_help_move         },
  { STGoalReachedTester, &goalreachable_guards_duplicate_avoider_inserter },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe           }
};

enum
{
  nr_goalreachable_guards_inserters = (sizeof goalreachable_guards_inserters
                                       / sizeof goalreachable_guards_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void insert_goalreachable_guards(slice_index si, goal_type goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(goal!=no_goal);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STAnd,
                                           &stip_traverse_structure_binary_operand1);
  stip_structure_traversal_override(&st,
                                    goalreachable_guards_inserters,
                                    nr_goalreachable_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_goal_tester_fork(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const branch_goal_fork = branch_find_slice(STEndOfBranchGoalImmobile,
                                                           si,
                                                           stip_traversal_context_intro);
    if (branch_goal_fork==no_slice)
    {
      slice_index const branch_goal = branch_find_slice(STEndOfBranch,
                                                        si,
                                                        stip_traversal_context_intro);
      assert(branch_goal!=no_slice);
      result = find_goal_tester_fork(SLICE_NEXT2(branch_goal));
    }
    else
      result = branch_find_slice(STGoalReachedTester,
                                 SLICE_NEXT2(branch_goal_fork),
                                 stip_traversal_context_intro);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    goal_type goal;
    slice_index is_maxtime_active;
    slice_index is_maxsolutions_active;
} insertion_struct_type;

static void remember_maxtime(slice_index si, stip_structure_traversal *st)
{
  insertion_struct_type * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insertion->is_maxtime_active = si;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_maxsolutions(slice_index si, stip_structure_traversal *st)
{
  insertion_struct_type * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insertion->is_maxsolutions_active = si;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_maxtime(slice_index si, slice_index incomplete)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",incomplete);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(incomplete)==STPhaseSolvingIncomplete);

  {
    slice_index const prototypes[] = {
        alloc_maxtime_guard(incomplete),
        alloc_maxtime_guard(incomplete),
        alloc_maxtime_guard(incomplete)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_maxsolutions(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_maxsolutions_guard_slice(),
        alloc_maxsolutions_guard_slice(),
        alloc_maxsolutions_guard_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_filter_inserter(slice_index si,
                                        stip_structure_traversal *st)
{
  insertion_struct_type const * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (insertion->goal)
  {
    case goal_proofgame:
    case goal_atob:
    {
      slice_index const prototype = alloc_intelligent_proof();
      slice_insertion_insert(si,&prototype,1);
      break;
    }

    case goal_mate:
    case goal_stale:
    {
      slice_index const prototypes[] = {
          alloc_pipe(STIntelligentFilter),
          alloc_pipe(STIntelligentFlightsGuarder),
          alloc_pipe(STIntelligentFlightsBlocker),
          insertion->goal==goal_mate
          ? alloc_intelligent_mate_target_position_tester(find_goal_tester_fork(si))
          : alloc_intelligent_stalemate_target_position_tester(),
          alloc_pipe(STIntelligentTargetPositionFound)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_prototypes);

      if (insertion->is_maxtime_active!=no_slice)
        insert_maxtime(si,SLICE_NEXT2(insertion->is_maxtime_active));
      if (insertion->is_maxsolutions_active!=no_slice)
        insert_maxsolutions(si);
      break;
    }

    default:
      assert(0);
      break;
  }

  {
    slice_index const prototype = alloc_intelligent_moves_left_initialiser();
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor intelligent_filters_inserters[] =
{
  { STMaxTimeSetter,           &remember_maxtime                      },
  { STMaxSolutionsInitialiser, &remember_maxsolutions                 },
  { STHelpAdapter,             &intelligent_filter_inserter           },
  { STTemporaryHackFork,       &stip_traverse_structure_children_pipe }
};

enum
{
  nr_intelligent_filters_inserters = (sizeof intelligent_filters_inserters
                                     / sizeof intelligent_filters_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void insert_intelligent_filters(slice_index si, goal_type goal)
{
  stip_structure_traversal st;
  insertion_struct_type insertion = { goal, no_slice, no_slice };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&insertion);
  stip_structure_traversal_override(&st,
                                    intelligent_filters_inserters,
                                    nr_intelligent_filters_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* How well does the stipulation support intelligent mode?
 */
typedef enum
{
  intelligent_not_supported,
  intelligent_not_active_by_default,
  intelligent_active_by_default
} support_for_intelligent_mode;

typedef struct
{
  support_for_intelligent_mode support;
  goal_type goal;
} detector_state_type;

static
void intelligent_mode_support_detector_or(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  support_for_intelligent_mode support1;
  support_for_intelligent_mode support2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->support!=intelligent_not_supported)
  {
    /* enumerators are ordered so that the weakest support has the
     * lowest enumerator etc. */
    {
      enum
      {
        ensure_intelligent_not_supported_lt_intelligent_not_active_by_default = 1/(intelligent_not_supported<intelligent_not_active_by_default),
        ensure_intelligent_not_active_by_default_lt_intelligent_active_by_default = 1/(intelligent_not_active_by_default<intelligent_active_by_default)
      };
    }

    stip_traverse_structure_binary_operand1(si,st);
    support1 = state->support;

    stip_traverse_structure_binary_operand2(si,st);
    support2 = state->support;

    state->support = support1<support2 ? support1 : support2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_none(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_goal_tester(slice_index si,
                                                 stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  goal_type const goal = SLICE_U(si).goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->goal==no_goal)
  {
    switch (goal)
    {
      case goal_mate:
      case goal_stale:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_not_active_by_default;
        break;

      case goal_proofgame:
      case goal_atob:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_active_by_default;
        break;

      default:
        state->support = intelligent_not_supported;
        break;
    }

    state->goal = goal;
  }
  else if (state->goal!=goal)
    state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor intelligent_mode_support_detectors[] =
{
  { STAnd,               &intelligent_mode_support_none         },
  { STOr,                &intelligent_mode_support_detector_or  },
  { STNot,               &intelligent_mode_support_none         },
  { STConstraintSolver,  &intelligent_mode_support_none         },
  { STConstraintTester,  &intelligent_mode_support_none         },
  { STReadyForDefense,   &intelligent_mode_support_none         },
  { STGoalReachedTester, &intelligent_mode_support_goal_tester  },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe }
};

enum
{
  nr_intelligent_mode_support_detectors
  = (sizeof intelligent_mode_support_detectors
     / sizeof intelligent_mode_support_detectors[0])
};

/* Determine whether the stipulation supports intelligent mode, and
 * how much so
 * @param si identifies slice where to start
 * @return degree of support for ingelligent mode by the stipulation
 */
static support_for_intelligent_mode stip_supports_intelligent(slice_index si)
{
  detector_state_type state = { intelligent_not_active_by_default, no_goal };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    intelligent_mode_support_detectors,
                                    nr_intelligent_mode_support_detectors);
  stip_structure_traversal_override_single(&st,
                                           STAnd,
                                           &stip_traverse_structure_binary_operand1);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.support);
  TraceFunctionResultEnd();
  return state.support;
}

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 *         stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  goal_to_be_reached = no_goal;

  switch (stip_supports_intelligent(si))
  {
    case intelligent_not_supported:
      result = !OptFlag[intelligent];
      break;

    case intelligent_not_active_by_default:
      result = true;
      if (OptFlag[intelligent])
      {
        goal_to_be_reached = determine_goal_to_be_reached(si);
        insert_intelligent_filters(si,goal_to_be_reached);
        insert_goalreachable_guards(si,goal_to_be_reached);
        check_no_king_is_possible();
      }
      break;

    case intelligent_active_by_default:
      result = true;
      goal_to_be_reached = determine_goal_to_be_reached(si);
      insert_intelligent_filters(si,goal_to_be_reached);
      insert_goalreachable_guards(si,goal_to_be_reached);
      check_no_king_is_possible();
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
