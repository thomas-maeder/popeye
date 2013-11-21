#include "output/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "pymsg.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

boolean WriteSpec(Flags sp, PieNam p, boolean printcolours)
{
  boolean ret = false;
  PieSpec spname;

  if (is_piece_neutral(sp))
  {
    StdChar(tolower(ColorString[UserLanguage][color_neutral][0]));
    ret = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(ColorString[UserLanguage][color_black][0]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(ColorString[UserLanguage][color_white][0]));
    }
    else
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(ColorString[UserLanguage][color_white][0]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(ColorString[UserLanguage][color_black][0]));
    }
  }

  for (spname = nr_sides; spname<PieSpCount; ++spname)
    if ((spname!=Volage || !CondFlag[volage])
        && (spname!=Patrol || !CondFlag[patrouille])
        && (spname!=Beamtet || !CondFlag[beamten])
        && (spname!=Royal || (p!=King && p!=Poseidon && p!=Sting))
        && TSTFLAG(sp, spname))
    {
      StdChar(tolower(PieSpString[UserLanguage][spname-nr_sides][0]));
      ret = true;
    }

  return ret;
}

void WritePiece(PieNam p)
{
  if (p<Hunter0 || p>= (Hunter0 + maxnrhuntertypes))
  {
    char const p1 = PieceTab[p][1];
    StdChar(toupper(PieceTab[p][0]));
    if (p1!=' ')
      StdChar(toupper(p1));
  }
  else
  {
    unsigned int const i = p-Hunter0;
    assert(i<maxnrhuntertypes);
    WritePiece(huntertypes[i].away);
    StdChar('/');
    WritePiece(huntertypes[i].home);
  }
}

void WriteSquare(square i)
{
  StdChar('a' - nr_files_on_board + i%onerow);
  if (isBoardReflected)
    StdChar('8' + nr_rows_on_board - i/onerow);
  else
    StdChar('1' - nr_rows_on_board + i/onerow);
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
