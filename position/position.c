#include "position/position.h"
#include "py.h"
#include "pieces/attributes/neutral/neutral.h"
#include "debugging/trace.h"

#include <assert.h>

#define ENUMERATION_TYPENAME Side
#define ENUMERATORS \
  ENUMERATOR(White), \
    ENUMERATOR(Black), \
                       \
    ENUMERATOR(nr_sides), \
    ASSIGNED_ENUMERATOR(no_side = nr_sides)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

echiquier e;
Flags spec[maxsquare+4];
square king_square[nr_sides];
boolean areColorsSwapped;
boolean isBoardReflected;

/* This is the InitialGameArray */
PieNam const PAS[nr_squares_on_board] = {
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
  PieNam p;
  square const *bnp;

  pos->king_square[White] = square_e1;
  pos->king_square[Black] = square_e8;

  for (p = 0; p<PieceCount; ++p)
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

  for (bnp = boardnum; *bnp; bnp++)
  {
    pos->board[*bnp] = Empty;
    CLEARFL(pos->spec[*bnp]);
  }

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    PieNam const p = PAS[i];
    square const square_i = boardnum[i];
    if (p==Empty || p==Invalid)
      pos->board[square_i] = p;
    else
    {
      Side const side = PAS_sides[i];
      pos->board[square_i] = p;
      ++pos->number_of_pieces[side][p];
      SETFLAG(pos->spec[square_i],side);
    }
  }

  pos->inum = 0;
  for (i = 0; i<maxinum; ++i)
    pos->isquare[i] = initsquare;
}

/* Swap the sides of all the pieces */
void swap_sides(void)
{
  square const *bnp;
  square const save_white_king_square = king_square[White];

  king_square[White] = king_square[Black]==initsquare ? initsquare : king_square[Black];
  king_square[Black] = save_white_king_square==initsquare ? initsquare : save_white_king_square;

  for (bnp = boardnum; *bnp; bnp++)
    if (!is_piece_neutral(spec[*bnp]) && !is_square_empty(*bnp))
      spec[*bnp]^= BIT(White)+BIT(Black);

  areColorsSwapped = !areColorsSwapped;
}

/* Reflect the position at the horizontal central line */
void reflect_position(void)
{
  square const *bnp;

  king_square[White] = king_square[White]==initsquare ? initsquare : transformSquare(king_square[White],mirra1a8);
  king_square[Black] = king_square[Black]==initsquare ? initsquare : transformSquare(king_square[Black],mirra1a8);

  for (bnp = boardnum; *bnp < (square_a1+square_h8)/2; bnp++)
  {
    square const sq_reflected = transformSquare(*bnp,mirra1a8);

    PieNam const p = e[sq_reflected];
    Flags const sp = spec[sq_reflected];

    e[sq_reflected] = e[*bnp];
    spec[sq_reflected] = spec[*bnp];

    e[*bnp] = p;
    spec[*bnp] = sp;
  }

  isBoardReflected = !isBoardReflected;
}

void empty_square(square s)
{
  e[s] = Empty;
  spec[s] = EmptySpec;
}

void occupy_square(square s, PieNam piece, Flags flags)
{
  assert(piece!=Empty);
  assert(piece!=Invalid);
  e[s] = piece;
  spec[s] = flags;
}

void replace_piece(square s, PieNam piece)
{
  assert(piece!=Empty);
  assert(piece!=Invalid);
  e[s] = piece;
}

void block_square(square s)
{
  assert(is_square_empty(s) || e[s]==Invalid);
  e[s] = Invalid;
  spec[s] = BorderSpec;
}

boolean is_square_empty(square s)
{
  return e[s]==Empty;
}

boolean is_square_blocked(square s)
{
  return e[s]==Invalid;
}

PieNam get_walk_of_piece_on_square(square s)
{
  return e[s];
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
