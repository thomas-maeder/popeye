/******************** MODIFICATIONS to pyint.c **************************
**
** 1997/03/04 TLi  Original
**
** 1997/04/08 TLi  A lot of modifications. Castling added.
**
** 1997/09/04 TLi  intelligent mode extended to stalemate problems
**
** 1997/10/03 TLi  intelligent bug in connection with ep keys fixed
**
** 1997/10/28 NG   intelligent mode shows detected mating positions 
**                 when option startmovenumber is used.
**
** 1998/06/05 NG   new option: MaxTime
**
** 1998/08/14 NG   On RISC machines SetIndex bug fixed 
**                 due to new  unsigned long  datatype of Flags
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/04/02 NG   New option:  NoCastling
**
** 2000/02/02 NG   testcastling "globalized".
**
** 2000/02/04 TLi  intelligent mode for stale mates enabled
**
** 2000/02/13 TLi  intelligent mode for stale mates enhanced
**
** 2000/02/25 NG   TimeOut for Intelligent Stalemate improved.
**
** 2000/09/17 TLi  bug fix in stale mates intelligent mode
**                 minor improvements of the intelligent mode
**
** 2000/12/04 TLi  bug fix: problems with holes in intelligent mode
**
** 2001/02/05 TLi  more detailed construction of mating positions
**		   1000times faster with some problems; generally
**		   about 5% faster or slower...
**
** 2001/02/13 NG   bug fix: undefined variables nside jouecoup due to
**			    usage of empile instead of encore()
**
** 2001/02/16 TLi  further bug fixes
**
** 2001/03/01 NG   bug fix: BlIllegalCheck() didn't always return a value.
**
** 2001/11/10 NG   bug fix: end message shows "interupted" in intelligent mode
**			    with maxsols, even if only one solution was found
**
**************************** End of List ******************************/
