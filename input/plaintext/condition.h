#if !defined(INPUT_PLAINTEXT_CONDITION_H)
#define INPUT_PLAINTEXT_CONDITION_H

#include "position/board.h"

/* to make function ReadSquares in PYIO.C more convenient define
 * ReadImitators und ReadHoles and ReadEpSquares too. They can have
 * any positiv number, but must not coincide with
 * MagicSq...BlConsForcedSq.  TLi
 * Must also not coincide with  WhPromSq  and  BlPromSq.   NG
 */
typedef enum
{
  ReadImitators = nrSquareFlags,
  ReadHoles,
  ReadEpSquares,
  ReadBlRoyalSq,
  ReadWhRoyalSq,
  ReadNoCastlingSquares,
  ReadGrid
} SquareListContext;

char *ReadSquares(SquareListContext context);

char *ParseCond(void);

void  InitCond(void);

#endif
