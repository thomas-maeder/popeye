#include "input/plaintext/input_stack.h"
#include "output/output.h"
#include "pymsg.h"

FILE *InputStack[MAXNEST];

unsigned int NestLevel = 0;

void OpenInput(char const *s)
{
  InputStack[0] = fopen(s,"r");
  if(InputStack[0]==NULL)
    InputStack[0] = stdin;
}

void CloseInput(void)
{
  if(InputStack[0]!=stdin)
    fclose(InputStack[0]);
}

int PushInput(char const *FileName)
{
  if ((NestLevel+1) < MAXNEST)
  {
    InputStack[NestLevel+1] = fopen(FileName,"r");
    if (InputStack[NestLevel+1]==NULL)
    {
      IoErrorMsg(RdOpenError,0);
      return -1;
    }
    else
    {
      ++NestLevel;
      return 0;
    }
  }
  else
  {
    IoErrorMsg(TooManyInputs,0);
    return -1;
  }
}

int PopInput(void)
{
  fclose(InputStack[NestLevel]);

  if (NestLevel-->0)
    return 0;
  else
    return -1;
}
