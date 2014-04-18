#include "input/plaintext/line.h"
#include "input/plaintext/token.h"
#include "input/plaintext/input_stack.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/message.h"

#include <string.h>

char InputLine[LINESIZE];    /* This array contains the input as is */

static char LineSpaceChar[] = " \t;.,";

/* read into InputLine until the next1 end of line */
void ReadToEndOfLine(void)
{
  char *p = InputLine;

  do
  {
    NextChar();
  } while (strchr(LineSpaceChar,LastChar));

  while (LastChar!='\n')
  {
    *p++ = LastChar;
    NextChar();
  }

  if (p >= (InputLine + sizeof(InputLine)))
    FtlMsg(InpLineOverflow);

  *p = '\0';
}

void ReadRemark(void)
{
  if (LastChar != '\n')
  {
    ReadToEndOfLine();
    if (TraceFile!=NULL)
    {
      fputs(InputLine, TraceFile);
      fflush(TraceFile);
    }
    Message(NewLine);
  }
}
