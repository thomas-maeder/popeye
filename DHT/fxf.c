#if defined (FXF)

#include <stdlib.h>
#include <stdio.h>

#if defined(__TURBOC__)
#  include <mem.h>
#else
#  include <memory.h>
#  if defined(DBMALLOC)
#    include "dbmalloc.h"
#  endif /*DBMALLOC*/
#endif /*__TURBOC__*/

#include "fxf.h"

#if !defined(Nil) && !defined(New) && !defined(nNew)
#  define Nil(type)      (type *)0
#  define New(type)      (type *)malloc(sizeof(type))
#  define nNew(n, type)  (type *)malloc((n)*sizeof(type))
#endif /*Nil*/

/*#define DEBUG*/

#if defined(DEBUG)
#  define  df  stderr
#  define  DBG(x)  fprintf x
#else
#  define  DBG(x)
#endif /*DEBUG*/

#if !defined(ERROR_LOG2) || !defined(ERROR_LOG3) || !defined(WARN_LOG3)
#  define ERROR_LOG2(s,a,b)   fprintf(stderr,s,a,b)
#  define ERROR_LOG3(s,a,b,c) fprintf(stderr,s,a,b,c)
#  define WARN_LOG3(s,a,b,c)  fprintf(stderr,s,a,b,c)
#endif /*!ERROR_LOGS*/



/* FiXed and Fast malloc, free
 * As the name tells: this code implements on top of traditional
 * malloc/realloc/free a fast version, that relies on a lot of
 * allocation/delallocation of fixed sized blocks of memory. For
 * each size of memory we keep a head pointer and all freed chunks
 * of memory is threaded on this list. If memory of this size
 * is requested, we drag it from the list, otherwise we carf it
 * from larger preallocated (via malloc) chunks of memory. If
 * the sizes between allocation and deallocation vary to often,
 * this package is lost in fragmented memory.
 */

typedef struct {
    unsigned long  MallocCount;
    unsigned long  FreeCount;
    char *    FreeHead;
} SizeHead;

#if defined(DOS)
/* MSDOS 16 Bit support (maxmemory <= 1 MB) */
#define SEGMENTED
#define ARENA_SEG_SIZE  32000
#define ARENA_SEG_COUNT ((1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "MSDOS"
#define OSMAXMEM "1 MB"
#elif defined(_WIN98)
/* Win95/Win98/WinME can only allocate chunks up to 255 MB */
/* maxmemory <= 768 MB */
#define SEGMENTED
#define ARENA_SEG_SIZE  1000000
#define ARENA_SEG_COUNT ((768*1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "Win95/Win98/WinME"
#define OSMAXMEM "768 MB"
#endif

/* The maximum size an fxfAlloc can handle */
#if defined(SEGMENTED) || defined(__TURBOC__)
#define fxfMAXSIZE  1024
#else
#define fxfMAXSIZE  2048  /* this is needed only when sizeof(void*)==8 */
#endif
/* Different size of fxfMINSIZE for 32-/64/Bit compilation */
#if defined(SIXTYFOUR)
#define fxfMINSIZE  (size_t)8
#else
#define fxfMINSIZE  sizeof(char*)
#endif

static SizeHead SizeData[fxfMAXSIZE+1];

#if defined(SEGMENTED)
/* #define  ARENA_SEG_SIZE  32000 */
/* #define  ARENA_SEG_COUNT  ((1024*1024)/ARENA_SEG_SIZE) */
static char *Arena[ARENA_SEG_COUNT] = { Nil(char) };
static int ArenaSegCnt= 0;
static int ActualSeg= 0;
#else
static char *Arena= Nil(char);
#endif /*SEGMENTED*/

static unsigned long GlobalSize;
static char *BotFreePtr;
static char *TopFreePtr;

#undef FREEMAP


#if defined(FREEMAP) && !defined(SEGMENTED)
static unsigned int *FreeMap;
#define  Bit(x)    (1<<((x)&31))
#define  LeftMask(x)  (-1<<((x)&31))
#define  SetFreeBit(x)  FreeMap[(x)>>5]|= Bit(x)
#define ClrFreeBit(x)  FreeMap[(x)>>5]&= ~Bit(x)
#define  MAC_SetRange(x,l)  {                       \
    int xi= (x)>>5, y= x+l, yi= (y)>>5;             \
    if (xi==yi)                                     \
      FreeMap[xi]|= LeftMask(x) & (~LeftMask(y));   \
    else {                                          \
      int i;                                        \
      FreeMap[xi]|= LeftMask(x);                    \
      FreeMap[yi]|= ~LeftMask(y);                   \
      yi--;                                         \
      for (i=xi+1; i<yi; i++)                       \
        FreeMap[i]= -1;                             \
    }                                               \
  }
#define  MAC_ClrRange(x,l)  {                       \
    int xi= (x)>>5, y= x+l, yi= (y)>>5;             \
    if (xi==yi)                                     \
      FreeMap[xi]&= (~LeftMask(x)) | LeftMask(y);   \
    else {                                          \
      int i;                                        \
      FreeMap[xi]&= ~LeftMask(x);                   \
      FreeMap[yi]&= LeftMask(y);                    \
      yi--;                                         \
      for (i=xi+1; i<yi; i++)                       \
        FreeMap[i]= 0;                              \
    }                                               \
  }

void  SetRange(int x, int l)  {
  int xi= (x)>>5, y= x+l, yi= (y)>>5;
  if (xi==yi)
    FreeMap[xi]|= LeftMask(x) & (~LeftMask(y));
  else {
    int i;
    FreeMap[xi]|= LeftMask(x);
    FreeMap[yi]|= ~LeftMask(y);
    yi--;
    for (i=xi+1; i<yi; i++)
      FreeMap[i]= -1;
  }
}

void ClrRange(int x, int l)  {
  int xi= (x)>>5, y= x+l, yi= (y)>>5;
  if (xi==yi)
    FreeMap[xi]&= (~LeftMask(x)) | LeftMask(y);
  else {
    int i;
    FreeMap[xi]&= ~LeftMask(x);
    FreeMap[yi]&= LeftMask(y);
    yi--;
    for (i=xi+1; i<yi; i++)
      FreeMap[i]= 0;
  }
}

void PrintFreeMap(FILE *f) {
  int i;
  for (i=0; i<GlobalSize; i++) {
    if (i % 80 == 0)
      fputc('\n', f);
    if (FreeMap[i>>5]&Bit(i)) {
      fputc('.', f);
    }
    else
      fputc(' ', f);
  }
}
#else
#  define  SetRange(x,l)
#  define  ClrRange(x,l)
#endif /*FREEMAP, !SEGMENTED*/

int fxfInit(unsigned long Size) {
  static char *myname= "fxfInit";
#if defined(SEGMENTED)
  unsigned long asize;
  while (ArenaSegCnt > 0) {
    ArenaSegCnt--;
    free(Arena[ArenaSegCnt]);
  }
  asize= Size+ ARENA_SEG_SIZE;
  while (asize > ARENA_SEG_SIZE) {
    if ((Arena[ArenaSegCnt++]=nNew(ARENA_SEG_SIZE, char)) == Nil(char))
      break;
    if (ArenaSegCnt >= ARENA_SEG_COUNT) {
      ERROR_LOG3("%s: whats going on here?\nCannot believe in more than %s on %s\n",
                 myname, OSMAXMEM, OSNAME);
      exit(2);
    }
    asize-= ARENA_SEG_SIZE;
  }
  ActualSeg= 0;
  BotFreePtr= Arena[ActualSeg];
  TopFreePtr= Arena[ActualSeg]+ARENA_SEG_SIZE;
  GlobalSize= ArenaSegCnt*ARENA_SEG_SIZE;
#else
  if (Arena)
    free(Arena);
  if ((Arena=nNew(Size, char)) == Nil(char)) {
    ERROR_LOG2("%s: Sorry, cannot allocate arena of %ld bytes\n",
               myname, Size);
    BotFreePtr= Arena;
    GlobalSize= 0;
    return -1;
  }
  BotFreePtr= Arena;
  TopFreePtr= Arena+Size;
  GlobalSize= Size;
#endif /*SEGMENTED*/

#if defined(FREEMAP) && !defined(SEGMENTED)
  if (FreeMap) {
    free(FreeMap);
  }
  FreeMap= nNew((Size+31)>>5, unsigned int);
  memset(FreeMap, 0, Size>>3);
#endif /*FREEMAP, !SEGMENTED*/

  memset(SizeData, '\0', sizeof(SizeData));

  return 0;
}

/* we have to define the following, since some architectures cannot
 * access words on unaligned addresses. Most probably the
 * case if you are compiling on any modern risc architecture:
 * SPARC, HPPA, MIPS. We wouldn't need this when running on an
 * Intel *86 type of CPU, but also there, aligned access is faster.
 */
#define PTRMASK            (sizeof(char *)-1)
#define ALIGNED_MINSIZE    (sizeof(char *)+PTRMASK)
#define ALIGN(ptr)         (((unsigned long)ptr+PTRMASK) & (~PTRMASK))

#define  GetNextPtr(ptr)       *(char **)ALIGN(ptr)
#define  PutNextPtr(dst, ptr)  *(char **)ALIGN(dst)= ptr

void *fxfAlloc(size_t size) {
  static char *myname= "fxfAlloc";
  SizeHead *sh;
  char *ptr;

  DBG((stderr, "%s(%d) =", myname, size));
  if (size < fxfMINSIZE) {
#if !defined(SIXTYFOUR)
    WARN_LOG3("%s: size=%d < %d\n", myname, size, fxfMINSIZE);
#endif
    size= fxfMINSIZE;
  }
  if (size > fxfMAXSIZE) {
    ERROR_LOG3("%s: size=%d > %d\n", myname, size, fxfMAXSIZE);
    return Nil(char);
  }
  if ( (size&PTRMASK) && size<ALIGNED_MINSIZE)
    size= ALIGNED_MINSIZE;

  sh= &SizeData[size];
  if (sh->FreeHead) {
    ptr= sh->FreeHead;
    sh->FreeHead= GetNextPtr(ptr);
    sh->FreeCount--;
    sh->MallocCount++;
    ClrRange((char *)ptr-Arena, size);
  }
  else {
    /* we have to allocate a new piece */
    if ((unsigned long)(TopFreePtr-BotFreePtr) >= size) {
      if (size&PTRMASK) {
        /* not aligned */
        ptr= BotFreePtr;
        BotFreePtr+= size;
      }
      else {
        /* aligned */
        ptr= TopFreePtr-= size;
      }
      sh->MallocCount++;
    }
    else
#if defined(SEGMENTED)
      if ((ActualSeg+1) < ArenaSegCnt) {
        ActualSeg+= 1;
        BotFreePtr= Arena[ActualSeg];
        TopFreePtr= Arena[ActualSeg]+ARENA_SEG_SIZE;
        ptr= fxfAlloc(size);
      }
      else
        ptr= Nil(char);
#else /*SEGMENTED*/
    ptr= Nil(char);
#endif /*!SEGMENTED*/
  }
  DBG((df, "0x%08x\n", ptr));
  return ptr;
}

void fxfFree(void *ptr, size_t size) {
  static char *myname= "fxfFree";
  SizeHead *sh;

  DBG((df, "%s(0x%08x, %d)\n", myname, ptr, size));
  if (size > fxfMAXSIZE) {
    fprintf(stderr, "%s: size=%d >= %d\n",
            myname, size, fxfMAXSIZE);
    exit(-5);
  }
  if (size < fxfMINSIZE)
    size= fxfMINSIZE;
  if ((size&PTRMASK) && size<ALIGNED_MINSIZE)
    size= ALIGNED_MINSIZE;
  sh= &SizeData[size];
  if (size&PTRMASK) {
    /* unaligned size */
    if ((char *)ptr+size == BotFreePtr) {
      BotFreePtr-= size;
      sh->MallocCount-= 1;
    }
    else {
      SetRange((char *)ptr-Arena,size);
      *(char **)ALIGN(ptr)= sh->FreeHead;
      sh->FreeHead= ptr;
      sh->FreeCount+= 1;
      sh->MallocCount-= 1;
    }
  }
  else {
    /* aligned size */
    if ((char *)ptr == TopFreePtr) {
      TopFreePtr+= size;
      sh->MallocCount-= 1;
    }
    else {
      SetRange((char *)ptr-Arena,size);
      *(char **)ptr= sh->FreeHead;
      sh->FreeHead= ptr;
      sh->FreeCount+= 1;
      sh->MallocCount-= 1;
    }
  }
}

void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize) {
  void *nptr= fxfAlloc(NewSize);
  memcpy(nptr, ptr, OldSize);
  fxfFree(ptr, OldSize);
  return nptr;
}

unsigned long fxfTotal() {
  int i;
  SizeHead *hd;
  unsigned long UsedBytes, FreeBytes;

  hd= SizeData;
  UsedBytes= FreeBytes= 0;
  for (i=0; i<=fxfMAXSIZE; i++,hd++) {
    if (hd->MallocCount + hd->FreeCount) {
      UsedBytes+= hd->MallocCount*i;
      FreeBytes+= hd->FreeCount*i;
    }
  }
  return UsedBytes+FreeBytes;
}

void fxfInfo(FILE *f) {
  int i, n;
  SizeHead *hd;
  unsigned long Used, Free;
  unsigned long UsedBytes, FreeBytes;
  fprintf(f, "fxfArenaSize = %lu bytes\n", GlobalSize);
  fprintf(f, "fxfArenaUsed = %lu bytes\n",
          GlobalSize-(unsigned long)(TopFreePtr-BotFreePtr)
#if defined(SEGMENTED)
          - (ArenaSegCnt-ActualSeg-1)*ARENA_SEG_SIZE
#endif /*SEGMENTED*/
    );
  fprintf(f, "fxfMAXSIZE   = %d bytes\n", fxfMAXSIZE);
  fprintf(f, "%12s  %10s%10s\n", "Size", "MallocCnt", "FreeCnt");
  UsedBytes= FreeBytes= Used= Free= 0;
  hd= SizeData;
  for (n=i=0; i<=fxfMAXSIZE; i++,hd++) {
    if (hd->MallocCount + hd->FreeCount) {
      fprintf(f, "%12d  %10ld%10ld\n", i,
              hd->MallocCount, hd->FreeCount);
      Used+= hd->MallocCount;
      UsedBytes+= hd->MallocCount*i;
      Free+= hd->FreeCount;
      FreeBytes+= hd->FreeCount*i;
      n++;
    }
  }
  fprintf(f, "%12s  %10lu%10lu\n", "Total:", Used, Free);
  fprintf(f, "%12s  %10lu%10lu\n", "Total Bytes:", UsedBytes, FreeBytes);
}

#endif /*FXF*/
