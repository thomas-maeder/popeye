#include "output/plaintext/proofgame.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "output/plaintext/position.h"
#include "input/plaintext/language.h"

#include <stdio.h>

void ProofWriteStartPosition(slice_index start)
{
  char InitialLine[40];
  sprintf(InitialLine,
          "\nInitial (%s ->):\n",
          ColourString[UserLanguage][slices[start].starter]);
  StdString(InitialLine);
  WritePosition();
}
