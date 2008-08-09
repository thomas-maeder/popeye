#include "trace.h"
#include "pyproc.h"
#include "pydata.h"

#include <stdio.h>
#include <stdlib.h>

static boolean deactivated = false;

void TraceDeactivate()
{
  deactivated = true;
}

#if defined(DOTRACE)

static unsigned int level;

static unsigned long move_counter;

void TraceFunctionEntry(char const *name)
{
  if (!deactivated)
  {
    ++level;
    printf("> #%d %s",level,name);
  }
}

void TraceFunctionExit(char const *name)
{
  if (!deactivated)
  {
    printf("< #%d %s",level,name);
    --level;
  }
}

void TraceValueImpl(char const *format, int value)
{
  if (!deactivated)
    printf(format,value);
}

void TraceText(char const *text)
{
  if (!deactivated)
    printf("  #%d %s",level,text);
}

void TraceSquareImpl(char const *prefix, square s)
{
  if (!deactivated)
  {
    printf("%s",prefix);
    WriteSquare(s);
  }
}

void TraceCurrentMove()
{
  if (!deactivated)
  {
    printf(" #%d %ld ",level,move_counter++);
    ecritcoup(no_goal);
    printf("\n");
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
      printf(" ");
    }

  printf("\n");
}

#endif
