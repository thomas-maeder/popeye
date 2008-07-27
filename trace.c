#include "trace.h"
#include "pyproc.h"
#include "pydata.h"

#include <stdio.h>

#if defined(DOTRACE)

static unsigned int level;

void TraceFunctionEntry(char const *name)
{
  ++level;
  printf("> #%d %s",level,name);
}

void TraceFunctionExit(char const *name)
{
  printf("< #%d %s",level,name);
  --level;
}

void TraceValueImpl(char const *format, int value)
{
  printf(format,value);
}

void TraceText(char const *text)
{
  printf("  #%d %s",level,text);
}

void TraceCurrentMove()
{
  printf(" #%d ",level);
  ecritcoup(no_goal);
  printf("\n");
}


#endif
