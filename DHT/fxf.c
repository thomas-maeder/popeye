#if defined (FXF)

#include "debugging/assert.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(__TURBOC__)
#  include <mem.h>
#else
#  include <memory.h>
#  if defined(DBMALLOC)
#    include <dbmalloc.h>
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

#if defined(LOG)
#  define ERROR_LOG2(s,a,b)   fprintf(stderr,s,a,b)
#  define ERROR_LOG3(s,a,b,c) fprintf(stderr,s,a,b,c)
#  define WARN_LOG3(s,a,b,c)  fprintf(stderr,s,a,b,c)
#else
#  define ERROR_LOG2(s,a,b)
#  define ERROR_LOG3(s,a,b,c)
#  define WARN_LOG3(s,a,b,c)
#endif /*!ERROR_LOGS*/

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || /* >= C99   -- We have printf ptrdiff_t/size_t specifiers. */ \
    (defined(__cplusplus) && (__cplusplus >= 201103L))              /* >= C++11 -- We have printf ptrdiff_t/size_t specifiers. */
#define PTRDIFF_T_PRINTF_SPECIFIER "td"
#define PTRDIFF_T_TO_PRINTF_TYPE(x) (x)
#define SIZE_T_PRINTF_SPECIFIER "zu"
#define SIZE_T_TO_PRINTF_TYPE(x) (x)
#elif defined(LLONG_MAX) /* We have long long integer types. */
#define PTRDIFF_T_PRINTF_SPECIFIER "lld"
#define PTRDIFF_T_TO_PRINTF_TYPE(x) ((long long int) (x))
#define SIZE_T_PRINTF_SPECIFIER "llu"
#define SIZE_T_TO_PRINTF_TYPE(x) ((unsigned long long int) (x))
#else /* We don't have long long integer types. */
#define PTRDIFF_T_PRINTF_SPECIFIER "ld"
#define PTRDIFF_T_TO_PRINTF_TYPE(x) ((long int) x)
#define SIZE_T_PRINTF_SPECIFIER "lu"
#define SIZE_T_TO_PRINTF_TYPE(x) ((unsigned long int) x)
#endif


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
#define fxfMAXSIZE  (size_t)1024
#else
#define fxfMAXSIZE  (size_t)2048  /* this is needed only when sizeof(void*)==8 */
#endif

/* Different size of fxfMINSIZE for 32-/64/Bit compilation */
enum
{
  fxfMINSIZE = sizeof(size_t)
};

static SizeHead SizeData[fxfMAXSIZE+1];

#if defined(SEGMENTED)
/* #define  ARENA_SEG_SIZE  32000 */
/* #define  ARENA_SEG_COUNT  ((1024*1024)/ARENA_SEG_SIZE) */
static char *Arena[ARENA_SEG_COUNT] = { Nil(char) };
static int ArenaSegCnt= 0;
static int CurrentSeg= 0;
#else
static char *Arena= Nil(char);
#endif /*SEGMENTED*/

static size_t GlobalSize;
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

int fxfInit(size_t Size) {
#if defined(LOG)
  static char const * const myname= "fxfInit";
#endif
#if defined(SEGMENTED)
  size_t asize= Size+ARENA_SEG_SIZE;
  while (ArenaSegCnt > 0) {
    ArenaSegCnt--;
    free(Arena[ArenaSegCnt]);
  }
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
  CurrentSeg= 0;
  BotFreePtr= Arena[CurrentSeg];
  TopFreePtr= Arena[CurrentSeg]+ARENA_SEG_SIZE;
  GlobalSize= ArenaSegCnt*ARENA_SEG_SIZE;
#else
  if (Arena)
    free(Arena);
  if ((Arena=nNew(Size, char)) == Nil(char)) {
    ERROR_LOG2("%s: Sorry, cannot allocate arena of %" SIZE_T_PRINTF_SPECIFIER " bytes\n",
               myname, SIZE_T_TO_PRINTF_TYPE(Size));
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

int fxfInitialised(void)
{
#if defined(SEGMENTED)
  return Arena[0]!=0;
#else
  return Arena!=0;
#endif
}

void fxfReset(void)
{
#if defined(SEGMENTED)
  CurrentSeg= 0;
  BotFreePtr= Arena[CurrentSeg];
  TopFreePtr= Arena[CurrentSeg]+ARENA_SEG_SIZE;
#else
  BotFreePtr= Arena;
  TopFreePtr= Arena+GlobalSize;

#if defined(FREEMAP)
  memset(FreeMap, 0, GlobalSize>>3);
#endif /*FREEMAP*/
#endif /*SEGMENTED*/

#if !defined(NDEBUG)
  {
    unsigned int i;
    for (i = 1; i<=50; ++i)
      assert(SizeData[i].MallocCount==0);
  }
#endif

  memset(SizeData, '\0', sizeof SizeData);
}

/* we have to define the following, since some architectures cannot
 * access words on unaligned addresses. Most probably the
 * case if you are compiling on any modern risc architecture:
 * SPARC, HPPA, MIPS. We wouldn't need this when running on an
 * Intel *86 type of CPU, but also there, aligned access is faster.
 */
#define PTRMASK            (sizeof(char *)-1)
#define ALIGNED_MINSIZE    (sizeof(char *)+PTRMASK)
#define ALIGN(ptr)         (((size_t)ptr+PTRMASK) & (~PTRMASK))

#define  GetNextPtr(ptr)       *(char **)ALIGN(ptr)
#define  PutNextPtr(dst, ptr)  *(char **)ALIGN(dst)= ptr

#define TMDBG(x) {}

void *fxfAlloc(size_t size) {
#if defined(LOG) || defined(DEBUG)
  static char const * const myname= "fxfAlloc";
#endif
  SizeHead *sh;
  char *ptr;

  TMDBG(printf("fxfAlloc - size:%" SIZE_T_PRINTF_SPECIFIER,SIZE_T_TO_PRINTF_TYPE(size)));
  DBG((stderr, "%s(%u) =", myname, (unsigned int)size));

  if (size<fxfMINSIZE)
    size = fxfMINSIZE;

  if (size>fxfMAXSIZE)
  {
    ERROR_LOG3("%s: size=%u > %u\n",
               myname,
               (unsigned int)size,
               (unsigned int)fxfMAXSIZE);
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
    TMDBG(printf(" FreeCount:%lu ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",sh->FreeCount,PTRDIFF_T_TO_PRINTF_TYPE((char*)ptr-Arena),sh->MallocCount));
  }
  else {
    /* we have to allocate a new piece */
    size_t const sizeCurrentSeg = TopFreePtr-BotFreePtr;
    TMDBG(printf(" sizeCurrentSeg:%" SIZE_T_PRINTF_SPECIFIER,SIZE_T_TO_PRINTF_TYPE(sizeCurrentSeg)));
    if (sizeCurrentSeg>=size) {
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
      TMDBG(printf(" current seg ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",PTRDIFF_T_TO_PRINTF_TYPE((char*)ptr-Arena),sh->MallocCount));
    }
    else
    {
#if defined(SEGMENTED)
      if ((CurrentSeg+1) < ArenaSegCnt) {
        TMDBG(printf(" next seg"));
        CurrentSeg+= 1;
        BotFreePtr= Arena[CurrentSeg];
        TopFreePtr= Arena[CurrentSeg]+ARENA_SEG_SIZE;
        ptr= fxfAlloc(size);
      }
      else
        ptr= Nil(char);
#else /*SEGMENTED*/
      ptr= Nil(char);
#endif /*!SEGMENTED*/
      TMDBG(printf(" ptr:%p\n",(void *)ptr));
    }
  }
  DBG((df, "%p\n", (void *) ptr));
  return ptr;
}

void fxfFree(void *ptr, size_t size) {
  static char const * const myname= "fxfFree";
  SizeHead *sh;

  TMDBG(printf("fxfFree - ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,PTRDIFF_T_TO_PRINTF_TYPE((char*)ptr-Arena),SIZE_T_TO_PRINTF_TYPE(size)));
  DBG((df, "%s(%p, %u)\n", myname, ptr, (unsigned int)size));
  if (size > fxfMAXSIZE) {
    fprintf(stderr, "%s: size=%u >= %u\n",
            myname, (unsigned int)size, (unsigned int)fxfMAXSIZE);
    exit(-5);
  }
  if (size < fxfMINSIZE)
    size= fxfMINSIZE;
  if ((size&PTRMASK) && size<ALIGNED_MINSIZE)
    size= ALIGNED_MINSIZE;
  sh= &SizeData[size];
  if (size&PTRMASK) {
    /* unaligned size */
    TMDBG(printf(" BotFreePtr-ptr:%" PTRDIFF_T_PRINTF_SPECIFIER,PTRDIFF_T_TO_PRINTF_TYPE(BotFreePtr-(char *)ptr)));
    if ((char *)ptr+size == BotFreePtr) {
      BotFreePtr-= size;
      TMDBG(printf(" BotFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,PTRDIFF_T_TO_PRINTF_TYPE(TopFreePtr-BotFreePtr)));
      sh->MallocCount-= 1;
    }
    else {
      SetRange((char *)ptr-Arena,size);
      *(char **)ALIGN(ptr)= sh->FreeHead;
      sh->FreeHead= ptr;
      sh->FreeCount+= 1;
      sh->MallocCount-= 1;
      TMDBG(printf(" FreeCount:%lu",sh->FreeCount));
    }
  }
  else {
    /* aligned size */
    TMDBG(printf(" ptr-TopFreePtr:%" PTRDIFF_T_PRINTF_SPECIFIER,PTRDIFF_T_TO_PRINTF_TYPE((char *)ptr-TopFreePtr)));
    if ((char *)ptr == TopFreePtr) {
      TopFreePtr+= size;
      TMDBG(printf(" TopFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,PTRDIFF_T_TO_PRINTF_TYPE(TopFreePtr-BotFreePtr)));
      sh->MallocCount-= 1;
    }
    else {
      SetRange((char *)ptr-Arena,size);
      *(char **)ptr= sh->FreeHead;
      sh->FreeHead= ptr;
      sh->FreeCount+= 1;
      sh->MallocCount-= 1;
      TMDBG(printf(" FreeCount:%lu",sh->FreeCount));
    }
  }
  TMDBG(printf(" MallocCount:%lu",sh->MallocCount));
  TMDBG(printf("\n"));
}

void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize) {
  void *nptr= fxfAlloc(NewSize);
  memcpy(nptr, ptr, OldSize);
  fxfFree(ptr, OldSize);
  return nptr;
}

size_t fxfTotal() {
  SizeHead const *hd = SizeData;
  size_t UsedBytes = 0;
  size_t FreeBytes = 0;

  unsigned int i;
  for (i=0; i<=fxfMAXSIZE; i++,hd++) {
    if (hd->MallocCount+hd->FreeCount>0) {
      UsedBytes+= hd->MallocCount*i;
      FreeBytes+= hd->FreeCount*i;
    }
  }

  return UsedBytes+FreeBytes;
}

void fxfInfo(FILE *f) {
  size_t const one_kilo = 1<<10;
  size_t const sizeCurrentSeg = (TopFreePtr-BotFreePtr);
  size_t const sizeArenaUsed =
          GlobalSize-sizeCurrentSeg
#if defined(SEGMENTED)
          - (ArenaSegCnt-CurrentSeg-1)*ARENA_SEG_SIZE
#endif /*SEGMENTED*/
      ;
  assert(GlobalSize/one_kilo<=ULONG_MAX);
  fprintf(f, "fxfArenaSize = %lu kB\n",
          (unsigned long)(GlobalSize/one_kilo));
  assert(sizeArenaUsed/one_kilo<=ULONG_MAX);
  fprintf(f, "fxfArenaUsed = %lu kB\n",
          (unsigned long)(sizeArenaUsed/one_kilo));
  fprintf(f, "fxfMAXSIZE   = %u B\n", (unsigned int)fxfMAXSIZE);

  {
    SizeHead const *hd = SizeData;
    unsigned long nrUsed = 0;
    unsigned long nrFree = 0;
    size_t UsedBytes = 0;
    size_t FreeBytes = 0;

    unsigned int i;
    fprintf(f, "%12s  %10s%10s\n", "Size", "MallocCnt", "FreeCnt");
    for (i=0; i<=fxfMAXSIZE; i++,hd++) {
      if (hd->MallocCount+hd->FreeCount>0) {
        fprintf(f, "%12u  %10lu%10lu\n", i, hd->MallocCount, hd->FreeCount);
        nrUsed+= hd->MallocCount;
        UsedBytes+= hd->MallocCount*i;
        nrFree+= hd->FreeCount;
        FreeBytes+= hd->FreeCount*i;
      }
    }
    fprintf(f, "%12s  %10lu%10lu\n", "Total:", nrUsed, nrFree);
    assert(UsedBytes/one_kilo<=ULONG_MAX);
    assert(FreeBytes/one_kilo<=ULONG_MAX);
    fprintf(f, "%12s  %10lu%10lu\n", "Total kB:",
            (unsigned long)(UsedBytes/one_kilo),
            (unsigned long)(FreeBytes/one_kilo));
  }
}

#endif /*FXF*/
