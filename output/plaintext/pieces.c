#include "output/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pyproc.h"

#include <ctype.h>

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
        && (spname!=Royal || (p!=King && p!=Poseidon))
        && TSTFLAG(sp, spname))
    {
      StdChar(tolower(PieSpString[UserLanguage][spname-nr_sides][0]));
      ret = true;
    }

  return ret;
}
