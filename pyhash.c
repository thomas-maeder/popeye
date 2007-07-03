/********************* MODIFICATIONS to pyhash.c ***********************
 **
 ** Date       Who  What
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
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 **************************** End of List ******************************/

/**********************************************************************
 ** We hash.
 **
 ** SmallEncode and LargeEncode are functions to encode the actual
 ** position. SmallEncode is used when the starting position contains
 ** less than or equal eight pieces. LargeEncode is used when more
 ** pieces are present. The function TellSmallEncode and TellLargeEncode
 ** are used to decide what encoding to use. Which function to use is
 ** stored in encode.
 ** SmallEncode encodes for each piece the field where it is located.
 ** LargeEncode uses the old scheme introduced by Torsten: eight
 ** bytes at the beginning give in 64 bits the locations of the pieces
 ** coded after the eight bytes. Both functions give for each piece its
 ** type (1 byte) and specification (2 bytes). After this information
 ** about ep-captures, Duellants and Imitators are coded.
 **
 ** The hash table uses a dynamic hashing scheme which allows dynamic
 ** growth and shrinkage of the hashtable. See the relevant dht* files
 ** for more details. Two procedures are used:
 **   dhtLookupElement: This procedure delivers
 **	a nil pointer, when the given position is not in the hashtable,
 **	or a pointer to a hashelement.
 **   dhtEnterElement:  This procedure enters an encoded position
 **	with its values into the hashtable.
 **
 ** When there is no more memory, or more than MaxPositions positions
 ** are stored in the hash-table, then some positions are removed
 ** from the table. This is done in the compress procedure.
 ** This procedure uses a little improved scheme introduced by Torsten.
 ** The selection of positions to remove is based on the value of
 ** information gathered about this position. The information about
 ** a position "unsolvable in 2 moves" is less valuable than "unsolvable
 ** in 5 moves", since the former can recomputed faster. For the other
 ** type of information ("solvable") the compare is the other way round.
 ** The compression of the table is an expensive operation, in a lot
 ** of exeperiments it has shown to be quite effective in keeping the
 ** most valuable information, and speeds up the computation time
 ** considerable. But to be of any use, there must be enough memory to
 ** to store more than 800 positions.
 ** Now Torsten changed popeye so that all stipulations use hashing.
 ** There seems to be no real penalty in using hashing, even if the
 ** hit ratio, is very small and only about 5% it speeds up the
 ** computationtime, by 30%.
 ** I changed the output of hashstat, since its really informative
 ** to see the hit rate.
 **
 ** inithash()
 **   -- enteres the startposition into the hash-table.
 **   -- determines which encode procedure to use
 **   -- Check's for the MaxPostion/MaxMemory settings
 **
 ** closehash()
 **   -- deletes the hashtable and gives back allocated storage.
 **
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/* TurboC and BorlandC	TLi */
#ifdef __TURBOC__
# include <mem.h>
# include <alloc.h>
# include <conio.h>
#else
# include <memory.h>
#endif	/* __TURBOC__ */

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyint.h"
#include "DHT/dhtvalue.h"
#include "DHT/dht.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"

typedef unsigned long	uLong;

struct dht *pyhash;

char	piece_nbr[PieceCount];	/* V3.22  TLi */
boolean one_byte_hash,			/* V3.22  TLi */
  flag_hashall,			/* V3.60  TLi */
  bytes_for_spec,
  bytes_for_piece;		/* V3.53  TLi */

#if defined(TESTHASH)
#define ifTESTHASH(x)	x
#if defined(UNIX)
#include <unistd.h>
static void *OldBreak;
extern int dhtDebug;
#endif /*UNIX*/
#else
#define ifTESTHASH(x)
#endif /*TESTHASH*/

#if defined(HASHRATE)
#define ifHASHRATE(x)	x
static unsigned long use_pos, use_all;	     /* V3.22  TLi */
#else
#define ifHASHRATE(x)
#endif /*HASHRATE*/

/* New Version for more ply's */	/* V3.21  ElB */
#define BitsForPly	10		/* Up to 1023 ply possible */
#define MaskForPly	((1<<BitsForPly)-1)

#define GetFirstHashValue(x)	(((unsigned)(x)>>BitsForPly)&MaskForPly)
/* SerNoSucc, WhHelpNoSucc, WhDirSucc */
#define GetSecondHashValue(x) (((unsigned)(x))&MaskForPly)
/* BlHelpNoSucc, WhDirNoSucc, IntroSerNoSucc */

#define MakeHashData(val1,val2)     (unsigned long)(((val1)<<BitsForPly)|(val2))
/* V3.22  TLi
** Series movers:   val1 = SerNoSucc
**		    val2 = IntroSerNoSucc
** Help problems:   val1 = WhHelpNoSucc
**		    val2 = BlHelpNoSucc
** direct play:     val1 = WhDirSucc
**		    val2 = WhDirNoSucc
*/

static	byte		buffer[256];
static	BCMemValue*	(*encode)(void);

smallint value_of_data(uLong Data)
{
  if (FlowFlag(Intro)) {
	return (GetFirstHashValue(Data)
            + enonce*GetSecondHashValue(Data));
  }
  else {
	if (GetFirstHashValue(Data) <= (unsigned)enonce		/* V3.82  NG */
        && GetFirstHashValue(Data) > GetSecondHashValue(Data))
	{
      return GetFirstHashValue(Data);
	}
	else {
      return GetSecondHashValue(Data);
	}
  }
} /* value_of_data */

#ifdef OLD_COMPRESS
void compresshash (void) {
  dhtElement	    *he;
  smallint	    min_val,x;
  uLong	    RemoveCnt, ToDelete;

  min_val= enonce;

  ifTESTHASH(printf("compressing: %ld -> ", dhtKeyCount(pyhash)));

  he= dhtGetFirstElement(pyhash);
  while (he) {
	x= value_of_data((uLong)he->Data);     /* V3.22  TLi */
	if (x < min_val)
      min_val= x;
	he= dhtGetNextElement(pyhash);
  }
  RemoveCnt= 0;
  ToDelete= (dhtKeyCount(pyhash)>>4) + 1;
  /* the former expression was based on MaxPosition.
   * This is now (and was probably before) wrong!!
   * Now we remove about 1/16 ~ 6% of the entries
   *					V3.29 ElB
   */
  if (ToDelete >= dhtKeyCount(pyhash)) {
	ToDelete= dhtKeyCount(pyhash);
	/* this is pathologic: it may only occur, when we are so
	 * low on memory, that only one or no position can be stored.
	 */
  }
  while (RemoveCnt < ToDelete) {
	min_val++;
	he= dhtGetFirstElement(pyhash);
	while (he) {
      if (value_of_data((uLong)he->Data) <= min_val) {
		RemoveCnt++;
		dhtRemoveElement(pyhash, he->Key);
      }
      he= dhtGetNextElement(pyhash);
	}
  }
#ifdef TESTHASH
  printf("%ld;", dhtKeyCount(pyhash));
  printf(" usage: %ld", use_pos);
  printf(" / %ld", use_all);
  printf(" = %ld%%", (100 * use_pos) / use_all);
#if defined(FREEMAP) && defined(FXF)
  PrintFreeMap(stdout);
#endif /*FREEMAP*/
#if defined(__TURBOC__)
  gotoxy(1, wherey());
#else
  printf("\n");
#endif /*__TURBOC__*/
#if defined(FXF)
  printf("\n after compression:\n");
  fxfInfo(stdout);
#endif /*FXF*/
#endif /*TESTHASH*/
} /* compresshash */
#else
static uLong totalRemoveCount = 0;
void compresshash (void) {
  dhtElement	    *he;
  smallint	    min_val,x;
  uLong	    RemoveCnt, ToDelete, runCnt;
#if defined(TESTHASH)
  uLong initCnt, visitCnt;
#endif
  flag_hashall= false;			/* V3.60  TLi */

  min_val= enonce;

  ifTESTHASH(printf("compressing: %ld -> ", dhtKeyCount(pyhash)));

  he= dhtGetFirstElement(pyhash);
  while (he) {
	x= value_of_data((uLong)he->Data);     /* V3.22  TLi */
	if (x < min_val)
      min_val= x;
	he= dhtGetNextElement(pyhash);
  }
  RemoveCnt= 0;
  ToDelete= (dhtKeyCount(pyhash)>>4) + 1;
  /* the former expression was based on MaxPosition.
   * This is now (and was probably before) wrong!!
   * Now we remove about 1/16 ~ 6% of the entries
   *					V3.29 ElB
   */
  if (ToDelete >= dhtKeyCount(pyhash)) {
	ToDelete= dhtKeyCount(pyhash);
	/* this is pathologic: it may only occur, when we are so
	 * low on memory, that only one or no position can be stored.
	 */
  }

#ifdef TESTHASH
  printf("\nmin_val: %d\n", min_val);
  printf("ToDelete: %ld\n", ToDelete);
  fflush(stdout);
  initCnt= dhtKeyCount(pyhash);
#endif	/* TESTHASH */

  runCnt= 0;
  while (RemoveCnt < ToDelete) {
	min_val++;

#ifdef TESTHASH
	printf("min_val: %d\n", min_val);
	printf("RemoveCnt: %ld\n", RemoveCnt);
	fflush(stdout);
	visitCnt= 0;
#endif	/* TESTHASH */

	he= dhtGetFirstElement(pyhash);
	while (he) {
      if (value_of_data((uLong)he->Data) <= min_val) {
		RemoveCnt++;
		totalRemoveCount++;
        dhtRemoveElement(pyhash, he->Key);
#ifdef TESTHASH
		if (RemoveCnt + dhtKeyCount(pyhash) != initCnt) {
          fprintf(stdout,
                  "dhtRemove failed on %ld-th element of run %ld. "
                  "This was the %ld-th call to dhtRemoveElement.\n"
                  "RemoveCnt=%ld, dhtKeyCount=%ld, initCnt=%ld\n",
                  visitCnt, runCnt, totalRemoveCount,
                  RemoveCnt, dhtKeyCount(pyhash), initCnt);
          exit(1);
		}
#endif	/* TESTHASH */
      }
#ifdef TESTHASH
      visitCnt++;
#endif	/* TESTHASH */
      he= dhtGetNextElement(pyhash);
	}
#ifdef TESTHASH
    runCnt++;
    printf("run=%ld, RemoveCnt: %ld, missed: %ld\n", runCnt, RemoveCnt, initCnt-visitCnt);
    {
      int l, counter[16];
      int KeyCount=dhtKeyCount(pyhash);
      dhtBucketStat(pyhash, counter, 16);
      for (l=0; l< 16-1; l++) {
	    fprintf(stdout, "%d %d %d\n", KeyCount, l+1, counter[l]);
      }
      fprintf(stdout, "%d %d %d\n\n", KeyCount, l+1, counter[l]);
      if (runCnt > 9) {
	    fprintf(stdout, "runCnt > 9 after %ld-th call to  dhtRemoveElement\n", totalRemoveCount);
      }
      dhtDebug= runCnt == 9;
    }
    fflush(stdout);
#endif	/* TESTHASH */

  }
#ifdef TESTHASH
  printf("%ld;", dhtKeyCount(pyhash));
  printf(" usage: %ld", use_pos);
  printf(" / %ld", use_all);
  printf(" = %ld%%", (100 * use_pos) / use_all);
#if defined(FREEMAP) && defined(FXF)
  PrintFreeMap(stdout);
#endif /*FREEMAP*/
#if defined(__TURBOC__)
  gotoxy(1, wherey());
#else
  printf("\n");
#endif /*__TURBOC__*/
#if defined(FXF)
  printf("\n after compression:\n");
  fxfInfo(stdout);
#endif /*FXF*/
#endif /*TESTHASH*/
} /* compresshash */
#endif /*OLD_COMPRESS*/

int HashRateLevel = 0;	/* Level = 0: No output of HashStat
                         * Level = 1: Output with every trace output
                         * Level = 2: Output at each table compression
                         * Level = 3: Output at every 1000th hash entry
                         * a call to HashStats with a value of 0 will
                         * always print
                         */

void HashStats(int level, char *trailer) {
#if defined(HASHRATE)
  int pos=dhtKeyCount(pyhash);
  char rate[60];

  if (level <= HashRateLevel) {
	StdString("  ");
	pos= dhtKeyCount(pyhash);
	logIntArg(pos);
	Message(HashedPositions);
	if (use_all > 0) {
      if (use_all < 10000)
        sprintf(rate, " %ld/%ld = %ld%%",
                use_pos, use_all, (use_pos*100) / use_all);
      else
        sprintf(rate, " %ld/%ld = %ld%%",
                use_pos, use_all, use_pos / (use_all/100));
	}
	else
      sprintf(rate, " -");
	StdString(rate);
	if (HashRateLevel > 3) {
      long Sec = StopTimer();
      if (Sec > 0) {
		sprintf(rate, ", %ld pos/s", use_all/Sec);
		StdString(rate);
      }
	}
	if (trailer)
      StdString(trailer);
  }
#endif /*HASHRATE*/
}

int TellCommonEncodePosLeng(int len, int nbr_p) {		/* V3.57  TLi */
  int i;

  len++;			/* Castling_Flag */	/* V3.35  NG */
  if (CondFlag[haanerchess]) {	/* V3.29  NG */
	/*
	** I assume an average of (64 - number of pieces)/2
	** additional holes per position.
	*/
	/* That's far too much. In a ser-h#5 there won't be more
	** than 5 holes in hashed positions.	  TLi
	*/
	int nbr_holes= FlowFlag(Series) != 0 ? enonce : 2*enonce;
	if (nbr_holes > (64 - nbr_p) / 2) {
      nbr_holes= (64 - nbr_p) / 2;
	}
	len += bytes_for_piece * nbr_holes;		/* V3.57  TLi */
  }
  if (CondFlag[messigny]) {				/* V3.57  TLi */
	len+= 2;
  }
  if (CondFlag[duellist]) {
	len+= 2;
  }
  if (CondFlag[blfollow] || CondFlag[whfollow]) {	/* V3.22  TLi */
    /* V3.29  NG */
	len++;
  }
  if (flag_synchron) {
    len++;
  }
  if (CondFlag[imitators]) {
	for (i = 0; i < inum[nbply]; i++) {
      len++;
	}
	/* coding of no. of imitators and average of one
	   imitator-promotion assumed.
    */						/* V3.29  NG */
	len+=2;
  }
  if (CondFlag[parrain]) {	/* V3.29  NG */
	/*
	** only one out of three positions with a capture
	** assumed.
	*/
	len++;
  }
  if (OptFlag[nontrivial]) {				/* V3.32  TLi */
	len++;
  }
  if (FlowFlag(Exact)) {          /* V3.78  TLi */
    len++;
  }
  return len;
} /* TellCommonEncodePosLeng */

int TellLargeEncodePosLeng(void) {
  square	*bnp;
  int		nbr_p= 0, len= 8;			/* V3.29  NG */

  for (bnp= boardnum; *bnp; bnp++) {
	if (e[*bnp] != vide) {
      len += bytes_for_piece;			/* V3.53  TLi */
      nbr_p++;	/* count no. of pieces and holes
                   V3.29  NG */
	}
  }
  if (CondFlag[BGL])
    len+= 2*sizeof(BGL_white); /* V4.06 SE */

  return TellCommonEncodePosLeng(len, nbr_p);		/* V3.57  TLi */
} /* TellLargeEncodePosLeng */

int TellSmallEncodePosLeng(void) {
  square  *bnp;
  int nbr_p= 0, len= 0;				/* V3.29  NG */

  for (bnp= boardnum; *bnp; bnp++) {
	/* piece	p;
	** Flags	pspec;
	*/
	if (e[*bnp] != vide) {
      len += 1 + bytes_for_piece;			/* V3.53  TLi */
      nbr_p++;		      /* count no. of pieces and holes
                             V3.29  NG */
	}
  }
  return TellCommonEncodePosLeng(len, nbr_p);		/* V3.57  TLi */
} /* TellSmallEncodePosLeng */

byte *CommonEncode(byte *bp)
{
  int i;

  if (CondFlag[messigny]) {				/* V3.57  TLi */
	if (move_generation_stack[nbcou].capture == messigny_exchange) {
      *bp++ = (byte)(move_generation_stack[nbcou].arrival - bas);
      *bp++ = (byte)(move_generation_stack[nbcou].departure - bas);
	}
	else {
      *bp++ = (byte)(0);
      *bp++ = (byte)(0);
	}
  }
  if (CondFlag[duellist]) {
	*bp++ = (byte)(whduell[nbply] - bas);
	*bp++ = (byte)(blduell[nbply] - bas);
  }
  if (CondFlag[blfollow] || CondFlag[whfollow]) {	/* V3.22  TLi */
	*bp++ = (byte)(move_generation_stack[nbcou].departure - bas);
  }
  if (flag_synchron) {
    *bp++= (byte)(sq_num[move_generation_stack[nbcou].departure]-sq_num[move_generation_stack[nbcou].arrival]+64);
  }
  if (CondFlag[imitators]) {				/* V3.22  TLi */
	/* The number of imitators has to be coded too to
	** avoid ambiguities.  V3.22  TLi
	*/
	*bp++ = (byte)inum[nbply];
	for (i = 0; i < inum[nbply]; i++) {
      *bp++ = (byte)(isquare[i] - bas);
	}
  }
  if (OptFlag[nontrivial]) {				/* V3.32  TLi */
	*bp++ = (byte)(NonTrivialNumber);		/* V3.62  TLi */
  }
  if (CondFlag[parrain]) {				/* V3.32  TLi */
	/* a piece has been captured and can be reborn */
	*bp++ = (byte)(move_generation_stack[nbcou].capture - bas);
	if (one_byte_hash) {
      *bp++ = (byte)(pprispec[nbply]) +
        ((byte)(piece_nbr[abs(pprise[nbply])]) << 4);
      /* V3.45  NG */
	}
	else {
      *bp++ = pprise[nbply];
      *bp++ = (byte)(pprispec[nbply]>>8);
      *bp++ = (byte)(pprispec[nbply]&0xff);
	}
  }
  if (FlowFlag(Exact)) {                              /* V3.78  TLi */
    *bp++ = (byte)(nbply);
  }

  if (ep[nbply]) {
	*bp++ = (byte)(ep[nbply] - bas);
  }
  *bp++ = castling_flag[nbply];		/* Castling_Flag */
  /* V3.35  NG */
  if (CondFlag[BGL]) /* V4.06 SE */
  { 
    long int* lip= (long int*)bp;
    *lip++= BGL_white;
    *lip++= BGL_black;
    bp=(byte*)lip;
  }
  return bp;
} /* CommonEncode */

BCMemValue *LargeEncode(void) {

  byte	*bp, *position;
  int		row, col;
  square	bnp;
  BCMemValue	*bcm;

  bcm= (BCMemValue *)buffer;
  position= bp= bcm->Data;
  /* clear the bits for storing the position of pieces */
  memset(position, 0, 8);
  bp= position+8;

  bnp= bas;
  for (row=0; row<8; row++, bnp+= 16) {
	for (col=0; col<8; col++, bnp++) {
      piece p;
      if ((p= e[bnp]) != vide) {
		Flags pspec= spec[bnp];
		if (!TSTFLAG(pspec, Neutral)) {
          SETFLAG(pspec, (p < vide ? Black : White));
		}
		p= abs(p);
		if (one_byte_hash) {			/* V3.22  TLi */
          *bp++ = (byte)(pspec) +		/* V3.22  TLi */
            ((byte)(piece_nbr[p]) << 4);/* V3.22  TLi */
		}
		else {					/* V3.22  TLi */
          smallint i;
          *bp++ = p;
          for (i = 0; i <= bytes_for_spec; i++) {
            /* V3.53  TLi */
			*bp++ = (byte)((pspec>>(8*i)) & 0xff);
          }
		}
		position[row] |= BIT(col);
      }
	}
  }
  /* Now the rest of the party */
  bp= CommonEncode(bp);

  bcm->Leng= bp - bcm->Data;
  return bcm;
} /* LargeEncode */

BCMemValue *SmallEncode(void)
{
  byte	*bp;
  int		i, row, col;
  square	bnp;
  BCMemValue	*bcm;

  bcm= (BCMemValue *)buffer;
  bp= bcm->Data;
  bnp= bas;
  for (row=0; row<8; row++, bnp+= 16) {
	for (col=0; col<8; col++, bnp++) {
      piece p;
      if ((p= e[bnp]) != vide) {
		Flags pspec= spec[bnp];
		if (!TSTFLAG(pspec, Neutral)) {
          SETFLAG(pspec, (p < vide ? Black : White));
		}
		p= abs(p);
		*bp++= (row<<3)+col;
		if (one_byte_hash) {			/* V3.22  TLi */
          *bp++ = (byte)(pspec) +		/* V3.22  TLi */
            ((byte)(piece_nbr[p]) << 4);/* V3.22  TLi */
		}
		else {					/* V3.22  TLi */
          *bp++ = p;
          /* V3.53  TLi */
          for (i = 0; i <= bytes_for_spec; i++) {
			*bp++ = (byte)((pspec>>(8*i)) & 0xff);
          }
		}
      }
	}
  }
  /* Now the rest of the party */
  bp= CommonEncode(bp);

  bcm->Leng= bp - bcm->Data;
  return bcm;
} /* SmallEncode */

boolean inhash(hashwhat what, smallint val)
{
  BCMemValue	*cmv;
  dhtElement	*he;
  boolean	ret;

  cmv= (*encode)();
  he= dhtLookupElement(pyhash, (dhtValue)cmv);

  ifHASHRATE(use_all++);

  if (he != dhtNilElement) {
	switch (what) {
	case SerNoSucc:
	case WhHelpNoSucc:
      ret = FlowFlag(Exact)
        ? (GetFirstHashValue((uLong)he->Data) == (unsigned)val)	/* V3.82  NG */
        : (GetFirstHashValue((uLong)he->Data) >= (unsigned)val);	/* V3.82  NG */
      if (ret) {
		ifHASHRATE(use_pos++);
		return True;
      }
      else {
		return False;
      }
	case IntroSerNoSucc:
	case BlHelpNoSucc:
	case WhDirNoSucc:
      ret = FlowFlag(Exact)
        ? (GetSecondHashValue((uLong)he->Data) == (unsigned)val)	/* V3.82  NG */
        : (GetSecondHashValue((uLong)he->Data) >= (unsigned)val);	/* V3.82  NG */
      if (ret) {
		ifHASHRATE(use_pos++);
		return True;
      } else
		return False;

	case WhDirSucc:
      ret = FlowFlag(Exact)
        ? (GetFirstHashValue((uLong)he->Data) == (unsigned)val)	/* V3.82  NG */
        : (GetFirstHashValue((uLong)he->Data) <= (unsigned)val);	/* V3.82  NG */
      if (ret) {
		ifHASHRATE(use_pos++);
		return True;
      } else
		return False;
	}
  }
  return False;
} /* inhash */

void addtohash(hashwhat what, smallint val)
{
  smallint	hv_1=0, hv_2=0;     /* V3.22  TLi */

  BCMemValue	*cmv;
  unsigned long	dat;
  dhtElement	*he;

  cmv= (*encode)();
  he= dhtLookupElement(pyhash, (dhtValue)cmv);

  if (he == dhtNilElement) { /* the position is new */
	switch (what) {		/* V3.22  TLi */
    case IntroSerNoSucc:
    case SerNoSucc:
    case WhHelpNoSucc:
    case BlHelpNoSucc:
      hv_1= hv_2= 0;
      break;
    case WhDirSucc:
    case WhDirNoSucc:
      hv_1= enonce + 1;
      hv_2= 0;
      break;
	}
	dat= MakeHashData(hv_1, hv_2);	/* V3.22  TLi */
	he= dhtEnterElement(pyhash, (dhtValue)cmv, (dhtValue)dat);
	if (he==dhtNilElement || dhtKeyCount(pyhash) > (unsigned long)MaxPositions) {	/* V3.82  NG */
      compresshash();
      he= dhtEnterElement(pyhash, (dhtValue)cmv, (dhtValue)dat);
      if (he==dhtNilElement
	      || dhtKeyCount(pyhash) > (unsigned long)MaxPositions) {	/* V3.82  NG */
#if defined(FXF)
		ifTESTHASH(
		  printf("make new hashtable, due to trashing\n"));
		inithash();
		he= dhtEnterElement(pyhash,
                            (dhtValue)cmv, (dhtValue)dat);
		if (he==dhtNilElement
            || dhtKeyCount(pyhash) > (unsigned long)MaxPositions) {	/* V3.82  NG */
          fprintf(stderr,
                  "Sorry, cannot enter more hashelements "
                  "despite compression\n");
          exit(-2);
		}
#else
		/* V3.29  NG */
		fprintf(stderr,
                "Sorry, cannot enter more hashelements "
                "despite compression\n");
		exit(-2);
#endif /*FXF*/
      }
	}
  }
  else { /* The position is already registered */
	hv_1= GetFirstHashValue((uLong)he->Data);
	hv_2= GetSecondHashValue((uLong)he->Data);
  }
  switch (what) {
  case SerNoSucc:
  case WhHelpNoSucc:
    if (hv_1 < val)
      he->Data= (dhtValue)MakeHashData(val, hv_2);
    break;
  case IntroSerNoSucc:
  case BlHelpNoSucc:
  case WhDirNoSucc:
    if (hv_2 < val)
      he->Data= (dhtValue)MakeHashData(hv_1, val);
    break;
  case WhDirSucc:
    if (hv_1 > val)
      he->Data= (dhtValue)MakeHashData(val, hv_2);
    break;
  }
#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
	HashStats(3, "\n");
#endif /*HASHRATE*/
} /* addtohash */

EXTERN smallint WhMovesLeft, BlMovesLeft;

boolean introseries(couleur camp, smallint n, boolean restartenabled) {
  /* V3.44 TLi */
  couleur ad = advers(camp);
  boolean flag1= false, flag2= false;

  /* set play */
  if (OptFlag[solapparent]
      || (FlowFlag(Exact) ? n == 0 : n < introenonce))
  {
	boolean is_exact= FlowFlag(Exact);
	int i;

	SatzFlag= True;
	for (i= FlowFlag(Exact) ? enonce : 1; i <= enonce; i++) {
      if (SortFlag(Help)
          ? shsol(ad, i, False)
          : ser_dsrsol(ad, i, False))
      {
		flag1= true;
		StipFlags |= FlowBit(Exact);
		if (OptFlag[stoponshort] && (i < enonce)) {
          /* V3.60  NG */
          FlagShortSolsReached= true;
          break;
		}
      }
	}

	if (n == introenonce) {
      Message(NewLine);
	}
	SatzFlag= False;
	if (!is_exact) {
      StipFlags &= ~FlowBit(Exact);
	}
  }

  if (n && !echecc(ad)) {
	/* generate a single move */
	genmove(camp);
	while (encore()) {
      if (   jouecoup()
             && !echecc(camp)
             && !(restartenabled && MoveNbr < RestartNbr)
             && !inhash(IntroSerNoSucc, n))		/* V3.57  TLi */
      {
		if (introseries(camp, n-1, False)) {
          flag2= true;
		}
		else {
          addtohash(IntroSerNoSucc, n);
		}
      }
      if (restartenabled) {			/* V3.44  TLi */
		IncrementMoveNbr();
      }
      repcoup();
      if (FlagTimeOut || FlagShortSolsReached) {	/* V3.54  NG */
        /* V3.60  NG */
		break;
      }
	} /* encore() */
	finply();
  }

  return flag1 || flag2;
}

boolean shsol(couleur camp, smallint n, boolean restartenabled) {
  couleur ad= advers(camp);
  boolean flag= false;
  boolean flag2= true;    /* V3.43 */

  /* reciprocal helpmovers -- let's check whether black can mate */
  if (FlowFlag(Reci)) {
	stipulation = ReciStipulation;
	TargetField = ReciTargetField;
	DoubleMate = ReciDoubleMate;	    /* V3.32  TLi */

	flag2 = matant(camp, 1);

	stipulation = NonReciStipulation;
	TargetField = NonReciTargetField;
	DoubleMate = NonReciDoubleMate;     /* V3.32  TLi */
  }

  n--;
  if (CounterMate && n == 0) {
	GenMatingMove(camp);
  }
  else {
	genmove(camp);
  }

  if (camp == blanc) {
	WhMovesLeft--;
  }
  else {
	BlMovesLeft--;
  }

  while (encore()) {
	if (jouecoup()
	    && (!OptFlag[intelligent] || MatePossible()) /* V3.45 TLi */
	    && !echecc(camp)
	    && !(restartenabled && MoveNbr < RestartNbr))
	{
      /* V3.44  SE */
      /*	  TLi */
      if (n) {					/* V3.32  TLi */
		if (!echecc(ad)) {
          if (!inhash(SerNoSucc, n+1)) {	/* V3.43  TLi */
            /* V3.57  TLi */
			if (shsol(camp, n, False))	/* V3.44  TLi */
              flag= true;
			else
              addtohash(SerNoSucc, n+1);	/* V3.57  TLi */
          }
		}
      }
      else {	    /* V3.44  TLi */
		if (flag2) {
          /* The following inquiry into the hash tables yields
          ** a significant speed up.	   V3.57   TLi
          */
          if (FlowFlag(Reci) || !inhash(SerNoSucc, 1)) {
			if (last_h_move(ad)) {
              flag= true;
              PrintReciSolution = True;
			}
			else {
              if (!FlowFlag(Reci))
				addtohash(SerNoSucc, 1);
			}
          }
		}
      }
	}
	if (restartenabled) {				/* V3.44  TLi */
      IncrementMoveNbr();
	}
	repcoup();
	if ((OptFlag[maxsols] && (solutions >= maxsolutions))
        || FlagTimeOut)				/* V3.54  NG */
	{
      break;
	}
  }
  if (camp == blanc) {
	WhMovesLeft++;
  }
  else {
	BlMovesLeft++;
  }

  finply();
  /* V3.31  TLi */
  if (flag && FlowFlag(Reci) && PrintReciSolution) {
	/* reciprocal helpmover */
	stipulation = ReciStipulation;
	TargetField = ReciTargetField;
	AlphaEnd = ReciAlphaEnd;
	DoubleMate = ReciDoubleMate;			/* V3.32  TLi */

	last_h_move(camp);

	PrintReciSolution = False;
	stipulation = NonReciStipulation;
	TargetField = NonReciTargetField;
	AlphaEnd = NonReciAlphaEnd;
	DoubleMate = NonReciDoubleMate;			/* V3.32  TLi */
  }
  return flag;
} /* shsol */

boolean mataide(couleur camp, smallint n, boolean restartenabled) {
  boolean flag= false;

  /* Let us check whether the position is already in the
  ** hash table and marked unsolvable.
  */
  /* if (inhash((camp == blanc) ? WhHelpNoSucc : BlHelpNoSucc, n-1)) */
  if ((flag_hashall || n > 1) &&			      /* V3.60	TLi */
      inhash((camp == blanc) ? WhHelpNoSucc : BlHelpNoSucc, n))
	return false;

  if (--n) {
	couleur ad= advers(camp);

	/* reciprocal helpmover */
	/* V3.31  TLi */
	if (n == 1 && FlowFlag(Reci)) {
      stipulation = ReciStipulation;
      TargetField = ReciTargetField;
      DoubleMate = ReciDoubleMate;	/* V3.32  TLi */

      flag = !matant(camp, 1);

      stipulation = NonReciStipulation;
      TargetField = NonReciTargetField;
      DoubleMate = NonReciDoubleMate;	/* V3.32  TLi */
	}
	if (flag)
      return false;

	/* keep mating piece for helpmates ... */	/* V3.42  NG */
	if (OptFlag[keepmating]) {
      piece p= roib+1;
      while (p < derbla && !nbpiece[maincamp == blanc ? p : -p])
        p++;
      if (p == derbla)
        return false;
	}	/* keep mating ... */

	if (CounterMate && n == 1)     /* V3.32  TLi */
      GenMatingMove(camp);
	else
      genmove(camp);

	if (camp == noir)   /* V3.45  TLi */
      BlMovesLeft--;
	else
      WhMovesLeft--;

	while (encore()){
      if (jouecoup()
          && (!OptFlag[intelligent] || MatePossible())  /* V3.45	TLi */
          && !echecc(camp)
          && !(restartenabled && MoveNbr < RestartNbr)
          && (mataide(ad, n, False)))  /* V3.44  SE/TLi */
		flag= true;
      if (restartenabled)    /* V3.44  TLi */
		IncrementMoveNbr();
      repcoup();
      /* Stop solving if a given number of solutions was encountered	 StHoe */
      if ((OptFlag[maxsols] && (solutions >= maxsolutions)) ||
          FlagTimeOut)		/* V3.54  NG */
        break;
	}
	if (camp == noir)   /* V3.45  TLi */
      BlMovesLeft++;
	else
      WhMovesLeft++;

	finply();

	if (flag && FlowFlag(Reci) && n == 1) {     /* reciprocal helpmover */
      stipulation = ReciStipulation;
      TargetField = ReciTargetField;
      AlphaEnd = ReciAlphaEnd;
      DoubleMate = ReciDoubleMate;	  /* V3.32  TLi */

      last_h_move(camp);

      stipulation = NonReciStipulation;
      TargetField = NonReciTargetField;
      AlphaEnd = NonReciAlphaEnd;
      DoubleMate = NonReciDoubleMate;	  /* V3.32  TLi */
	}
  } else {   /* n == 0 */
	flag= last_h_move(camp);		/* V3.31  TLi */
  }
  /* Add the position to the hash table if it has no solutions */
  if (!flag && (flag_hashall || n > 0))
	addtohash(camp == blanc ? WhHelpNoSucc : BlHelpNoSucc, n+1);  /* V3.57	TLi */

  return flag;
} /* mataide */

boolean last_dsr_move(couleur camp)	/* V3.13  TLi */
{
  couleur ad= advers(camp);
  boolean flag = false;			    /* 3.13  TLi */
  if (SortFlag(Direct))	      /* 3.0  TLi */
	GenMatingMove(camp);
  else
	genmove(camp);

  while (encore()) {
	if (jouecoup()) {   /* V3.44  SE/TLi */
      if (SortFlag(Direct)) {
		if ((*stipulation)(camp)) {
          linesolution();	    /* V2.90  NG */
          flag = true;	    /* V3.13  TLi */
		}
      } else
		if (!echecc(camp) && dsr_e(ad,1))
          flag = last_h_move(ad);	   /* V2.90  NG, V3.13	TLi */
	}
	repcoup();
  }
  finply();
  return flag;
} /* last_dsr_move */

boolean ser_dsrsol(couleur camp, smallint n, boolean restartenabled)
{
  boolean flag= false;

  if (SortFlag(Reflex) && matant(camp,1))
	return false;

  if (--n) {
	couleur ad= advers(camp);
	genmove(camp);
	if (camp == blanc)
      WhMovesLeft--;
	else
      BlMovesLeft--;

	while (encore()) {
      if (jouecoup()
          && (!OptFlag[intelligent] || MatePossible())  /* V3.45	TLi */
          && !echecc(camp)
          && !echecc(ad) &&
          !(restartenabled && MoveNbr < RestartNbr)) {   /* V3.44  TLi */
		if (!inhash(SerNoSucc, n)) {
          if (ser_dsrsol(camp,n, False))
			flag= true;
          else
			addtohash(SerNoSucc, n);
		}
      }
      if (restartenabled)
		IncrementMoveNbr();
      repcoup();
      if ((OptFlag[maxsols] && (solutions >= maxsolutions)) ||
          FlagTimeOut)		/* V3.54  NG */
        break;
	}
#ifdef VERYSILLYTYPO   /* V3.62  TLi */
	if (camp == blanc)
      WhMovesLeft--;
	else
      BlMovesLeft--;
#endif
	if (camp == blanc)
      WhMovesLeft++;
	else
      BlMovesLeft++;

	finply();
  } else
	flag = last_dsr_move(camp);   /* V3.32	TLi */

  return  flag;
} /* ser_dsrsol */
/* #endif  V3.44  TLi */

void	inithash(void)
{
  int Small, Large;
  int i, j;	     /* V3.22  TLi */

  ifTESTHASH(
    sprintf(GlobalStr, "calling inithash\n");
    StdString(GlobalStr)
    );

#if defined(UNIX) && defined(TESTHASH)
  OldBreak= sbrk(0);
#endif /*UNIX,TESTHASH*/

#if defined(FXF)
  if (fxfInit(MaxMemory) == -1)	/* we didn't get hashmemory ... */	/* V3.42  NG */
    FtlMsg(NoMemory);
  ifTESTHASH(fxfInfo(stdout));
#endif /*FXF*/

  flag_hashall= true;			/* V3.60  TLi */

  PositionCnt= 0;
  dhtRegisterValue(dhtBCMemValue, 0, &dhtBCMemoryProcs);
  dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
  pyhash= dhtCreate(dhtBCMemValue, dhtCopy, dhtSimpleValue, dhtNoCopy);

  ifHASHRATE(use_pos = use_all = 0);			/* V3.22  TLi */

  /* check whether a piece can be coded in a single byte */
  j = 0;						/* V3.22  TLi */
  for (i = PieceCount; Empty < i; i--) {		/* V3.22  TLi */
	if (exist[i])					/* V3.22  TLi */
      piece_nbr[i] = j++;				/* V3.22  TLi */
  }
  if (CondFlag[haanerchess])				/* V3.57  TLi */
    piece_nbr[obs]= j++;

  one_byte_hash = (j < 16) && (PieSpExFlags < 16);	/* V3.22  TLi */

  bytes_for_spec= 0;		/* V3.53  TLi */
  for (i= 8; i < 24; i+=8)
	if (PieSpExFlags >> i)
      bytes_for_spec += 1;
  bytes_for_piece= one_byte_hash ? 1 : 1 + bytes_for_spec;

  if (OptFlag[intelligent]) { /* V3.76  TLi */
    one_byte_hash = false;
    bytes_for_spec= 4;
  }

  if (SortFlag(Proof)) {     /* V3.35  TLi */
	encode= ProofEncode;
	if (MaxMemory && !MaxPositions)
      MaxPositions= MaxMemory/(24+sizeof(char *)+1);
  } else {
	Small= TellSmallEncodePosLeng();
	Large= TellLargeEncodePosLeng();
	if (Small <= Large) {
      encode= SmallEncode;
      if (MaxMemory && !MaxPositions)
		MaxPositions= MaxMemory/(Small+sizeof(char *)+1);
	}
	else {
      encode= LargeEncode;
      if (MaxMemory && !MaxPositions)
		MaxPositions= MaxMemory/(Large+sizeof(char *)+1);
	}
  }

#if defined(FXF)
  ifTESTHASH(printf("MaxPositions: %7ld\n", MaxPositions));
  ifTESTHASH(printf("MaxMemory:    %7ld KB\n", MaxMemory/1024));
#else
  ifTESTHASH(
    printf("room for up to %ld positions in hash table\n", MaxPositions));
#endif /*FXF*/
} /* inithash */

void	closehash(void)
{
#if defined(TESTHASH)
  sprintf(GlobalStr, "calling closehash\n");
  StdString(GlobalStr);

  sprintf(GlobalStr, "%ld enquiries out of %ld successful. ",
          use_pos, use_all);
  StdString(GlobalStr);
  if (use_all) {
	sprintf(GlobalStr, "Makes %ld%%\n", (100 * use_pos) / use_all);
	StdString(GlobalStr);
  }
#if defined(UNIX)
  {
	unsigned long HashCount, HashMem, BytePerPos;
#if defined(FXF)
	HashMem= fxfTotal();
#else
	HashMem= sbrk(0)-OldBreak;
#endif /*FXF*/
    if ((HashCount=dhtKeyCount(pyhash))>0) {
      BytePerPos= (HashMem*100)/HashCount;
      sprintf(GlobalStr,
              "Memory for hash-table: %ld, "
              "gives %ld.%02ld bytes per position\n",
              HashMem, BytePerPos/100, BytePerPos%100);
    }
    else {
      sprintf(GlobalStr, "Nothing in hashtable\n");
    }
    StdString(GlobalStr);
#endif /*UNIX*/
  }
#endif /*TESTHASH*/

  dhtDestroy(pyhash);
#if defined(TESTHASH) && defined(FXF)
  fxfInfo(stdout);
#endif /*TESTHASH,FXF*/

} /* closehash */

#ifdef NODEF	/* This functions is not used any longer.        */
/* Since when?                        V4.00  TLi */
boolean hashdefense(couleur camp, smallint n) {
  boolean flag = true;

  /* generate a ply */
  genmove(camp);
  while (flag && encore()) {
	/* search for the position in the hash table */
	if (jouecoup() && inhash(WhDirNoSucc, n))   /* V3.44  SE/TLi */
      /* found and marked unsolvable */
      flag = false;
	repcoup();
  }
  finply();

  return !flag;
} /* hashdefense */
#endif

boolean mate(couleur camp, smallint n) {
  /* returns true if camp can defend against a mate in n */
  boolean flag= true, pat= true;
  couleur ad= advers(camp);
  smallint ntcount=0;

  /* check whether `black' can reach a position that is already
  ** marked unsolvable for white in the hash table. */

  /* Check whether the black king has more flight squares than he is
  ** allowed to have. The number of allowed flights (maxflights) is entered
  ** using the solflights option. */

  if (n > 1 && OptFlag[solflights]) {   /* V3.12  TLi */
	/* Initialise the flight counter. The number of flights is counted
	** down. */
	integer zae = maxflights + 1;

	/* Store the square of the black king. */
	square	x = camp == noir ? rn : rb;

	/* generate a ply */
	genmove(camp);

	/* test all possible moves */
	while (encore() && zae) {
      if (jouecoup()) {	  /* V3.44  SE/TLi */
		/* Test whether the king has moved and this move is legal. */
		if ((x != (camp == noir ? rn : rb)) && !echecc(camp))
          /* It is a legal king move. Hence decrement the flight counter */
          zae--;
      }
      repcoup();
	}
	finply();

	if (zae == 0)
      /* The number of flight squares is greater than allowed. */
      return false;
  } /* solflights */

  /* Threat restriction -- white must always check or threat mate in
  ** in a certain number of moves (droh). It is active if droh < enonce-1.
  ** droh is entered with the option solmenaces. */

  if (n > droh)
	if (!(echecc(camp) || matant(ad,droh)))
      return false;

  /* Check whether black has more non trivial moves than he is
  ** allowed to have. The number of such moves allowed (NonTrivialNumber)
  ** is entered using the nontrivial option. */

  if (n > NonTrivialLength) {   /* V3.32  TLi */
	ntcount= -1;

	/* generate a ply */
	genmove(camp);

	/* test all possible moves */
	while (encore() && (NonTrivialNumber >= ntcount)) {
      if (jouecoup()) {  /* V3.44  SE/TLi */
		/* Test whether the move is legal and not trivial. */
		if (!echecc(camp) && !((NonTrivialLength > 0)
                               && matant(ad, NonTrivialLength)))
          /* The move is legal and not trivial.
          ** Hence decrement the  counter. */
          ntcount++;
      }
      repcoup();
	}
	finply();

	if (NonTrivialNumber < ntcount)
      return false;

	NonTrivialNumber -= ntcount;
  } /* nontrivial */

  if (flag) {
    if (n>1)     /* V3.78  SE */
      move_generation_mode= move_generation_mode_opti_per_couleur[camp];

    genmove(camp);
    move_generation_mode= move_generation_optimized_by_killer_move;

    while (flag && encore()) {
      if (jouecoup() && !echecc(camp)) {	 /* V3.44  SE/TLi */
        pat= false;
        if (!(flag= matant(ad,n)))
          coupfort();
      }
      repcoup();
    }
    finply();
  }

  if (n > NonTrivialLength)			/* V3.63  TLi */
	NonTrivialNumber += ntcount;

  if (pat)
	return false;

  return flag;
} /* mate */

boolean matant(couleur camp, smallint n)
{   integer i;
  boolean flag= false;
  couleur ad= advers(camp);

  /* Let's first have a look in the hash_table */
  /* In move orientated stipulations (%, z, x etc.) it's less expensive to
  ** compute a "mate" in 1.	TLi */
  if (n > (FlagMoveOrientatedStip ? 1 : 0)	    /* V3.33  TLi */
      && !SortFlag(Self) && !FlowFlag(Reci)) {    /* V3.32  TLi */
    /* It is more likely that a position has no solution.           */
    /* Therefore let's check for "no solution" first.    V4.00  TLi */
	if (inhash(WhDirNoSucc, n))
      return false;
	if (inhash(WhDirSucc, n))
      return true;
  }


  /* keep mating piece for direct mates ... */	/* V3.42  TLi, NG */
  if (OptFlag[keepmating]) {
    piece p= roib+1;
    while (p < derbla && !nbpiece[camp == blanc ? p : -p])
      p++;
    if (p == derbla)
      return false;
  }	/* keep mating ... */

  n--;
  for (i= FlowFlag(Exact) ? n : 0; !flag && (i <= n); i++) {	/* V3.62  TLi */
	if (i > droh)
      i= n;
	if (i == 0)
      GenMatingMove(camp);
	else
      genmove(camp);

	while (encore() && !flag) {
      if (jouecoup()) {  /* V3.44  SE/TLi */
		if (i)
          flag= !echecc(camp) && mate(ad,i);
		else
          flag= (*stipulation)(camp);
		if (flag)
          coupfort();
      }
      repcoup();
      if (FlagTimeOut)			/* V3.54  NG */
		break;
	}
	finply();
  }

  /* store the results in the hashtable */
  /* In move orientated stipulations (%, z, x etc.) it's less expensive to
  ** compute a "mate" in 1.	TLi */
  if (++n > (FlagMoveOrientatedStip ? 1 : 0)	 /* V3.33  TLi */
      && !SortFlag(Self) && !FlowFlag(Reci))    /* V3.32  TLi */
	addtohash(flag ? WhDirSucc : WhDirNoSucc, n);

  return flag;
} /* matant */

/* Is there a solution to a s# or r# in n? */
boolean invref(couleur	camp, smallint n) {
  boolean flag= false;
  couleur ad= advers(camp);
  int i;

  /* It is more likely that a position has no solution.           */
  /* Therefore let's check for "no solution" first.    V4.00  TLi */
  if (inhash(WhDirNoSucc, n))
	return false;
  if (inhash(WhDirSucc, n))
	return true;

  if (!FlowFlag(Exact))	    /* V3.0exact */
	if ((*stipulation)(ad)) {
      addtohash(WhDirSucc, n);	/* V3.63  TLi */
      return true;
	}

  if (SortFlag(Reflex) && !FlowFlag(Semi) && matant(camp, 1))
	return false;

  for (i = FlowFlag(Exact) ? n : 1; !flag && (i <= n); i++) {
    /* V3.62  TLi */
	if (i > droh || i > NonTrivialLength)
      i = n;
	genmove(camp);
	while ((!flag && encore())) {
      if (jouecoup()) {	/* V3.44  SE/TLi */
		flag= !echecc(camp) && (!definvref(ad,i)||
                                (OptFlag[quodlibet] && (*stipulation)(camp)));	 /* V3.60 SE */
		if (flag)
          coupfort();
      }
      repcoup();
      if (FlagTimeOut)		/* V3.65  NG */
		break;
	}
	finply();
  }

  addtohash(flag ? WhDirSucc :WhDirNoSucc, n);	/* V3.70  TLi */

  return flag;
} /* invref */

square *selfbnp, initiallygenerated;

/* Generate (piece by piece) candidate moves the last move of a s# or
 * r#. Do *not* generate moves for the piece on square
 * initiallygenerated; this piece has already been taken care of. */
boolean selflastencore(couleur camp) {
  piece   p;

  if (encore())
	return true;
  if (TSTFLAG(PieSpExFlags,Neutral))
	initneutre(advers(camp));
  while (*selfbnp) {
	if ((*selfbnp != initiallygenerated) && ((p= e[*selfbnp]) != vide)) {
      if (TSTFLAG(spec[*selfbnp], Neutral))
		p= -p;
      if (camp == blanc) {
		if (p > obs)
          gen_wh_piece(*selfbnp, p);
      } else
		if (p < vide)
          gen_bl_piece(*selfbnp, p);
	}
	selfbnp++;
	if (encore())
      return true;
  }
  return false;
} /* selflastencore */

/* Can camp defend against a s# or r# in n? */
boolean definvref(couleur camp, smallint n) {
  boolean pat= true, flag= true;
  couleur ad= advers(camp);
  integer ntcount=0;

  if (SortFlag(Reflex)) {
	if (matant(camp,1))
      return false;
	if (n == 1)
      return true;
  }

  n--;

  /* seems to be little efficient !!!!  TLi */

  /* Threat restriction -- white must always check or threat mate in
  ** in a certain number of moves (droh). It is active if droh < enonce-1.
  ** droh is entered with the option solmenaces. */

  if (n > droh) {
	if  (!(echecc(camp) || !((droh > 0) && invref(ad,droh)))) {
      return true;
	}
  }

  /* Check whether black has still a piece left to mate */
  if (OptFlag[keepmating]) {
	piece p = roib+1;
	while (p < derbla && !nbpiece[camp == blanc ? p : -p]) {
      p++;
	}
	if (p == derbla) {
      return true;
	}
  } /* keepmating */

  /* Check whether the black king has more flight squares than he is
     allowed to have. The number of allowed flights (maxflights) is
     entered using the solflights option.
  */

  if (n > 1 && OptFlag[solflights]) {   /* V3.12  TLi */
	/* Initialise the flight counter. The number of flights is
	   counted down.
    */
	integer zae = maxflights + 1;

	/* generate a ply */
	genmove(camp);

	/* test all possible moves */
	while (encore() && zae) {
      if (jouecoup() && !echecc(camp)) {
		/* It is a legal move.
		** Hence decrement the flight counter */
		zae--;
      }
      repcoup();
	}
	finply();

	if (zae == 0) {
      /* The number of flight squares is greater than allowed. */
      return true;
	}
  } /* solflights */


  /* Check whether black has more non trivial moves than he is
     allowed to have. The number of such moves allowed
     (NonTrivialNumber)
     is entered using the nontrivial option.
  */

  if (n > NonTrivialLength) {				/* V3.32  TLi */
	ntcount= -1;

	/* generate a ply */
	genmove(camp);

	/* test all possible moves */
	while (encore() && (NonTrivialNumber >= ntcount)) {
      if (jouecoup()
          && !echecc(camp)
          && !((NonTrivialLength > 0)
               && invref(ad, NonTrivialLength)))
      {
		/* The move is legal and not trivial.
		   Hence increment the	counter.
        */
		ntcount++;
      }
      repcoup();
	}
	finply();

	if (NonTrivialNumber < ntcount) {
      /* The number of flight squares is greater than allowed. */
      return true;
	}
	NonTrivialNumber -= ntcount;
  } /* nontrivial */

  if (n || (camp==noir ? flagblackmummer : flagwhitemummer)) {
	move_generation_mode=
      n>1
      ? move_generation_mode_opti_per_couleur[camp]
      : move_generation_optimized_by_killer_move;
	genmove(camp);
	move_generation_mode= move_generation_optimized_by_killer_move;

	while (flag && encore()) {
      if (jouecoup() && !echecc(camp)) {	 /* V3.44  SE/TLi */
		pat= false;
		if (!(flag= n ? invref(ad,n) : (*stipulation)(camp))) {
          coupfort();
		}
      }
      repcoup();
	}
	finply();
  }
  else {
	piece p;
	selfbnp= boardnum;

	if (!(FlagMoveOrientatedStip
	      && stipulation == stip_ep
	      && ep[nbply] == initsquare
	      && ep2[nbply] == initsquare))
	{
      nextply();
      init_move_generation_optimizer();
      trait[nbply]= camp;
      /* flagminmax= false;		 V2.90, V3.44  TLi */
      /* flag_minmax[nbply]= false;	 V3.44	TLi */
      if (TSTFLAG(PieSpExFlags,Neutral)) {
		initneutre(advers(camp));
      }
      initiallygenerated= initsquare;
      p= e[current_killer_state.move.departure];
      if (p!=vide) {
		if (TSTFLAG(spec[current_killer_state.move.departure], Neutral))
          p= -p;
		if (camp == blanc) {
          if (p > obs) {
            initiallygenerated= current_killer_state.move.departure;
			gen_wh_piece(initiallygenerated, p);
          }
		}
		else {
          if (p < -obs) {
            initiallygenerated= current_killer_state.move.departure;
			gen_bl_piece(initiallygenerated, p);
          }
		}
      }
      finish_move_generation_optimizer();
      while (flag && selflastencore(camp)) {
		if (jouecoup() && !echecc(camp)) {   /* V3.44  SE/TLi */
          pat= false;
          flag= (*stipulation)(camp);
          if (!flag) {
			coupfort();
          }
		}
		repcoup();
      }
      finply();
	}
  }

  if (n > NonTrivialLength) {
	NonTrivialNumber += ntcount;
  }

  return !flag || pat;
} /* definvref */
