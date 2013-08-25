#include "output/plaintext/proofgame.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/position.h"
#include "pylang.h"
#include "pymsg.h"

#include <stdio.h>

void ProofWriteStartPosition(slice_index start)
{
  char InitialLine[40];
  sprintf(InitialLine,
          "\nInitial (%s ->):\n",
          ColorString[UserLanguage][slices[start].starter]);
  StdString(InitialLine);
  WritePosition();
}
