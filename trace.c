#include "trace.h"
#include "pyproc.h"
#include "pydata.h"

#include <stdio.h>

#if defined(DOTRACE)

void TraceFunctionEntry(char const *name)
{
  printf("> %s",name);
}

void TraceFunctionExit(char const *name)
{
  printf("< %s",name);
}

void TraceValueImpl(char const *format, int value)
{
  printf(format,value);
}

void TraceText(char const *text)
{
  printf("%s",text);
}

void TraceCurrentMove()
{
  WriteSquare(move_generation_stack[nbcou].departure);
  printf("-");
  WriteSquare(move_generation_stack[nbcou].arrival);
  printf("\n");
}


#endif
