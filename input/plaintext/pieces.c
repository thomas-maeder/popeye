#include "input/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/output.h"
#include "output/latex/latex.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "pymsg.h"

#include <string.h>

int GetPieNamIndex(char a, char b)
{
  /* We search the array PieNam, for an index, where
     it matches the two characters a and b
  */
  int indexx;
  char *ch;

  ch= PieceTab[2];
  for (indexx= 2;
       indexx<PieceCount;
       indexx++,ch+= sizeof(PieceChar))
  {
    if (*ch == a && *(ch + 1) == b) {
      return indexx;
    }
  }
  return 0;
}

char *ParseSinglePiece(char *tok, PieNam *result)
{
  switch (strlen(tok))
  {
    case 1:
      *result = GetPieNamIndex(tok[0],' ');
      return ReadNextTokStr();
      break;

    case 2:
      *result = GetPieNamIndex(tok[0],tok[1]);
      return ReadNextTokStr();
      break;

    default:
      *result = PieceCount;
      return tok;
  }
}

square SquareNum(char a, char b)
{
  if ('a'<=a && a<='h' && '1'<=b && b<='8')
    return square_a1 + (a-'a')*dir_right +(b-'1')*dir_up;
  else
    return initsquare;
}

static void signal_overwritten_square(square Square)
{
  WriteSquare(Square);
  StdChar(' ');
  Message(OverwritePiece);
}

static void echo_added_piece(Flags Spec, PieNam Name, square Square)
{
  if (LaTeXout)
    LaTeXEchoAddedPiece(Spec,Name,Square);

  if (is_square_empty(Square))
    StdChar('+');

  WriteSpec(Spec,Name, Name!=Empty);
  WritePiece(Name);
  WriteSquare(Square);
  StdChar(' ');
}

static char *ParseSquareList(char *tok,
                             PieNam Name,
                             Flags Spec,
                             piece_addition_type type)
{
  /* We interpret the tokenString as SquareList
     If we return always the next1 tokenstring
  */
  unsigned int SquareCnt = 0;

  while (true)
  {
    square const Square = SquareNum(*tok,tok[1]);
    if (tok[0]!=0 && tok[1]!=0 && Square!=initsquare)
    {
      if (!is_square_empty(Square))
      {
        if (type==piece_addition_initial)
          signal_overwritten_square(Square);

        if (Square==king_square[White])
          king_square[White] = initsquare;
        if (Square==king_square[Black])
          king_square[Black] = initsquare;
      }

      if (type==piece_addition_twinning)
        echo_added_piece(Spec,Name,Square);

      occupy_square(Square,Name,Spec);
      tok += 2;
      ++SquareCnt;
    }
    else if (SquareCnt==0)
    {
      ErrorMsg(MissngSquareList);
      tok = ReadNextTokStr();
    }
    else
    {
      if (tok[0]!=0)
        ErrorMsg(WrongSquareList);
      break;
    }
  }

  return ReadNextTokStr();
}

static char *PrsPieShortcut(boolean onechar, char *tok, PieNam *pienam)
{
  if (onechar)
  {
    *pienam= GetPieNamIndex(*tok,' ');
    tok++;
  }
  else
  {
    *pienam= GetPieNamIndex(*tok,tok[1]);
    tok+= 2;
  }

  return tok;
}

char *ParsePieceName(char *tok, PieNam *name)
{
  size_t len_token;
  char const * const hunterseppos = strchr(tok,'/');
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    PieNam away, home;
    tok = PrsPieShortcut((hunterseppos-tok)%2==1,tok,&away);
    ++tok; /* skip slash */
    len_token = strlen(tok);
    tok = PrsPieShortcut(len_token%2==1,tok,&home);
    *name = hunter_make_type(away,home);
    if (*name==Invalid)
      IoErrorMsg(HunterTypeLimitReached,maxnrhuntertypes);
  }
  else
  {
    len_token = strlen(tok);
    tok = PrsPieShortcut(len_token%2==1,tok,name);
  }

  return tok;
}

static char *ParsePieceNameAndSquares(char *tok, Flags Spec, piece_addition_type type)
{
  /* We read from tok the name of the piece */
  int     NameCnt= 0;
  char    *btok;

  while (true)
  {
    PieNam Name;
    btok = tok; /* Save it, if we want to return it */
    tok = ParsePieceName(tok,&Name);

    if (Name>=King)
    {
      if (strchr("12345678",tok[1])==0)
        break;
      /* We have read a character (pair) that is a piece name short cut, but tok
       * isn't a piecename squarelist squence.
       * E.g. tok=="Black"; we have read 'B' for Bauer or Bishop, but "lack"
       * isn't a list of squares.
       */
      NameCnt++;
      if (*tok==0)
        tok = ReadNextTokStr();
      tok = ParseSquareList(tok, Name, Spec, type);
      /* undocumented feature: "royal" only applies to the immediately next
       * piece indication because there can be at most 1 royal piece per side
       */
      CLRFLAG(Spec,Royal);
    }
    else if (NameCnt>0)
      return btok;
    else
    {
      IoErrorMsg(WrongPieceName,0);
      tok = ReadNextTokStr();
    }
  }

  return btok;
}

Flags ParseColor(char *tok, boolean color_is_mandatory)
{
  Colors const color = GetUniqIndex(nr_colors,ColorTab,tok);
  if (color==nr_colors)
  {
    if (color_is_mandatory)
      IoErrorMsg(NoColorSpec,0);
    return 0;
  }
  else if (color>nr_colors)
  {
    IoErrorMsg(PieSpecNotUniq,0);
    return 0;
  }
  else if (color==color_neutral)
    return NeutralMask;
  else
    return BIT(color);
}

char *ParsePieceFlags(Flags *flags)
{
  char *tok;

  while (true)
  {
    tok = ReadNextTokStr();

    {
      PieSpec const ps = GetUniqIndex(PieSpCount-nr_sides,PieSpTab,tok);
      if (ps==PieSpCount-nr_sides)
        break;
      else if (ps>PieSpCount-nr_sides)
        IoErrorMsg(PieSpecNotUniq,0);
      else
        SETFLAG(*flags,ps+nr_sides);
    }
  }

  return tok;
}

char *ParsePieces(piece_addition_type type)
{
  int nr_groups = 0;
  char *tok = ReadNextTokStr();
  while (true)
  {
    Flags PieSpFlags = ParseColor(tok,nr_groups==0);
    if (PieSpFlags==0)
      break;
    else
    {
      ++nr_groups;

      if (is_piece_neutral(PieSpFlags))
        SETFLAGMASK(some_pieces_flags,NeutralMask);

      {
        Flags nonColorFlags = 0;
        tok = ParsePieceFlags(&nonColorFlags);
        PieSpFlags |= nonColorFlags;
        some_pieces_flags |= nonColorFlags;
      }

      tok = ParsePieceNameAndSquares(tok,PieSpFlags,type);
    }
  }

  return tok;
}
