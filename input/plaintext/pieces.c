#include "input/plaintext/pieces.h"
#include "input/plaintext/geometry/square.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/message.h"
#include "output/output.h"
#include "output/latex/latex.h"
#include "position/underworld.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "conditions/circe/parachute.h"

#include <ctype.h>
#include <string.h>

int GetPieNamIndex(char a, char b)
{
  /* We search the array PieNam, for an index, where
     it matches the two characters a and b
  */
  char const * ch = PieceTab[2];
  int indexx;
  for (indexx = 2;
       indexx<nr_piece_walks;
       ++indexx, ch += sizeof(PieceChar))
    if (ch[0]==tolower(a) && ch[1]==tolower(b))
      return indexx;

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

typedef struct
{
    piece_walk_type walk;
    Flags spec;
} piece_addition_settings;

static void HandleAddedPiece(square s, void *param)
{
  piece_addition_settings * const settings = param;

  if (!is_square_empty(s))
  {
    move_effect_journal_do_circe_volcanic_remember(move_effect_reason_diagram_setup,s);
    move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,s);
  }

  move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                        s,settings->walk,
                                        settings->spec,
                                        no_side);
}

static char *ParseWalkShortcut(boolean onechar, char *tok, piece_walk_type *pienam)
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

char *ParsePieceWalk(char *tok, piece_walk_type *walk)
{
  size_t len_token;
  char const * const hunterseppos = strchr(tok,'/');
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    piece_walk_type away;
    piece_walk_type home;
    tok = ParseWalkShortcut((hunterseppos-tok)%2==1,tok,&away);
    ++tok; /* skip separator */
    len_token = strlen(tok);
    tok = ParseWalkShortcut(len_token%2==1,tok,&home);
    *walk = hunter_find_type(away,home);
    if (*walk==Invalid)
    {
      *walk = hunter_make_type(away,home);
      if (*walk==Invalid)
        output_plaintext_input_error_message(HunterTypeLimitReached,max_nr_hunter_walks);
    }
  }
  else
  {
    len_token = strlen(tok);
    tok = ParseWalkShortcut(len_token%2==1,tok,walk);
  }

  return tok;
}

static char *ParsePieceWalkAndSquares(char *tok, Flags Spec)
{
  unsigned int nr_walks_parsed = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  while (true)
  {
    piece_walk_type walk;
    char * const save_tok = tok;

    tok = ParsePieceWalk(tok,&walk);

    if (walk>=King)
    {
      piece_addition_settings settings = { walk, Spec };

      ++nr_walks_parsed;

      if (tok[0]==0)
      {
        /* the next token must be a valid square list, e.g. B a1b2
         */
        char * const squares_tok = ReadNextTokStr();
        tok = ParseSquareList(squares_tok,&HandleAddedPiece,&settings);
        if (tok==squares_tok)
          output_plaintext_input_error_message(MissngSquareList,0);
        else if (*tok!=0)
          output_plaintext_error_message(WrongSquareList);
      }
      else
      {
        /* the remainder of the token may be
         * * a valid square list, e.g. Ba1b2
         * * the remainder of a different word e.g. Black
         */
        if (*ParseSquareList(tok,&HandleAddedPiece,&settings)!=0)
        {
          tok = save_tok;
          break;
        }
      }

      tok = ReadNextTokStr();

      /* undocumented feature: "royal" only applies to the immediately next
       * piece indication because there can be at most 1 royal piece per side
       */
      CLRFLAG(Spec,Royal);
    }
    else
    {
      if (nr_walks_parsed==0)
      {
        output_plaintext_input_error_message(WrongPieceName,0);
        tok = ReadNextTokStr();
      }
      else
        tok = save_tok;

      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
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

char *ParsePieces(char *tok)
{
  int nr_groups = 0;
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

      tok = ParsePieceWalkAndSquares(tok,PieSpFlags);

      ++nr_groups;
    }
  }

  return tok;
}
