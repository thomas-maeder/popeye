#include "output/plaintext/twinning.h"
#include "output/plaintext/plaintext.h"
#include "pymsg.h"

#include <stdio.h>
#include <string.h>

void WriteTwinNumber(unsigned int TwinNumber)
{
  if (TwinNumber-1<='z'-'a')
    sprintf(GlobalStr, "%c) ", 'a'+TwinNumber-1);
  else
    sprintf(GlobalStr, "z%u) ", (unsigned int)(TwinNumber-1-('z'-'a')));

  StdString(GlobalStr);
}
