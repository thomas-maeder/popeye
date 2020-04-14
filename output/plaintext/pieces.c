#include "output/plaintext/pieces.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/hunters.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <string.h>

boolean WriteSpec(output_engine_type const * engine, FILE *file,
                  Flags sp, piece_walk_type p, boolean printcolours)
{
  boolean result = false;

  if (is_piece_neutral(sp))
  {
    (*engine->fputc)(tolower(ColourTab[colour_neutral][0]),file);
    result = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        (*engine->fputc)(tolower(ColourTab[colour_black][0]),file);
      if (TSTFLAG(sp,Black))
        (*engine->fputc)(tolower(ColourTab[colour_white][0]),file);
    }
    else
    {
      if (TSTFLAG(sp,White))
        (*engine->fputc)(tolower(ColourTab[colour_white][0]),file);
      if (TSTFLAG(sp,Black))
        (*engine->fputc)(tolower(ColourTab[colour_black][0]),file);
    }
  }

  if (!(TSTFLAG(sp,Chameleon)) || total_invisible_number==0)
  {
    piece_flag_type spname;
    for (spname = nr_sides; spname<nr_piece_flags; ++spname)
      if ((spname!=Volage || !CondFlag[volage])
          && (spname!=Patrol || !CondFlag[patrouille])
          && (spname!=Beamtet || !CondFlag[beamten])
          && (spname!=Royal || !is_king(p))
          && TSTFLAG(sp, spname))
      {
        char const *curr = PieSpTab[spname-nr_sides];
        while (*curr!=0)
        {
          if (isupper(*curr))
            (*engine->fputc)(tolower(*curr),file);
          ++curr;
        }
        result = true;
      }
  }

  return result;
}

void WriteWalk(output_engine_type const * engine, FILE *file, piece_walk_type p)
{
  if (p==Dummy || (total_invisible_number>0 && p==Empty))
     (*engine->fprintf)(file,"%s",TITab[0]);
  else if (p<Hunter0 || p>= (Hunter0 + max_nr_hunter_walks))
  {
    char const p1 = PieceTab[p][1];
    (*engine->fputc)(toupper(PieceTab[p][0]),file);
    if (p1!=' ')
      (*engine->fputc)(toupper(p1),file);
  }
  else
  {
    unsigned int const i = p-Hunter0;
    WriteWalk(engine,file,huntertypes[i].away);
    (*engine->fputc)('/',file);
    WriteWalk(engine,file,huntertypes[i].home);
  }
}

void WriteSquare(output_engine_type const * engine, FILE *file, square i)
{
  if (i==capture_by_invisible || i==move_by_invisible)
    (*engine->fputc)('~',file);
  else
  {
    (*engine->fputc)('a' - nr_files_on_board + i%onerow,file);
    if (isBoardReflected)
      (*engine->fputc)('8' + nr_rows_on_board - i/onerow,file);
    else
      (*engine->fputc)('1' - nr_rows_on_board + i/onerow,file);
  }
}

void AppendSquare(char *List, square s)
{
  char    add[4];

  add[0]= ' ';
  add[1]= (char)('a' - nr_files_on_board + s%onerow);
  add[2]= (char)('1' - nr_rows_on_board + s/onerow);
  add[3]= '\0';
  strcat(List, add);
}
