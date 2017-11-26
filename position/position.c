#include "position/position.h"
#include "pieces/attributes/neutral/neutral.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

boolean areColorsSwapped;
boolean isBoardReflected;

position being_solved;

/* This is the InitialGameArray */
piece_walk_type const PAS[nr_squares_on_board] = {
  Rook, Knight, Bishop, Queen, King,  Bishop,  Knight,  Rook,
  Pawn, Pawn,   Pawn,   Pawn,  Pawn,  Pawn,    Pawn,    Pawn,
  Empty, Empty, Empty,  Empty, Empty, Empty,   Empty,   Empty,
  Empty, Empty, Empty,  Empty, Empty, Empty,   Empty,   Empty,
  Empty, Empty, Empty,  Empty, Empty, Empty,   Empty,   Empty,
  Empty, Empty, Empty,  Empty, Empty, Empty,   Empty,   Empty,
  Pawn, Pawn,   Pawn,   Pawn,  Pawn,  Pawn,    Pawn,    Pawn,
  Rook, Knight, Bishop, Queen, King,  Bishop,  Knight,  Rook
};

Side const PAS_sides[nr_squares_on_board] = {
    White,   White,   White,   White, White,   White,   White,   White,
    White,   White,   White,   White,   White,   White,   White,   White,
    no_side, no_side, no_side, no_side, no_side, no_side, no_side, no_side,
    no_side, no_side, no_side, no_side, no_side, no_side, no_side, no_side,
    no_side, no_side, no_side, no_side, no_side, no_side, no_side, no_side,
    no_side, no_side, no_side, no_side, no_side, no_side, no_side, no_side,
    Black,   Black,   Black,   Black,   Black,   Black,   Black,   Black,
    Black,   Black,   Black,   Black, Black,   Black,   Black,   Black
  };

void initialise_game_array(position *pos)
{
  unsigned int i;
  piece_walk_type p;
  square const *bnp;

  pos->king_square[White] = square_e1;
  pos->king_square[Black] = square_e8;

  for (p = 0; p<nr_piece_walks; ++p)
  {
    pos->number_of_pieces[White][p] = 0;
    pos->number_of_pieces[Black][p] = 0;
  }

  /* TODO avoid duplication with InitBoard()
   */
  for (i = 0; i<maxsquare+4; ++i)
  {
    pos->board[i] = Invalid;
    pos->spec[i] = BorderSpec;
  }

  /* dummy squares for various purposes -- must be empty */
  pos->board[pawn_multistep] = Empty;
  pos->board[messigny_exchange] = Empty;
  pos->board[kingside_castling] = Empty;
  pos->board[queenside_castling] = Empty;
  pos->board[retro_capture_departure] = Empty;

  for (bnp = boardnum; *bnp; bnp++)
  {
    pos->board[*bnp] = Empty;
    CLEARFL(pos->spec[*bnp]);
  }

  pos->currPieceId = NullPieceId;

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    piece_walk_type const p = PAS[i];
    square const square_i = boardnum[i];
    if (p==Empty || p==Invalid)
      pos->board[square_i] = p;
    else
    {
      Side const side = PAS_sides[i];
      pos->board[square_i] = p;
      ++pos->number_of_pieces[side][p];
      SETFLAG(pos->spec[square_i],side);
      SetPieceId(pos->spec[square_i],++pos->currPieceId);
    }
  }

  pos->number_of_imitators = 0;
  for (i = 0; i<maxinum; ++i)
    pos->isquare[i] = initsquare;

  pos->castling_rights = wh_castlings|bl_castlings;
}

/* Swap the sides of all the pieces */
void swap_sides(void)
{
  square const *bnp;
  square const save_white_king_square = being_solved.king_square[White];

  being_solved.king_square[White] = being_solved.king_square[Black]==initsquare ? initsquare : being_solved.king_square[Black];
  being_solved.king_square[Black] = save_white_king_square==initsquare ? initsquare : save_white_king_square;

  for (bnp = boardnum; *bnp; bnp++)
    if (!is_piece_neutral(being_solved.spec[*bnp])
        && !is_square_empty(*bnp)
        && !is_square_blocked(*bnp))
      piece_change_side(&being_solved.spec[*bnp]);

  {
    piece_walk_type walk;
    for (walk = Empty; walk!=nr_piece_walks; ++walk)
    {
      unsigned int const save_nr_white = being_solved.number_of_pieces[White][walk];
      being_solved.number_of_pieces[White][walk] = being_solved.number_of_pieces[Black][walk];
      being_solved.number_of_pieces[Black][walk] = save_nr_white;
    }
  }

  areColorsSwapped = !areColorsSwapped;
}

static void swap_castling_rights(void)
{
  castling_rights_type const white_castlings = being_solved.castling_rights&wh_castlings;
  castling_rights_type const black_castlings = being_solved.castling_rights&bl_castlings;
  being_solved.castling_rights = (white_castlings<<black_castling_rights_offset) | (black_castlings>>black_castling_rights_offset);
}

/* Reflect the position at the horizontal central line */
void reflect_position(void)
{
  square const *bnp;

  being_solved.king_square[White] = being_solved.king_square[White]==initsquare ? initsquare : transformSquare(being_solved.king_square[White],mirra1a8);
  being_solved.king_square[Black] = being_solved.king_square[Black]==initsquare ? initsquare : transformSquare(being_solved.king_square[Black],mirra1a8);

  for (bnp = boardnum; *bnp < (square_a1+square_h8)/2; bnp++)
  {
    square const sq_reflected = transformSquare(*bnp,mirra1a8);

    piece_walk_type const p = being_solved.board[sq_reflected];
    Flags const sp = being_solved.spec[sq_reflected];

    being_solved.board[sq_reflected] = being_solved.board[*bnp];
    being_solved.spec[sq_reflected] = being_solved.spec[*bnp];

    being_solved.board[*bnp] = p;
    being_solved.spec[*bnp] = sp;
  }

  swap_castling_rights();

  isBoardReflected = !isBoardReflected;
}

/* Change the side of some piece specs
 * @param being_solved.spec address of piece specs where to change the side
 */
void piece_change_side(Flags *spec)
{
  *spec ^= BIT(Black)|BIT(White);
}

void empty_square(square s)
{
  being_solved.board[s] = Empty;
  being_solved.spec[s] = EmptySpec;
}

void occupy_square(square s, piece_walk_type walk, Flags flags)
{
  assert(walk!=Empty);
  assert(walk!=Invalid);
  being_solved.board[s] = walk;
  being_solved.spec[s] = flags;
}

void replace_walk(square s, piece_walk_type walk)
{
  assert(walk!=Empty);
  assert(walk!=Invalid);
  being_solved.board[s] = walk;
}

void block_square(square s)
{
  assert(is_square_empty(s) || is_square_blocked(s));
  being_solved.board[s] = Invalid;
  being_solved.spec[s] = BorderSpec;
}

square find_end_of_line(square from, numvec dir)
{
  square result = from;
  do
  {
    result += dir;
  }
  while (is_square_empty(result));

  return result;
}
