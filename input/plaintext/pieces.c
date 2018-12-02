#include "input/plaintext/pieces.h"
#include "input/plaintext/geometry/square.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/message.h"
#include "output/output.h"
#include "output/latex/latex.h"
#include "position/underworld.h"
#include "position/piece_creation.h"
#include "position/piece_removal.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "conditions/circe/parachute.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

/* Identify a piece walk from the characters of its shortcut
 * @param a 1st character
 * @param b 2nd character (pass ' ' for 1 character shortcuts)
 * @return index of the shortcut in the language-dependent table of piece walk shortcuts
 *         nr_piece_walks if a and b don't represent a piece walk in the current language
 * @note the characters are treated independently from their case
 */
int GetPieNamIndex(char a, char b)
{
  char const * ch = PieceTab[2];
  int indexx;
  for (indexx = 2;
       indexx<nr_piece_walks;
       ++indexx, ch += sizeof(PieceChar))
    if (ch[0]==tolower(a) && ch[1]==tolower(b))
      break;

  return indexx;
}

typedef struct
{
    piece_walk_type walk;
    Flags spec;
} piece_addition_settings;

/* Deal with a parsed piece to be added to the current position
 * @param s where to add the piece
 * @param param other aspectes of the piece
 */
static void HandleAddedPiece(square s, void *param)
{
  piece_addition_settings * const settings = param;

  if (!is_square_empty(s))
  {
    if (nbply!=ply_twinning)
      move_effect_journal_do_circe_volcanic_remember(move_effect_reason_diagram_setup,s);
    move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,s);
  }

  move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                        s,settings->walk,
                                        settings->spec,
                                        no_side);
}

/* Parse a piece walk from its shortcut
 * @param onechar are we parsing a 1 character shortcut?
 * @param tok where to start parsing
 * @param pienam where to store the detected walk
 * @return first unparsed character in tok
 * @note assigns Empty to *pienam if tok doesn't start with a recognised piece walk shortcut
 */
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

/* Parse a piece walk whose shortcut uses up an entire token
 * @param tok where to start parsing
 * @param result where to store the detected walk
 * @return start of subsequent token if parsing was successful, tok otherwise
 * @note assigns nr_piece_walks to *result parsing was not successful
 */
char *ParsePieceWalkToken(char *tok, piece_walk_type *result)
{
  char * const save_tok = tok;
  char const hunter_separator = '/';
  char const * const hunterseppos = strchr(tok,hunter_separator);
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    piece_walk_type away;
    tok = ParseWalkShortcut(tok[1]==hunter_separator,tok,&away);

    if (away==nr_piece_walks)
    {
      *result = nr_piece_walks;
      return save_tok;
    }
    else
    {
      piece_walk_type home;

      assert(tok[0]==hunter_separator);
      ++tok;

      tok = ParseWalkShortcut(strlen(tok)==1,tok,&home);

      if (tok[0]!=0 || home==nr_piece_walks)
      {
        *result = nr_piece_walks;
        return save_tok;
      }
      else
      {
        *result = hunter_find_type(away,home);
        if (*result==nr_piece_walks)
        {
          *result = hunter_make_type(away,home);
          if (*result==nr_piece_walks)
            output_plaintext_input_error_message(HunterTypeLimitReached,max_nr_hunter_walks);
        }

        return ReadNextTokStr();
      }
    }
  }
  else
  {
    tok = ParseWalkShortcut(strlen(tok)==1,tok,result);

    if (tok[0]!=0 || *result==nr_piece_walks)
    {
      *result = nr_piece_walks;
      return save_tok;
    }
    else
      return ReadNextTokStr();
  }
}

/* Parse a piece walk whose shortcut is part of a token (e.g. Qa8)
 * @param tok where to parse from
 * @param name where to write the detected walk to
 * @return position immediately behind the walk (no white space needed between
 *         walk and squares in pieces)
 * @note assigns nr_piece_walks to *walk if tok doesn't contain a
 *       recognisable piece walk shortcut
 */
char *ParsePieceWalk(char *tok, piece_walk_type *walk)
{
  char const hunter_separator = '/';
  char const * const hunterseppos = strchr(tok,hunter_separator);
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    piece_walk_type away;
    tok = ParseWalkShortcut((hunterseppos-tok)%2==1,tok,&away);

    if (away==nr_piece_walks)
      *walk = nr_piece_walks;
    else
    {
      piece_walk_type home;
      size_t len_token;

      assert(tok[0]==hunter_separator);
      ++tok;

      len_token = strlen(tok);
      tok = ParseWalkShortcut(len_token%2==1,tok,&home);
      if (home==nr_piece_walks)
        *walk = nr_piece_walks;
      else
      {
        *walk = hunter_find_type(away,home);
        if (*walk==nr_piece_walks)
        {
          *walk = hunter_make_type(away,home);
          if (*walk==nr_piece_walks)
            output_plaintext_input_error_message(HunterTypeLimitReached,max_nr_hunter_walks);
        }
      }
    }
  }
  else
  {
    size_t const len_token = strlen(tok);
    tok = ParseWalkShortcut(len_token%2==1,tok,walk);
  }

  return tok;
}

/* Parse a sequence of piece walks and squares and add pieces to the current position
 * @param tok where to start parsing
 * @param Spec flags of the piece(s) to be added
 * @return start of first token that is not part of the sequence
 */
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

    if (walk!=nr_piece_walks)
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

/* Try to parse a piece colour
 * @param tok where to start parsing
 * @param colour_is_mandatory is a colour indication mandatory or optional?
 * @return the detected colour, 0 if none is detected
 */
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

/* Advance the input stream to the next token, then parse 0 to n non-colour piece flags.
 * @param flags where to save the detected flags
 * @return start of subsequent token
 */
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

/* Parse a sequence of pieces including flags
 * @param tok where to start parsing
 * @return start of subsequent token
 */
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
