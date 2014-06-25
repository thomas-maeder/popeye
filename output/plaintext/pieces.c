#include "output/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/hunters.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <string.h>

boolean WriteSpec(FILE *file, Flags sp, piece_walk_type p, boolean printcolours)
{
  boolean ret = false;
  piece_flag_type spname;

  if (is_piece_neutral(sp))
  {
    fputc(tolower(ColourString[UserLanguage][colour_neutral][0]),file);
    ret = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        fputc(tolower(ColourString[UserLanguage][colour_black][0]),file);
      if (TSTFLAG(sp,Black))
        fputc(tolower(ColourString[UserLanguage][colour_white][0]),file);
    }
    else
    {
      if (TSTFLAG(sp,White))
        fputc(tolower(ColourString[UserLanguage][colour_white][0]),file);
      if (TSTFLAG(sp,Black))
        fputc(tolower(ColourString[UserLanguage][colour_black][0]),file);
    }
  }

  for (spname = nr_sides; spname<nr_piece_flags; ++spname)
    if ((spname!=Volage || !CondFlag[volage])
        && (spname!=Patrol || !CondFlag[patrouille])
        && (spname!=Beamtet || !CondFlag[beamten])
        && (spname!=Royal || !is_king(p))
        && TSTFLAG(sp, spname))
    {
      fputc(tolower(PieSpString[UserLanguage][spname-nr_sides][0]),file);
      ret = true;
    }

  return ret;
}

boolean WriteSpec1(Flags sp, piece_walk_type p, boolean printcolours)
{
  boolean ret = false;
  piece_flag_type spname;

  if (is_piece_neutral(sp))
  {
    protocol_fputc(tolower(ColourString[UserLanguage][colour_neutral][0]));
    ret = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        protocol_fputc(tolower(ColourString[UserLanguage][colour_black][0]));
      if (TSTFLAG(sp,Black))
        protocol_fputc(tolower(ColourString[UserLanguage][colour_white][0]));
    }
    else
    {
      if (TSTFLAG(sp,White))
        protocol_fputc(tolower(ColourString[UserLanguage][colour_white][0]));
      if (TSTFLAG(sp,Black))
        protocol_fputc(tolower(ColourString[UserLanguage][colour_black][0]));
    }
  }

  for (spname = nr_sides; spname<nr_piece_flags; ++spname)
    if ((spname!=Volage || !CondFlag[volage])
        && (spname!=Patrol || !CondFlag[patrouille])
        && (spname!=Beamtet || !CondFlag[beamten])
        && (spname!=Royal || !is_king(p))
        && TSTFLAG(sp, spname))
    {
      protocol_fputc(tolower(PieSpString[UserLanguage][spname-nr_sides][0]));
      ret = true;
    }

  return ret;
}

void WritePiece(FILE *file, piece_walk_type p)
{
  if (p<Hunter0 || p>= (Hunter0 + max_nr_hunter_walks))
  {
    char const p1 = PieceTab[p][1];
    fputc(toupper(PieceTab[p][0]),file);
    if (p1!=' ')
      fputc(toupper(p1),file);
  }
  else
  {
    unsigned int const i = p-Hunter0;
    assert(i<max_nr_hunter_walks);
    WritePiece(file,huntertypes[i].away);
    fputc('/',file);
    WritePiece(file,huntertypes[i].home);
  }
}

void WritePiece1(piece_walk_type p)
{
  if (p<Hunter0 || p>= (Hunter0 + max_nr_hunter_walks))
  {
    char const p1 = PieceTab[p][1];
    protocol_fputc(toupper(PieceTab[p][0]));
    if (p1!=' ')
      protocol_fputc(toupper(p1));
  }
  else
  {
    unsigned int const i = p-Hunter0;
    assert(i<max_nr_hunter_walks);
    WritePiece1(huntertypes[i].away);
    protocol_fputc('/',stdout);
    WritePiece1(huntertypes[i].home);
  }
}

void WriteSquare1(square i)
{
  protocol_fputc('a',stdout - nr_files_on_board + i%onerow);
  if (isBoardReflected)
    protocol_fputc('8',stdout + nr_rows_on_board - i/onerow);
  else
    protocol_fputc('1',stdout - nr_rows_on_board + i/onerow);
}

void WriteSquare(FILE *file, square i)
{
  fputc('a' - nr_files_on_board + i%onerow,file);
  if (isBoardReflected)
    fputc('8' + nr_rows_on_board - i/onerow,file);
  else
    fputc('1' - nr_rows_on_board + i/onerow,file);
}

void AppendSquare(char *List, square s)
{
  char    add[4];

  add[0]= ' ';
  add[1]= 'a' - nr_files_on_board + s%onerow;
  add[2]= '1' - nr_rows_on_board + s/onerow;
  add[3]= '\0';
  strcat(List, add);
}
