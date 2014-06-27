#include "input/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/message.h"
#include "output/output.h"
#include "output/latex/latex.h"
#include "position/underworld.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "conditions/circe/parachute.h"

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
       indexx<nr_piece_walks;
       indexx++,ch+= sizeof(PieceChar))
  {
    if (*ch == a && *(ch + 1) == b) {
      return indexx;
    }
  }
  return 0;
}

char *ParseSingleWalk(char *tok, piece_walk_type *result)
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
      *result = nr_piece_walks;
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

static char *ParseSquareList(char *tok,
                             piece_walk_type Name,
                             Flags Spec,
                             piece_addition_type type)
{
  /* We interpret the tokenString as SquareList
     If we return always the next1 tokenstring
  */
  unsigned int SquareCnt = 0;

  while (true)
  {
    square const Square = SquareNum(tok[0],tok[1]);
    if (tok[0]!=0 && tok[1]!=0 && Square!=initsquare)
    {
      if (!is_square_empty(Square))
      {
        if (type==piece_addition_initial)
        {
          WriteSquare(&output_plaintext_engine,stdout,Square);
          output_plaintext_message(OverwritePiece);

          underworld_make_space(nr_ghosts);
          underworld[nr_ghosts-1].walk = get_walk_of_piece_on_square(Square);
          underworld[nr_ghosts-1].flags = being_solved.spec[Square];
          underworld[nr_ghosts-1].on = Square;
        }
        else
        {
          move_effect_journal_do_circe_volcanic_remember(move_effect_reason_diagram_setup,
                                                         Square);
          move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,
                                               Square);
        }
      }

      if (type==piece_addition_twinning)
        move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                              Square,Name,Spec);
      else
        occupy_square(Square,Name,Spec);

      tok += 2;
      ++SquareCnt;
    }
    else if (SquareCnt==0)
    {
      output_plaintext_error_message(MissngSquareList);
      tok = ReadNextTokStr();
    }
    else
    {
      if (tok[0]!=0)
        output_plaintext_error_message(WrongSquareList);
      break;
    }
  }

  return ReadNextTokStr();
}

static char *PrsPieShortcut(boolean onechar, char *tok, piece_walk_type *pienam)
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

char *ParsePieceName(char *tok, piece_walk_type *name)
{
  size_t len_token;
  char const * const hunterseppos = strchr(tok,'/');
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    piece_walk_type away, home;
    tok = PrsPieShortcut((hunterseppos-tok)%2==1,tok,&away);
    ++tok; /* skip slash */
    len_token = strlen(tok);
    tok = PrsPieShortcut(len_token%2==1,tok,&home);
    *name = hunter_make_type(away,home);
    if (*name==Invalid)
      output_plaintext_input_error_message(HunterTypeLimitReached,max_nr_hunter_walks);
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
    piece_walk_type Name;
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
      output_plaintext_input_error_message(WrongPieceName,0);
      tok = ReadNextTokStr();
    }
  }

  return btok;
}

Flags ParseColour(char *tok, boolean colour_is_mandatory)
{
  Colour const colour = GetUniqIndex(nr_colours,ColourTab,tok);
  if (colour==nr_colours)
  {
    if (colour_is_mandatory)
      output_plaintext_input_error_message(NoColourSpec,0);
    return 0;
  }
  else if (colour>nr_colours)
  {
    output_plaintext_input_error_message(PieSpecNotUniq,0);
    return 0;
  }
  else if (colour==colour_neutral)
    return NeutralMask;
  else
    return BIT(colour);
}

char *ParsePieceFlags(Flags *flags)
{
  char *tok;

  while (true)
  {
    tok = ReadNextTokStr();

    {
      piece_flag_type const ps = GetUniqIndex(nr_piece_flags-nr_sides,PieSpTab,tok);
      if (ps==nr_piece_flags-nr_sides)
        break;
      else if (ps>nr_piece_flags-nr_sides)
        output_plaintext_input_error_message(PieSpecNotUniq,0);
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
    Flags PieSpFlags = ParseColour(tok,nr_groups==0);
    if (PieSpFlags==0)
      break;
    else
    {
      Flags nonCOLOURFLAGS = 0;
      tok = ParsePieceFlags(&nonCOLOURFLAGS);
      PieSpFlags |= nonCOLOURFLAGS;

      tok = ParsePieceNameAndSquares(tok,PieSpFlags,type);

      ++nr_groups;
    }
  }

  return tok;
}
