/********************* MODIFICATIONS to pyhash.c ***********************
** 
** 1993/08/01 TLi  Original
**
** 1993/08/06 NG   Some bug-fixes
**
** 1993/08/09 TLi  detailed explanantion added. Some bug-fixes.
**
** 1993/10/16 ElB  Complete rewrite of the hashing.
**                 Kept only the code of coding the position (frompopye)
**
** 1994/02/19 ElB  New encoding function, improved method of
**                 compressing the hash table
**
** 1994/05/03 ElB  Changed some initialisation in LargeEncode, which
**                 produced core dumps on align-sensitive architectures
**                 memset is used there ("memeset(position, 0, 8);").
**                 This is a little bit overhead for clearing 8 bytes.
**                 If you know a better (AND portable!!) solution, let
**                 me know.
**
** 1994/05/04 TLi  First part of "ser-r and hash"-bug fixed.
**
** 1994/05/05 ElB  Second part of "ser-r and hash"-bug fixed.
**
** 1994/05/05 NG   Third part of "ser-r and hash"-bug fixed.
**
** 1994/08/31 TLi  ??
**
** 1995/01/09 NG   Tell?????EncodePosLeng(): some changes by TLi undone 
**                 due to resulting MaxPositions calculation bug. 
**                 Also changes due to increasing number of pieces/holes
**                 with conditions Imitator and HaanerChess.
**
** 1995/04/26 TLi  new stipulation: h#/= etc.
**
** 1995/06/08 TLi  hashing for direct/self problems improved
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**
** 1995/06/12 TLi  new option: keep mating piece
** 1995/06/12 TLi  coding bug (parrain/ep) in ???Encode fixed
**
** 1995/06/13 TLi  hashing with series movers improved
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/06/28 TLi  hashing bug in matant fixed
**
** 1995/07/19 TLi  new stipulation: diagram == proof game
**
** 1995/08/04 NG   Castling in totalortho play implemented.
**           
** 1995/10/23 NG   MaxMemory assignment moved to py6.c !
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
** 1996/04/13 TLi  bug fixed in ser_dsrsol and shsol
**
** 1996/09/16 NG   option keep mating piece implemented (and speed improved)
**                 in matant() and mataide().
**
** 1996/11/28 TLi  bug fixed in shsol
**
** 1997/02/06 TLi  implementation of help- and series play revised
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/04/23 NG   illegal array index bug fixed.
**
** 1998-01-20 ElB  Changed/Added feature to do Level dependent output
**                 of Hashstatistics. See HashRateLevel.
**
** 1998/06/05 NG   new option: MaxTime
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/07/29 TLi  Haaner chess bug fixed in function inithash()
**
** 1999/07/29 TLi  Messigny bug fixed in functions ?????Encode()
**                 and Tell?????EncodePosLeng()
**                 new functions: CommonEncode() and
**                 TellCommonEncodePosLeng()
**
** 1999/08/17 TLi  shsol() and mataide() modified: additional inquiry into
**                 hash tables to speed up solution procedure.
**
** 1999/08/24 TLi  new: hashing for seriesmovers with introductory moves
**
** 1999/09/29 TLi  bug fix: BrunnerChess
**
** 1999/10/08 SE   new option : Quodlibet
**                 in self- and (semi-)reflex play, allows White to make
**                 contract by direct stip also (eg. #/S# or #/R#).
**
** 1999/11/08 NG   MaxSolutions now calculated also for twins, duplex, ...
**                 New option:  StopOnShortSolutions
**
** 1999/12/22 TLi  intelligent mode for orthodox ser-# enabled
**
** 2000/04/13 TLi  nontrivial implementation changed -- did not work
**                 properly before
**
** 2000/05/09 TLi  optimization of black moves in MarsCirce disabled
**                 yielded mistakes
**
** 2000/05/25 TLi  bug fix in nontrivial mode
**
** 2000/05/27 NG   bug fix: fatal message if main memory can't be allocated
**
** 2001/01/18 NG   bug fix: maxtime didn't work well with stipulation selfmate
**
** 2001/03/01 NG   bug fix: uninitialised variable visitCnt inside compresshash()
**
** 2001/10/04 TLi  hashing bug in invref fixed
**
** 2003/05/12 TLi  hashing bug fixed: h= + intel did not find all solutions .
**
** 2004/03/22 TLi  hashing for exact-* stipulations improved
**
** 2005/02/01 TLi  function hashdefense is not used anymore...
**
** 2005/02/01 TLi  in matant and invref exchanged the inquiry into the hash
**                 table for "white can mate" and "white cannot mate" because
**                 it is more likely that a position has no solution
**                 This yields an incredible speedup of .5-1%  *fg*
**
**************************** End of List ******************************/
