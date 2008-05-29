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
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)  
 **
 **************************** End of List ******************************/

/**********************************************************************
 ** We hash.
 **
 ** SmallEncode and LargeEncode are functions to encode the current
 ** position. SmallEncode is used when the starting position contains
 ** less than or equal to eight pieces. LargeEncode is used when more
 ** pieces are present. The function TellSmallEncode and TellLargeEncode
 ** are used to decide what encoding to use. Which function to use is
 ** stored in encode.
 ** SmallEncode encodes for each piece the square where it is located.
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
 ** a nil pointer, when the given position is not in the hashtable,
 ** or a pointer to a hashelement.
 **   dhtEnterElement:  This procedure enters an encoded position
 ** with its values into the hashtable.
 **
 ** When there is no more memory, or more than MaxPositions positions
 ** are stored in the hash-table, then some positions are removed
 ** from the table. This is done in the compress procedure.
 ** This procedure uses a little improved scheme introduced by Torsten.
 ** The selection of positions to remove is based on the value of
 ** information gathered about this position. The information about
 ** a position "unsolvable in 2 moves" is less valuable than "unsolvable
 ** in 5 moves", since the former can be recomputed faster. For the other
 ** type of information ("solvable") the comparison is the other way round.
 ** The compression of the table is an expensive operation, in a lot
 ** of exeperiments it has shown to be quite effective in keeping the
 ** most valuable information, and speeds up the computation time
 ** considerably. But to be of any use, there must be enough memory to
 ** to store more than 800 positions.
 ** Now Torsten changed popeye so that all stipulations use hashing.
 ** There seems to be no real penalty in using hashing, even if the
 ** hit ratio is very small and only about 5%, it speeds up the
 ** computation time by 30%.
 ** I changed the output of hashstat, since its really informative
 ** to see the hit rate.
 **
 ** inithash()
 **   -- enters the startposition into the hash-table.
 **   -- determines which encode procedure to use
 **   -- Check's for the MaxPostion/MaxMemory settings
 **
 ** closehash()
 **   -- deletes the hashtable and gives back allocated storage.
 **
 ***********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* TurboC and BorlandC  TLi */
#if defined(__TURBOC__)
# include <mem.h>
# include <alloc.h>
# include <conio.h>
#else
# include <memory.h>
#endif  /* __TURBOC__ */

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyint.h"
#include "DHT/dhtvalue.h"
#include "DHT/dht.h"
#include "pyproof.h"
#include "platform/maxtime.h"

struct dht *pyhash;

static char    piece_nbr[PieceCount];
static boolean one_byte_hash;
static boolean flag_hashall;
static unsigned int bytes_per_spec;
static unsigned int bytes_per_piece;

#if defined(TESTHASH)
#define ifTESTHASH(x)   x
#if defined(__unix)
#include <unistd.h>
static void *OldBreak;
extern int dhtDebug;
#endif /*__unix*/
#else
#define ifTESTHASH(x)
#endif /*TESTHASH*/

#if defined(HASHRATE)
#define ifHASHRATE(x)   x
static unsigned long use_pos, use_all;
#else
#define ifHASHRATE(x)
#endif /*HASHRATE*/

/* New Version for more ply's */
enum {
  ByteMask = (1u<<CHAR_BIT)-1,
  BitsForPly = 10      /* Up to 1023 ply possible */
};

void (*encode)(HashBuffer*);

typedef struct {
	dhtValue	Key;
    struct
    {
        unsigned int dummy : BitsForPly+BitsForPly;
        unsigned int what : 3;
    } data;
} genericElement_t;

typedef struct {
	dhtValue	Key;
    struct
    {
        unsigned int notSolvableInLessThan : BitsForPly;
        unsigned int solvableIn : BitsForPly;
        unsigned int what : 3;
    } data;
} whDirElement_t;

typedef struct {
	dhtValue	Key;
    struct
    {
        unsigned int blackNotSolvableIn : BitsForPly;
        unsigned int whiteNotSolvableIn : BitsForPly;
        unsigned int what : 3;
    } data;
} helpElement_t;

typedef struct {
	dhtValue	Key;
    struct
    {
        unsigned int introNotSolvableIn : BitsForPly;
        unsigned int serNotSolvableIn : BitsForPly;
        unsigned int what : 3;
    } data;
} serElement_t;

/* how much is element *he worth to us? This information is used to
 * determine which elements to discard from the hash table if it has
 * reached its capacity. */
static int value_of_data(dhtElement const *he)
{
  genericElement_t const * const ge = (genericElement_t const *)he;
  switch (ge->data.what) {
  case SerNoSucc:
  case IntroSerNoSucc:
  {
    serElement_t const * const se = (serElement_t const *)he;
    return se->data.serNotSolvableIn + enonce*se->data.introNotSolvableIn;
  }
  
  case WhHelpNoSucc:
  case BlHelpNoSucc:
  {
    helpElement_t const * const hle = (helpElement_t const *)he;
    if (hle->data.blackNotSolvableIn > hle->data.whiteNotSolvableIn)
      return hle->data.blackNotSolvableIn;
    else
      return hle->data.whiteNotSolvableIn;
  }
  
  case WhDirSucc:
  case WhDirNoSucc:
  {
    whDirElement_t const * const wde = (whDirElement_t const *)he;
    if (wde->data.solvableIn <= (unsigned int)enonce
        && wde->data.solvableIn+1 > wde->data.notSolvableInLessThan)
      return wde->data.solvableIn;
    else
      return wde->data.notSolvableInLessThan-1;
  }
  
  default:
    assert(0);
  }

  return 0; /* avoid compiler warning */
} /* value_of_data */

static unsigned long totalRemoveCount = 0;

static void compresshash (void) {
  dhtElement        *he;
  int       min_val,x;
  unsigned long RemoveCnt, ToDelete, runCnt;
#if defined(TESTHASH)
  unsigned long initCnt, visitCnt;
#endif
  flag_hashall= false;

  min_val= enonce;

  ifTESTHASH(printf("compressing: %ld -> ", dhtKeyCount(pyhash)));

  he= dhtGetFirstElement(pyhash);
  while (he) {
    x= value_of_data(he);
    if (x < min_val)
      min_val= x;
    he= dhtGetNextElement(pyhash);
  }
  RemoveCnt= 0;
  ToDelete= dhtKeyCount(pyhash)/16 + 1;
  if (ToDelete >= dhtKeyCount(pyhash)) {
    ToDelete= dhtKeyCount(pyhash);
    /* this is a pathological case: it may only occur, when we are so
     * low on memory, that only one or no position can be stored.
     */
  }

#if defined(TESTHASH)
  printf("\nmin_val: %d\n", min_val);
  printf("ToDelete: %ld\n", ToDelete);
  fflush(stdout);
  initCnt= dhtKeyCount(pyhash);
#endif  /* TESTHASH */

  runCnt= 0;
  while (RemoveCnt < ToDelete) {
    min_val++;

#if defined(TESTHASH)
    printf("min_val: %d\n", min_val);
    printf("RemoveCnt: %ld\n", RemoveCnt);
    fflush(stdout);
    visitCnt= 0;
#endif  /* TESTHASH */

    he= dhtGetFirstElement(pyhash);
    while (he) {
      if (value_of_data(he) <= min_val) {
        RemoveCnt++;
        totalRemoveCount++;
        dhtRemoveElement(pyhash, he->Key);
#if defined(TESTHASH)
        if (RemoveCnt + dhtKeyCount(pyhash) != initCnt) {
          fprintf(stdout,
                  "dhtRemove failed on %ld-th element of run %ld. "
                  "This was the %ld-th call to dhtRemoveElement.\n"
                  "RemoveCnt=%ld, dhtKeyCount=%ld, initCnt=%ld\n",
                  visitCnt, runCnt, totalRemoveCount,
                  RemoveCnt, dhtKeyCount(pyhash), initCnt);
          exit(1);
        }
#endif  /* TESTHASH */
      }
#if defined(TESTHASH)
      visitCnt++;
#endif  /* TESTHASH */
      he= dhtGetNextElement(pyhash);
    }
#if defined(TESTHASH)
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
#endif  /* TESTHASH */

  }
#if defined(TESTHASH)
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

#if defined(HASHRATE)
/* Level = 0: No output of HashStat
 * Level = 1: Output with every trace output
 * Level = 2: Output at each table compression
 * Level = 3: Output at every 1000th hash entry
 * a call to HashStats with a value of 0 will
 * always print
 */
static unsigned int HashRateLevel = 0;

void IncHashRateLevel(void)
{
  ++HashRateLevel;
  StdString("  ");
  PrintTime();
  logIntArg(HashRateLevel);
  Message(IncrementHashRateLevel);
  HashStats(0, "\n");
}

void DecHashRateLevel(void)
{
  if (HashRateLevel>0)
    --HashRateLevel;
  StdString("  ");
  PrintTime();
  logIntArg(HashRateLevel);
  Message(DecrementHashRateLevel);
  HashStats(0, "\n");
}

#else

void IncHashRateLevel(void)
{
  /* intentionally nothing */
}

void DecHashRateLevel(void)
{
  /* intentionally nothing */
}

#endif

void HashStats(int level, char *trailer) {
#if defined(HASHRATE)
  int pos=dhtKeyCount(pyhash);
  char rate[60];

  if (level <= HashRateLevel) {
    StdString("  ");
    pos= dhtKeyCount(pyhash);
    logIntArg(pos);
    Message(HashedPositions);
    if (use_all > 0)
    {
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
    if (HashRateLevel > 3)
    {
      unsigned long msec;
      unsigned long Seconds;
      StopTimer(&Seconds,&msec);
      if (Seconds > 0)
      {
        sprintf(rate, ", %lu pos/s", use_all/Seconds);
        StdString(rate);
      }
    }
    if (trailer)
      StdString(trailer);
  }
#endif /*HASHRATE*/
}

static int TellCommonEncodePosLeng(int len, int nbr_p) {
  int i;

  len++; /* Castling_Flag */
  if (CondFlag[haanerchess]) {
    /*
    ** I assume an average of (nr_files_on_board*nr_rows_on_board -
    ** number of pieces)/2 additional holes per position.
    */
    /* That's far too much. In a ser-h#5 there won't be more
    ** than 5 holes in hashed positions.      TLi
    */
    int nbr_holes= FlowFlag(Series) != 0 ? enonce : 2*enonce;
    if (nbr_holes > (nr_files_on_board*nr_rows_on_board-nbr_p)/2)
      nbr_holes= (nr_files_on_board*nr_rows_on_board-nbr_p)/2;
    len += bytes_per_piece*nbr_holes;
  }
  if (CondFlag[messigny]) {
    len+= 2;
  }
  if (CondFlag[duellist]) {
    len+= 2;
  }
  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue]) {
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
    */
    len+=2;
  }
  if (CondFlag[parrain]) {
    /*
    ** only one out of three positions with a capture
    ** assumed.
    */
    len++;
  }
  if (OptFlag[nontrivial]) {
    len++;
  }
  if (FlowFlag(Exact)) {
    len++;
  }
  return len;
} /* TellCommonEncodePosLeng */

static int TellLargeEncodePosLeng(void) {
  square    *bnp;
  int       nbr_p= 0, len= 8;

  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != vide) {
      len += bytes_per_piece;
      nbr_p++;  /* count no. of pieces and holes */
    }
  }
  if (CondFlag[BGL])
    len+= sizeof BGL_white + sizeof BGL_black;

  return TellCommonEncodePosLeng(len, nbr_p);
} /* TellLargeEncodePosLeng */

static int TellSmallEncodePosLeng(void) {
  square  *bnp;
  int nbr_p= 0, len= 0;

  for (bnp= boardnum; *bnp; bnp++) {
    /* piece    p;
    ** Flags    pspec;
    */
    if (e[*bnp] != vide) {
      len += 1 + bytes_per_piece;
      nbr_p++;            /* count no. of pieces and holes */
    }
  }
  return TellCommonEncodePosLeng(len, nbr_p);
} /* TellSmallEncodePosLeng */

static byte *CommonEncode(byte *bp)
{
  if (CondFlag[messigny]) {
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

  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue])
    *bp++ = (byte)(move_generation_stack[nbcou].departure - bas);

  if (flag_synchron)
    *bp++= (byte)(sq_num[move_generation_stack[nbcou].departure]
                  -sq_num[move_generation_stack[nbcou].arrival]
                  +64);

  if (CondFlag[imitators]) {
    int i;
    /* The number of imitators has to be coded too to
    ** avoid ambiguities.
    */
    *bp++ = (byte)inum[nbply];
    for (i = 0; i < inum[nbply]; i++) {
      *bp++ = (byte)(isquare[i] - bas);
    }
  }

  if (OptFlag[nontrivial])
    *bp++ = (byte)(NonTrivialNumber);

  if (CondFlag[parrain]) {
    /* a piece has been captured and can be reborn */
    *bp++ = (byte)(move_generation_stack[nbcou].capture - bas);
    if (one_byte_hash) {
      *bp++ = (byte)(pprispec[nbply])
        + ((byte)(piece_nbr[abs(pprise[nbply])]) << (CHAR_BIT/2));
    }
    else {
      *bp++ = pprise[nbply];
      *bp++ = (byte)(pprispec[nbply]>>CHAR_BIT);
      *bp++ = (byte)(pprispec[nbply]&ByteMask);
    }
  }

  if (FlowFlag(Exact))
    *bp++ = (byte)(nbply);

  if (ep[nbply]!=initsquare)
    *bp++ = (byte)(ep[nbply] - bas);

  *bp++ = castling_flag[nbply];     /* Castling_Flag */

  if (CondFlag[BGL]) {
    memcpy(bp, &BGL_white, sizeof BGL_white);
    bp += sizeof BGL_white;
    memcpy(bp, &BGL_black, sizeof BGL_black);
    bp += sizeof BGL_black;
  }

  return bp;
} /* CommonEncode */

static void LargeEncode(HashBuffer *hb) {
  byte  *position= hb->cmv.Data;
  byte  *bp= position+nr_rows_on_board;
  int       row, col;
  square a_square= square_a1;

  /* clear the bits for storing the position of pieces */
  memset(position, 0, nr_rows_on_board);

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow) {
    square curr_square = a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right) {
      piece p= e[curr_square];
      if (p!=vide) {
        Flags pspec= spec[curr_square];
        if (!TSTFLAG(pspec, Neutral))
          SETFLAG(pspec, (p < vide ? Black : White));
        p= abs(p);
        if (one_byte_hash)
          *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
        else {
          unsigned int i;
          *bp++ = p;
          for (i = 0; i<bytes_per_spec; i++)
            *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
        }
        position[row] |= BIT(col);
      }
    }
  }

  /* Now the rest of the party */
  bp= CommonEncode(bp);

  hb->cmv.Leng= bp - hb->cmv.Data;
} /* LargeEncode */

static void SmallEncode(HashBuffer *hb)
{
  byte   *bp= hb->cmv.Data;
  int    row, col;
  square a_square= square_a1;

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow) {
    square curr_square= a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right) {
      piece p= e[curr_square];
      if (p!=vide) {
        Flags pspec= spec[curr_square];
        if (!TSTFLAG(pspec, Neutral))
          SETFLAG(pspec, (p < vide ? Black : White));
        p= abs(p);
        *bp++= (byte)((row<<(CHAR_BIT/2))+col);
        if (one_byte_hash)
          *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
        else {
          unsigned int i;
          *bp++ = p;
          for (i = 0; i<bytes_per_spec; i++)
            *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
        }
      }
    }
  }

  /* Now the rest of the party */
  bp= CommonEncode(bp);

  hb->cmv.Leng= bp - hb->cmv.Data;
} /* SmallEncode */

boolean inhash(hashwhat what, int val, HashBuffer *hb)
{
  dhtElement const * const he= dhtLookupElement(pyhash, (dhtValue)hb);

  ifHASHRATE(use_all++);

  if (he==dhtNilElement)
    return False;
  else
    switch (what)
    {
    case SerNoSucc:
    {
      serElement_t const * const sere = (serElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? sere->data.serNotSolvableIn == (unsigned)val
        : sere->data.serNotSolvableIn >= (unsigned)val;
      assert(sere->data.what==SerNoSucc || sere->data.what==IntroSerNoSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      }
      else
        return False;
    }
    case IntroSerNoSucc:
    {
      serElement_t const * const sere = (serElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? sere->data.introNotSolvableIn == (unsigned)val
        : sere->data.introNotSolvableIn >= (unsigned)val;
      assert(sere->data.what==SerNoSucc || sere->data.what==IntroSerNoSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      } else
        return False;
    }
    case WhHelpNoSucc:
    {
      helpElement_t const * const hlpe = (helpElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? hlpe->data.whiteNotSolvableIn == (unsigned)val
        : hlpe->data.whiteNotSolvableIn >= (unsigned)val;
      assert(hlpe->data.what==WhHelpNoSucc || hlpe->data.what==BlHelpNoSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      }
      else
        return False;
    }
    case BlHelpNoSucc:
    {
      helpElement_t const * const hlpe = (helpElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? hlpe->data.blackNotSolvableIn == (unsigned)val
        : hlpe->data.blackNotSolvableIn >= (unsigned)val;
      assert(hlpe->data.what==WhHelpNoSucc || hlpe->data.what==BlHelpNoSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      } else
        return False;
    }
    case WhDirNoSucc:
    {
      whDirElement_t const * const wde = (whDirElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? wde->data.notSolvableInLessThan == (unsigned)val+1
        : wde->data.notSolvableInLessThan >= (unsigned)val+1;
      assert(wde->data.what==WhDirNoSucc || wde->data.what==WhDirSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      } else
        return False;
    }
    case WhDirSucc:
    {
      whDirElement_t const * const wde = (whDirElement_t*)he;
      boolean const ret = FlowFlag(Exact)
        ? wde->data.solvableIn == (unsigned)val
        : wde->data.solvableIn <= (unsigned)val;
      assert(wde->data.what==WhDirNoSucc || wde->data.what==WhDirSucc);
      if (ret) {
        ifHASHRATE(use_pos++);
        return True;
      } else
        return False;
    }
    default:
      assert(0);
    }

  return False; /* avoid compiler warning */
} /* inhash */

void addtohash(hashwhat what, int val, HashBuffer *hb)
{
  dhtElement *he = dhtLookupElement(pyhash, (dhtValue)hb);

  if (he == dhtNilElement) { /* the position is new */
    he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
    if (he==dhtNilElement
        || dhtKeyCount(pyhash) > MaxPositions) {
      compresshash();
      he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
      if (he==dhtNilElement
          || dhtKeyCount(pyhash) > MaxPositions) {
#if defined(FXF)
        ifTESTHASH(
          printf("make new hashtable, due to trashing\n"));
        inithash();
        he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
        if (he==dhtNilElement
            || dhtKeyCount(pyhash) > MaxPositions) {
          fprintf(stderr,
                  "Sorry, cannot enter more hashelements "
                  "despite compression\n");
          exit(-2);
        }
#else
        fprintf(stderr,
                "Sorry, cannot enter more hashelements "
                "despite compression\n");
        exit(-2);
#endif /*FXF*/
      }
    }
    switch (what) {
    case IntroSerNoSucc:
    {
      serElement_t * const sere = (serElement_t*)he;
      sere->data.what = what;
      sere->data.introNotSolvableIn = val;
      sere->data.serNotSolvableIn = 0;
      break;
    }
    case SerNoSucc:
    {
      serElement_t * const sere = (serElement_t*)he;
      sere->data.what = what;
      sere->data.serNotSolvableIn = val;
      sere->data.introNotSolvableIn = 0;
      break;
    }
    case WhHelpNoSucc:
    {
      helpElement_t * const hlpe = (helpElement_t*)he;
      hlpe->data.what = what;
      hlpe->data.whiteNotSolvableIn = val;
      hlpe->data.blackNotSolvableIn = 0;
      break;
    }
    case BlHelpNoSucc:
    {
      helpElement_t * const hlpe = (helpElement_t*)he;
      hlpe->data.what = what;
      hlpe->data.whiteNotSolvableIn = 0;
      hlpe->data.blackNotSolvableIn = val;
      break;
    }
    case WhDirSucc:
    {
      whDirElement_t * const wde = (whDirElement_t*)he;
      wde->data.what = what;
      wde->data.solvableIn = val;
      wde->data.notSolvableInLessThan = 0;
      break;
    }
    case WhDirNoSucc:
    {
      whDirElement_t * const wde = (whDirElement_t*)he;
      wde->data.what = what;
      wde->data.solvableIn = enonce+1;
      wde->data.notSolvableInLessThan = val+1;
      break;
    }
    }
  }
  else
  {
    switch (what) {
    case IntroSerNoSucc:
    {
      serElement_t * const sere = (serElement_t*)he;
      assert(sere->data.what==SerNoSucc || sere->data.what==IntroSerNoSucc);
      if (sere->data.introNotSolvableIn < val)
        sere->data.introNotSolvableIn = val;
      break;
    }
    case SerNoSucc:
    {
      serElement_t * const sere = (serElement_t*)he;
      assert(sere->data.what==SerNoSucc || sere->data.what==IntroSerNoSucc);
      if (sere->data.serNotSolvableIn < val)
        sere->data.serNotSolvableIn = val;
      break;
    }
    case WhHelpNoSucc:
    {
      helpElement_t * const hlpe = (helpElement_t*)he;
      assert(hlpe->data.what==WhHelpNoSucc || hlpe->data.what==BlHelpNoSucc);
      if (hlpe->data.whiteNotSolvableIn < val)
        hlpe->data.whiteNotSolvableIn = val;
      break;
    }
    case BlHelpNoSucc:
    {
      helpElement_t * const hlpe = (helpElement_t*)he;
      assert(hlpe->data.what==WhHelpNoSucc || hlpe->data.what==BlHelpNoSucc);
      if (hlpe->data.blackNotSolvableIn < val)
        hlpe->data.blackNotSolvableIn = val;
      break;
    }
    case WhDirSucc:
    {
      whDirElement_t * const wde = (whDirElement_t*)he;
      assert(wde->data.what==WhDirNoSucc || wde->data.what==WhDirSucc);
      if (wde->data.solvableIn > val)
        wde->data.solvableIn = val;
      break;
    }
    case WhDirNoSucc:
    {
      whDirElement_t * const wde = (whDirElement_t*)he;
      assert(wde->data.what==WhDirNoSucc || wde->data.what==WhDirSucc);
      if (wde->data.notSolvableInLessThan < val+1)
        wde->data.notSolvableInLessThan = val+1;
      break;
    }
    }
  }
#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
} /* addtohash */

EXTERN int WhMovesLeft, BlMovesLeft;

boolean introseries(couleur camp, int n, boolean restartenabled) {
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
      if (jouecoup()
          && !echecc(camp)
          && !(restartenabled && MoveNbr < RestartNbr))
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(IntroSerNoSucc, n, &hb))
        {
          if (introseries(camp, n-1, False))
            flag2= true;
          else
            addtohash(IntroSerNoSucc, n, &hb);
        }
      }
      if (restartenabled) {
        IncrementMoveNbr();
      }
      repcoup();
      if (FlagTimeOut || FlagShortSolsReached)
        break;
    }
    finply();
  }

  return flag1 || flag2;
}

boolean shsol(couleur camp, int n, boolean restartenabled) {
  couleur ad= advers(camp);
  boolean flag= false;
  boolean flag2= true;

  /* reciprocal helpmovers -- let's check whether black can mate */
  if (FlowFlag(Reci)) {
    currentStipSettings = stipSettings[reciprocal];
    flag2 = matant(camp, 1);
    currentStipSettings = stipSettings[nonreciprocal];
  }

  n--;
  if (currentStipSettings.stipulation==stip_countermate && n == 0) {
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
        && (!OptFlag[intelligent] || MatePossible())
        && !echecc(camp)
        && !(restartenabled && MoveNbr < RestartNbr))
    {
      if (n>0) {
        if (!echecc(ad)) {
          HashBuffer hb;
          (*encode)(&hb);
          if (!inhash(SerNoSucc,n+1,&hb)) {
            if (shsol(camp, n, False))
              flag= true;
            else
              addtohash(SerNoSucc,n+1,&hb);
          }
        }
      }
      else {
        if (flag2) {
          /* The following inquiry into the hash tables yields
          ** a significant speed up.
          */
          HashBuffer hb;
          (*encode)(&hb);
          if (FlowFlag(Reci) || !inhash(SerNoSucc,1,&hb)) {
            if (last_h_move(ad)) {
              flag= true;
              PrintReciSolution = True;
            }
            else {
              if (!FlowFlag(Reci))
                addtohash(SerNoSucc,1,&hb);
            }
          }
        }
      }
    }
    if (restartenabled) {
      IncrementMoveNbr();
    }
    repcoup();
    if ((OptFlag[maxsols] && (solutions >= maxsolutions))
        || FlagTimeOut)
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

  if (flag && FlowFlag(Reci) && PrintReciSolution) {
    /* reciprocal helpmover */
    currentStipSettings = stipSettings[reciprocal];
    last_h_move(camp);
    PrintReciSolution = False;
    currentStipSettings = stipSettings[nonreciprocal];
  }
  return flag;
} /* shsol */

boolean mataide(couleur camp, int n, boolean restartenabled) {
  boolean flag= false;
  HashBuffer hb;
  boolean const dohash = flag_hashall || n > 1;

  /* Let us check whether the position is already in the
  ** hash table and marked unsolvable.
  */
  if (dohash) {
    (*encode)(&hb);
    if (inhash((camp == blanc) ? WhHelpNoSucc : BlHelpNoSucc, n, &hb))
      return false;
  }

  if (--n) {
    couleur ad= advers(camp);

    /* reciprocal helpmover */
    if (n == 1 && FlowFlag(Reci)) {
      currentStipSettings = stipSettings[reciprocal];
      flag = !matant(camp, 1);
      currentStipSettings = stipSettings[nonreciprocal];
    }
    if (flag)
      return false;

    /* keep mating piece for helpmates ... */
    if (OptFlag[keepmating]) {
      piece p= roib+1;
      while (p < derbla && nbpiece[maincamp == blanc ? p : -p]==0)
        p++;
      if (p == derbla)
        return false;
    }   /* keep mating ... */

    if (currentStipSettings.stipulation==stip_countermate && n == 1)
      GenMatingMove(camp);
    else
      genmove(camp);

    if (camp == noir)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore()){
      if (jouecoup()
          && (!OptFlag[intelligent] || MatePossible())
          && !echecc(camp)
          && !(restartenabled && MoveNbr < RestartNbr)
          && (mataide(ad, n, False)))
        flag= true;
      if (restartenabled)
        IncrementMoveNbr();
      repcoup();
      /* Stop solving if a given number of solutions was encountered */
      if ((OptFlag[maxsols] && (solutions >= maxsolutions)) ||
          FlagTimeOut)
        break;
    }
    if (camp == noir)
      BlMovesLeft++;
    else
      WhMovesLeft++;

    finply();

    if (flag && FlowFlag(Reci) && n == 1) {     /* reciprocal helpmover */
      currentStipSettings = stipSettings[reciprocal];
      last_h_move(camp);
      currentStipSettings = stipSettings[nonreciprocal];
    }
  } else {   /* n == 0 */
    flag= last_h_move(camp);
  }

  /* Add the position to the hash table if it has no solutions */
  if (!flag && dohash)
    addtohash(camp == blanc ? WhHelpNoSucc : BlHelpNoSucc, n+1, &hb);

  return flag;
} /* mataide */

boolean last_dsr_move(couleur camp)
{
  couleur ad= advers(camp);
  boolean flag = false;
  if (SortFlag(Direct))
    GenMatingMove(camp);
  else
    genmove(camp);

  while (encore()) {
    if (jouecoup()) {
      if (SortFlag(Direct)) {
        if (currentStipSettings.checker(camp)) {
          linesolution();
          flag = true;
        }
      } else
        if (!echecc(camp) && dsr_e(ad,1))
          flag = last_h_move(ad);
    }
    repcoup();
  }
  finply();
  return flag;
} /* last_dsr_move */

boolean ser_dsrsol(couleur camp, int n, boolean restartenabled)
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
          && (!OptFlag[intelligent] || MatePossible())
          && !echecc(camp)
          && !echecc(ad) &&
          !(restartenabled && MoveNbr < RestartNbr)) {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(SerNoSucc, n, &hb)) {
          if (ser_dsrsol(camp,n, False))
            flag= true;
          else
            addtohash(SerNoSucc, n, &hb);
        }
      }
      if (restartenabled)
        IncrementMoveNbr();
      repcoup();
      if ((OptFlag[maxsols] && (solutions >= maxsolutions)) ||
          FlagTimeOut)
        break;
    }
    if (camp == blanc)
      WhMovesLeft++;
    else
      BlMovesLeft++;

    finply();
  } else
    flag = last_dsr_move(camp);

  return  flag;
} /* ser_dsrsol */

void inithash(void)
{
  int Small, Large;
  int i, j;

  ifTESTHASH(
    sprintf(GlobalStr, "calling inithash\n");
    StdString(GlobalStr)
    );

#if defined(__unix) && defined(TESTHASH)
  OldBreak= sbrk(0);
#endif /*__unix,TESTHASH*/

#if defined(FXF)
  if (fxfInit(MaxMemory) == -1) /* we didn't get hashmemory ... */
    FtlMsg(NoMemory);
  ifTESTHASH(fxfInfo(stdout));
#endif /*FXF*/

  flag_hashall= true;

  PositionCnt= 0;
  dhtRegisterValue(dhtBCMemValue, 0, &dhtBCMemoryProcs);
  dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
  pyhash= dhtCreate(dhtBCMemValue, dhtCopy, dhtSimpleValue, dhtNoCopy);

  ifHASHRATE(use_pos = use_all = 0);

  /* check whether a piece can be coded in a single byte */
  j = 0;
  for (i = PieceCount; Empty < i; i--) {
    if (exist[i])
      piece_nbr[i] = j++;
  }
  if (CondFlag[haanerchess])
    piece_nbr[obs]= j++;

  one_byte_hash = j<(1<<(CHAR_BIT/2)) && PieSpExFlags<(1<<(CHAR_BIT/2));

  bytes_per_spec= 1;
  if ((PieSpExFlags >> CHAR_BIT) != 0)
    bytes_per_spec++;
  if ((PieSpExFlags >> 2*CHAR_BIT) != 0)
    bytes_per_spec++;

  bytes_per_piece= one_byte_hash ? 1 : 1+bytes_per_spec;

  if (OptFlag[intelligent]) {
    one_byte_hash = false;
    bytes_per_spec= 5; /* TODO why so high??? */
  }

  if (SortFlag(Proof)) {
    encode= ProofEncode;
    if (MaxMemory>0 && MaxPositions==0)
      MaxPositions= MaxMemory/(24+sizeof(char *)+1);
  } else {
    Small= TellSmallEncodePosLeng();
    Large= TellLargeEncodePosLeng();
    if (Small <= Large) {
      encode= SmallEncode;
      if (MaxMemory>0 && MaxPositions==0)
        MaxPositions= MaxMemory/(Small+sizeof(char *)+1);
    }
    else {
      encode= LargeEncode;
      if (MaxMemory>0 && MaxPositions==0)
        MaxPositions= MaxMemory/(Large+sizeof(char *)+1);
    }
  }

#if defined(FXF)
  ifTESTHASH(printf("MaxPositions: %7lu\n", MaxPositions));
  ifTESTHASH(printf("MaxMemory:    %7lu KB\n", MaxMemory/1024));
#else
  ifTESTHASH(
    printf("room for up to %lu positions in hash table\n", MaxPositions));
#endif /*FXF*/
} /* inithash */

void    closehash(void)
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
#if defined(__unix)
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
#endif /*__unix*/
  }
#endif /*TESTHASH*/

  dhtDestroy(pyhash);
#if defined(TESTHASH) && defined(FXF)
  fxfInfo(stdout);
#endif /*TESTHASH,FXF*/

} /* closehash */

boolean mate(couleur camp, int n) {
  /* returns true if camp can defend against a mate in n */
  boolean flag= true, pat= true;
  couleur ad= advers(camp);
  int ntcount=0;

  /* check whether `black' can reach a position that is already
  ** marked unsolvable for white in the hash table. */

  /* Check whether the black king has more flight squares than he is
  ** allowed to have. The number of allowed flights (maxflights) is entered
  ** using the solflights option. */

  if (n > 1 && OptFlag[solflights]) {
    /* Initialise the flight counter. The number of flights is counted
    ** down. */
    int zae = maxflights + 1;

    /* Store the square of the black king. */
    square  x = camp == noir ? rn : rb;

    /* generate a ply */
    genmove(camp);

    /* test all possible moves */
    while (encore() && zae) {
      if (jouecoup()) {
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

  if (n > NonTrivialLength) {
    ntcount= -1;

    /* generate a ply */
    genmove(camp);

    /* test all possible moves */
    while (encore() && (NonTrivialNumber >= ntcount)) {
      if (jouecoup()) {
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
    if (n>1)
      move_generation_mode= move_generation_mode_opti_per_couleur[camp];

    genmove(camp);
    move_generation_mode= move_generation_optimized_by_killer_move;

    while (flag && encore()) {
      if (jouecoup() && !echecc(camp)) {
        pat= false;
        if (!(flag= matant(ad,n)))
          coupfort();
      }
      repcoup();
    }
    finply();
  }

  if (n > NonTrivialLength)
    NonTrivialNumber += ntcount;

  if (pat)
    return false;

  return flag;
} /* mate */

boolean matant(couleur camp, int n)
{
  int i;
  boolean flag= false;
  couleur ad= advers(camp);
  HashBuffer hb;
  boolean const dohash = (n > (FlagMoveOrientatedStip ? 1 : 0)
                          && !SortFlag(Self) && !FlowFlag(Reci));

  /* Let's first have a look in the hash_table */
  /* In move orientated stipulations (%, z, x etc.) it's less expensive to
  ** compute a "mate" in 1. TLi */
  if (dohash) {
    /* It is more likely that a position has no solution.           */
    /* Therefore let's check for "no solution" first.  TLi */
    (*encode)(&hb);
    if (inhash(WhDirNoSucc, n, &hb))
      return false;
    if (inhash(WhDirSucc, n, &hb))
      return true;
  }


  /* keep mating piece for direct mates ... */
  if (OptFlag[keepmating]) {
    piece p= roib+1;
    while (p < derbla && nbpiece[camp == blanc ? p : -p]==0)
      p++;
    if (p == derbla)
      return false;
  } /* keep mating ... */

  n--;
  for (i= FlowFlag(Exact) ? n : 0; !flag && (i <= n); i++) {
    if (i > droh)
      i= n;
    if (i == 0)
      GenMatingMove(camp);
    else
      genmove(camp);

    while (encore() && !flag) {
      if (jouecoup()) {
        if (i)
          flag= !echecc(camp) && mate(ad,i);
        else
          flag= currentStipSettings.checker(camp);
        if (flag)
          coupfort();
      }
      repcoup();
      if (FlagTimeOut)
        break;
    }
    finply();
  }

  ++n;

  /* store the results in the hashtable */
  /* In move orientated stipulations (%, z, x etc.) it's less expensive to
  ** compute a "mate" in 1. TLi */
  if (dohash)
    addtohash(flag ? WhDirSucc : WhDirNoSucc, n, &hb);

  return flag;
} /* matant */

/* Is there a solution to a s# or r# in n? */
boolean invref(couleur  camp, int n) {
  boolean result= false;
  couleur ad= advers(camp);
  int i;
  HashBuffer hb;

  /* It is more likely that a position has no solution. */
  /* Therefore let's check for "no solution" first. TLi */
  (*encode)(&hb);
  if (inhash(WhDirNoSucc,n,&hb)) {
    assert(!inhash(WhDirSucc,n,&hb));
    return false;
  }
  if (inhash(WhDirSucc,n,&hb))
    return true;

  if (!FlowFlag(Exact))
    if (currentStipSettings.checker(ad)) {
      addtohash(WhDirSucc,n,&hb);
      assert(!inhash(WhDirNoSucc,n,&hb));
      return true;
    }

  if (SortFlag(Reflex) && !FlowFlag(Semi) && matant(camp, 1))
    return false;

  for (i = FlowFlag(Exact) ? n : 1; !result && (i <= n); i++) {
    if (i > droh || i > NonTrivialLength)
      i = n;
    genmove(camp);
    while ((!result && encore())) {
      if (jouecoup()) {
        result= (!echecc(camp)
                 && (!definvref(ad,i)
                     || (OptFlag[quodlibet]
                         && currentStipSettings.checker(camp))));
        if (result)
          coupfort();
      }
      repcoup();
      if (FlagTimeOut)
        break;
    }
    finply();
  }

  addtohash(result ? WhDirSucc : WhDirNoSucc, n, &hb);

  return result;
} /* invref */

/* Generate (piece by piece) candidate moves the last move of a s# or
 * r#. Do *not* generate moves for the piece on square
 * initiallygenerated; this piece has already been taken care of. */
boolean selflastencore(couleur camp,
                       square const **selfbnp,
                       square initiallygenerated) {
  if (encore())
    return true;
  else {
    square curr_square= **selfbnp;

    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(advers(camp));

    while (curr_square) {
      if (curr_square!=initiallygenerated) {
        piece p= e[curr_square];
        if (p!=vide) {
          if (TSTFLAG(spec[curr_square], Neutral))
            p= -p;
          if (camp==blanc) {
            if (p>obs)
              gen_wh_piece(curr_square,p);
          } else
            if (p<vide)
              gen_bl_piece(curr_square,p);
        }
      }
    
      ++*selfbnp;
      curr_square= **selfbnp;

      if (encore())
        return true;
    }

    return false;
  }
} /* selflastencore */

/* Can camp defend against a s# or r# in n? */
boolean definvref(couleur camp, int n) {
  boolean pat= true, flag= true;
  couleur ad= advers(camp);
  int ntcount=0;

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
    while (p < derbla && nbpiece[camp == blanc ? p : -p]==0) {
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

  if (n > 1 && OptFlag[solflights]) {
    /* Initialise the flight counter. The number of flights is
       counted down.
    */
    int zae = maxflights + 1;

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

  if (n > NonTrivialLength) {
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
           Hence increment the  counter.
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
      if (jouecoup() && !echecc(camp)) {
        pat= false;
        if (!(flag= n ? invref(ad,n) : currentStipSettings.checker(camp))) {
          coupfort();
        }
      }
      repcoup();
    }
    finply();
  }
  else if (!(FlagMoveOrientatedStip
             && currentStipSettings.stipulation == stip_ep
             && ep[nbply] == initsquare
             && ep2[nbply] == initsquare))
  {
    piece p;
    square const *selfbnp= boardnum;
    square initiallygenerated= initsquare;

    nextply();
    init_move_generation_optimizer();
    trait[nbply]= camp;
    if (TSTFLAG(PieSpExFlags,Neutral)) {
      initneutre(advers(camp));
    }
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
    while (flag && selflastencore(camp,&selfbnp,initiallygenerated)) {
      if (jouecoup() && !echecc(camp)) {
        pat= false;
        flag= currentStipSettings.checker(camp);
        if (!flag) {
          coupfort();
        }
      }
      repcoup();
    }
    finply();
  }

  if (n > NonTrivialLength) {
    NonTrivialNumber += ntcount;
  }

  return !flag || pat;
} /* definvref */

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made in the hashing module. Abort if one of them
 * isn't met.
 * This is called from checkGlobalAssumptions() once at program start.
 */
void check_hash_assumptions(void)
{
  /* SmallEncode uses 1 byte for both row and file of a square */
  assert(nr_rows_on_board<1<<(CHAR_BIT/2));
  assert(nr_files_on_board<1<<(CHAR_BIT/2));

  /* LargeEncode() uses 1 bit per square */
  assert(nr_files_on_board<=CHAR_BIT);

  /* the encoding functions encode Flags as 2 bytes */
  assert(PieSpCount<=2*CHAR_BIT);
  
  assert(sizeof(dhtElement)==sizeof(genericElement_t));
  assert(sizeof(dhtElement)==sizeof(whDirElement_t));
  assert(sizeof(dhtElement)==sizeof(helpElement_t));
  assert(sizeof(dhtElement)==sizeof(serElement_t));

  {
    genericElement_t ge;
    
    /* using this rather than &ge further down keeps gcc quiet: */
    genericElement_t *pge = &ge;

    ge.data.what = WhDirSucc;
    assert(WhDirSucc==((whDirElement_t*)pge)->data.what);
    assert(WhDirSucc==((helpElement_t*)pge)->data.what);
    assert(WhDirSucc==((serElement_t*)pge)->data.what);
  }
}
