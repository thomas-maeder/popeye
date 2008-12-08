#include "trace.h"
#include "pyproc.h"
#include "pydata.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(DOTRACE)

static boolean deactivated = false;

static unsigned int level;

static unsigned long move_counter;

void TraceDeactivate()
{
  deactivated = true;
}

void TraceFunctionEntry(char const *name)
{
  ++level;
    
  if (!deactivated)
    fprintf(stdout,"> #%d %s",level,name);
}

void TraceFunctionExit(char const *name)
{
  if (!deactivated)
    fprintf(stdout,"< #%d %s",level,name);

  --level;
}

void TraceValueImpl(char const *format, int value)
{
  if (!deactivated)
    fprintf(stdout,format,value);
}

void TraceText(char const *text)
{
  if (!deactivated)
    fprintf(stdout,"  #%d %s",level,text);
}

void TraceSquareImpl(char const *prefix, square s)
{
  if (!deactivated)
  {
    fprintf(stdout,"%s",prefix);
    WriteSquare(s);
  }
}

void TracePieceImpl(char const *prefix, piece p)
{
  if (!deactivated)
  {
    fprintf(stdout,"%s",prefix);
    WritePiece(p);
  }
}

void TraceCurrentMove()
{
  if (!deactivated)
  {
    fprintf(stdout," #%d %ld ",level,move_counter++);
    ecritcoup(no_goal);
    fprintf(stdout,"\n");
  }
}

void TracePosition(echiquier e, Flags flags[maxsquare+4])
{
  square *bnp;
  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (e[*bnp]!=vide && e[*bnp]!=obs)
    {
      WriteSpec(spec[*bnp],true);
      WritePiece(abs(e[*bnp]));
      WriteSquare(*bnp);
      fprintf(stdout," ");
    }

  fprintf(stdout,"\n");
}

#endif
