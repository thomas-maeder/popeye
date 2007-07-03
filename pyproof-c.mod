/********************* MODIFICATIONS to pyproof.c ***********************
**
** Date       Who  What
**
** 1995/08/10 TLi  Original
**
** 1995/09/28 NG   Bug fixing:
**		   function ProofBlKingMovesNeeded() cast variable
**		   was not set EVERY time, but used !
**		   typo fixed: at the end of function BlPawnMovesNeeded()
**		   (Wh|Bl)PromPieceMovesFromTo(): cenpromsq+-i got out
**		   of range sometimes (example: cenpromsq==272, but there
**		   where 5 captures possible, so cenpromsq+i reached 276)
**		   Also the assignment of  *captures  was different there.
**
** 1995/10/03 NG   ProofFairyImpossible(): test included to check if a
**		   pawn can move from it's original square in AntiCirce,
**		   because it would need at least two captures to bring
**		   it back.
**
** 1995/10/06 NG   ProofFairyImpossible(): bug in above test fixed
**		   and slightly improved.
**
** 1995/10/27 NG   spec[] initilisation needed in ProofInitialise() !
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/08 NG   Lines 533,542,551 not compilable WITHOUT -DOPTIMIZE
**		   so I did a not usable hack only for compilation.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**		   deleted
**		   bug in functions (Wh|Bl)PromPieceMovesFromTo() fixed.
**
** 1995/12/29 TLi  new condition: GlasgowChess
**
** 1996/06/10 NG   new stipulation: ser-dia (SeriesProofGame)
**
** 1996/12    SE/TLi  new condition: isardam
**
** 1997/01/15 NG   antiandernachchess bug fixed
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/10/28 SE   proofgame twinning bug fixed.
**		   problems due to BorlandC V3.1 compiler fixed.
**
** 1997/11/08 NG   problems due to compilerbug fix above fixed, ie.
**		   correction only within define  BC31_SUCKS
**
** 1998/06/05 NG   new option: MaxTime
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/05/25 TLi  bugfix: condition messigny and proofgames
**
** 2000/10/17 TLi  bugfix: Haaner Chess
**
** 2001/05/20 SE   new stipulation: atob
**
** 2002/08/20 TLi  bug fix in ProofFairyImpossible
**
**************************** INFORMATION END **************************/
