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

void WriteFlag(output_engine_type const * engine, FILE *file,
               piece_flag_type spname)
{
  char const *curr = PieSpTab[spname-nr_sides];
  while (*curr!=0)
  {
	if (isupper((unsigned char)*curr))
	  (*engine->fputc)(tolower((unsigned char)*curr),file);
    ++curr;
  }
}

boolean WriteSpec(output_engine_type const * engine, FILE *file,
                  Flags sp, piece_walk_type p, boolean printcolours)
{
  boolean result = false;

  if (is_piece_neutral(sp))
  {
    (*engine->fputc)(tolower((unsigned char)ColourTab[colour_neutral][0]),file);
    result = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        (*engine->fputc)(tolower((unsigned char)ColourTab[colour_black][0]),file);
      if (TSTFLAG(sp,Black))
        (*engine->fputc)(tolower((unsigned char)ColourTab[colour_white][0]),file);
    }
    else
    {
      if (TSTFLAG(sp,White))
        (*engine->fputc)(tolower((unsigned char)ColourTab[colour_white][0]),file);
      if (TSTFLAG(sp,Black))
        (*engine->fputc)(tolower((unsigned char)ColourTab[colour_black][0]),file);
    }
  }

  if (!(TSTFLAG(sp,Chameleon)) || total_invisible_number==0)
  {
    piece_flag_type spname;
    for (spname = (piece_flag_type)((unsigned int)nr_sides); spname<nr_piece_flags; ++spname)
      if ((spname!=Volage || !CondFlag[volage])
          && (spname!=Patrol || !CondFlag[patrouille])
          && (spname!=Beamtet || !CondFlag[beamten])
          && (spname!=Royal || !is_king(p))
          && TSTFLAG(sp, spname))
      {
    	WriteFlag(engine,file,spname);
        result = true;
      }
  }

  return result;
}

void WriteWalk(output_engine_type const * engine, FILE *file, piece_walk_type p)
{
  if (total_invisible_number>0 && (p==Dummy || p==Empty))
    (*engine->fprintf)(file,"%s",TITab[0]);
  else if (p<Hunter0 || p>= (Hunter0 + max_nr_hunter_walks))
  {
    char const p1 = PieceTab[p][1];
    (*engine->fputc)(toupper((unsigned char)PieceTab[p][0]),file);
    if (p1!=' ')
      (*engine->fputc)(toupper((unsigned char)p1),file);
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
    (*engine->fputc)(getBoardFileLabel((i%onerow) - nr_files_on_board),file);
    if (isBoardReflected)
      (*engine->fputc)(getBoardRowLabel(((2*nr_rows_on_board)-1) - (i/onerow)),file);
    else
      (*engine->fputc)(getBoardRowLabel((i/onerow) - nr_rows_on_board),file);
  }
}

void AppendSquare(char *List, square s)
{
  char    add[4];

  add[0]= ' ';
  add[1]= (char)getBoardFileLabel((s%onerow) - nr_files_on_board);
  add[2]= (char)getBoardRowLabel((s/onerow) - nr_rows_on_board);
  add[3]= '\0';
  strcat(List, add);
}
