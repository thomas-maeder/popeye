#if defined (FXF)

#include "debugging/assert.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__TURBOC__)
#  include <mem.h>
#else
#  include <memory.h>
#  if defined(DBMALLOC)
#    include <dbmalloc.h>
#  endif /*DBMALLOC*/
#endif /*__TURBOC__*/

#include "fxf.h"

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || /* >= C99   -- We have printf ptrdiff_t/size_t specifiers. */ \
    (defined(__cplusplus) && (__cplusplus >= 201103L))              /* >= C++11 -- We have printf ptrdiff_t/size_t specifiers. */
#  include <stddef.h>
#  include <inttypes.h>
   typedef ptrdiff_t ptrdiff_t_printf_type;
   typedef size_t size_t_printf_type;
#  define MAX_HASHTABLE_SIZE PTRDIFF_MAX
#    if defined(UINTPTR_MAX)
   typedef uintptr_t pointer_to_int_type;
#    else
   typedef uintmax_t pointer_to_int_type;
#    endif
#  define PTRDIFF_T_PRINTF_SPECIFIER "td"
#  define SIZE_T_PRINTF_SPECIFIER "zu"
#elif defined(LLONG_MAX) /* We have long long integer types. */
   typedef long long int ptrdiff_t_printf_type;
   typedef unsigned long long int size_t_printf_type;
   typedef unsigned long long int pointer_to_int_type;
#  define MAX_HASHTABLE_SIZE LLONG_MAX
#  define PTRDIFF_T_PRINTF_SPECIFIER "lld"
#  define SIZE_T_PRINTF_SPECIFIER "llu"
#else /* We don't have long long integer types. */
   typedef long int ptrdiff_t_printf_type;
   typedef unsigned long int size_t_printf_type;
   typedef unsigned long int pointer_to_int_type;
#  define MAX_HASHTABLE_SIZE LONG_MAX
#  define PTRDIFF_T_PRINTF_SPECIFIER "ld"
#  define SIZE_T_PRINTF_SPECIFIER "lu"
#endif

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#  define MAX_ALIGNMENT _Alignof(max_align_t)
#elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#  define MAX_ALIGNMENT alignof(max_align_t)
#else
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#    include <stdint.h>
#  endif
union MAX_ALIGNED_TYPE {
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  uintmax_t unsigned_integer;
#  elif defined(LLONG_MAX) /* We have long long integer types. */
  unsigned long long int unsigned_integer;
#  else
  unsigned long int unsigned_integer;
#  endif
  const volatile void * object_pointer;
  void (*function_pointer)(void);  
  long double floating_point;
};
#  define MAX_ALIGNMENT (sizeof(union MAX_ALIGNED_TYPE) & -sizeof(union MAX_ALIGNED_TYPE))
#endif

#if !defined(Nil) && !defined(New) && !defined(nNewUntyped) && !defined(nNewCallocUntyped) /* TODO: Is this the correct check for all of the below lines? */
#  define Nil(type)      ((type *)0)
#  define New(type)      ((type *)malloc(sizeof(type)))
#  define nNewUntyped(n, type)  nNewImpl(n,sizeof(type))
#  define nNewCallocUntyped(n, type)  calloc(n,sizeof(type))
static inline void * nNewImpl(size_t const nmemb, size_t const size) {
  return ((size && (nmemb > (((size_t)-1)/size))) ? Nil(void) : malloc(nmemb*size));
}
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



/* FiXed and Fast malloc, free
 * As the name tells: this code implements on top of traditional
 * malloc/realloc/free a fast version, that relies on a lot of
 * allocation/deallocation of fixed sized blocks of memory. For
 * each size of memory we keep a head pointer and all freed chunks
 * of memory is threaded on this list. If memory of this size
 * is requested, we drag it from the list, otherwise we carve it
 * from larger preallocated (via malloc) chunks of memory. If
 * the sizes between allocation and deallocation vary too often,
 * this package is lost in fragmented memory.
 */

typedef struct {
    unsigned long  MallocCount;
    unsigned long  FreeCount;
    void *    FreeHead;
} SizeHead;

#if defined(DOS)
/* MSDOS 16 Bit support (maxmemory <= 1 MB) */
#define SEGMENTED
#define ARENA_SEG_SIZE  (32000 & ~(MAX_ALIGNMENT - 1U))
#define ARENA_SEG_COUNT ((1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "MSDOS"
#define OSMAXMEM "1 MB"
#elif defined(_WIN98)
/* Win95/Win98/WinME can only allocate chunks up to 255 MB */
/* maxmemory <= 768 MB */
#define SEGMENTED
#define ARENA_SEG_SIZE  (1000000 & ~(MAX_ALIGNMENT - 1U))
#define ARENA_SEG_COUNT ((768*1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "Win95/Win98/WinME"
#define OSMAXMEM "768 MB"
#endif

/* The maximum size an fxfAlloc can handle */
#if defined(SEGMENTED) || defined(__TURBOC__)
#define fxfMAXSIZE  (((size_t)1024) & ~(MAX_ALIGNMENT - 1U))
#else
#define fxfMAXSIZE  (((size_t)2048) & ~(MAX_ALIGNMENT - 1U))  /* this is needed only when sizeof(void*)==8 */
#endif

/* Different size of fxfMINSIZE for 32-/64/Bit compilation */
enum
{
  fxfMINSIZE = sizeof(void *)
};
#define BOTTOM_BIT_OF_FXFMINSIZE ((size_t)fxfMINSIZE & -(size_t)fxfMINSIZE)
#define MIN_ALIGNMENT_UNDERESTIMATE ((BOTTOM_BIT_OF_FXFMINSIZE > MAX_ALIGNMENT) ? MAX_ALIGNMENT : BOTTOM_BIT_OF_FXFMINSIZE) /* We'd prefer the top bit, but we'll compute that during fxfInit.
                                                                                                                               (Of course, they're probably the same.)
                                                                                                                               TODO: Can we compute what we want at compile time and just use it? */
static size_t MIN_ALIGNMENT= MIN_ALIGNMENT_UNDERESTIMATE; /* for now */

static SizeHead SizeData[1 + (fxfMAXSIZE - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE]; /* Minimum allocation is (fxfMINSIZE + (MIN_ALIGNMENT_UNDERESTIMATE - 1U)) & ~(MIN_ALIGNMENT_UNDERESTIMATE - 1U).
                                                                                        Maximum allocation is fxfMAXSIZE.
                                                                                        All allocations will be multiples of MIN_ALIGNMENT_UNDERESTIMATE. */

#if defined(SEGMENTED)
/* #define  ARENA_SEG_SIZE  32000 */
/* #define  ARENA_SEG_COUNT  ((1024*1024)/ARENA_SEG_SIZE) */
static void *Arena[ARENA_SEG_COUNT]= { Nil(void) };
static int ArenaSegCnt= 0;
static int CurrentSeg= 0;
#else
static void *Arena= Nil(void);
#endif /*SEGMENTED*/

static size_t GlobalSize;
static void *BotFreePtr;
static void *TopFreePtr;

#undef FREEMAP


#if defined(FREEMAP) && !defined(SEGMENTED)
#if UINT_MAX < 0xFFFFFFFFU
typedef unsigned long int FreeMapType;
#else
typedef unsigned int FreeMapType;
#endif /*UINT_MAX < 0xFFFFFFFFU*/
static FreeMapType *FreeMap;
#define  Bit(x)    (((FreeMapType)1)<<((x)&31))
#define  LeftMask(x)  (-Bit(x))
#define  SetFreeBit(x)  (FreeMap[(x)>>5]|= Bit(x))
#define  ClrFreeBit(x)  (FreeMap[(x)>>5]&= ~Bit(x))
#define  MAC_SetRange(x,l)  do {                      \
    size_t z= (x), y= z+(l);                          \
    if (FreeMap) {                                    \
      size_t xi= z>>5, yi= y>>5;                      \
      if (xi==yi)                                     \
        FreeMap[xi]|= (LeftMask(z) & ~LeftMask(y));   \
      else {                                          \
        FreeMap[xi]|= LeftMask(z);                    \
        FreeMap[yi]|= ~LeftMask(y);                   \
        while (yi > ++xi)                             \
          FreeMap[xi]= -1;                            \
      }                                               \
    }                                                 \
  } while (0)
#define  MAC_ClrRange(x,l)  do {                      \
    size_t z= (x), y= z+(l);                          \
    if (FreeMap) {                                    \
      size_t xi= z>>5, yi= y>>5;                      \
      if (xi==yi)                                     \
        FreeMap[xi]&= (LeftMask(y) | ~LeftMask(z));   \
      else {                                          \
        FreeMap[xi]&= ~LeftMask(z);                   \
        FreeMap[yi]&= LeftMask(y);                    \
        while (yi > ++xi)                             \
          FreeMap[xi]= 0;                             \
      }                                               \
    }                                                 \
  } while (0)

static void SetRange(size_t x, size_t l)  {
  if (FreeMap) {
    size_t xi= x>>5, y= x+l, yi= y>>5;
    if (xi==yi)
      FreeMap[xi]|= (LeftMask(x) & ~LeftMask(y));
    else {
      FreeMap[xi]|= LeftMask(x);
      FreeMap[yi]|= ~LeftMask(y);
      while (yi > ++xi)
        FreeMap[xi]= -1;
    }
  }
}

static void ClrRange(size_t x, size_t l)  {
  if (FreeMap) {
    size_t xi= x>>5, y= x+l, yi= y>>5;
    if (xi==yi)
      FreeMap[xi]&= (LeftMask(y) | ~LeftMask(x));
    else {
      FreeMap[xi]&= ~LeftMask(x);
      FreeMap[yi]&= LeftMask(y);
      while (yi > ++xi)
        FreeMap[xi]= 0;
    }
  }
}

void PrintFreeMap(FILE *f) {
  size_t i;
  for (i=0; i<GlobalSize; ++i) {
    if ((i % 80) == 0)
      fputc('\n', f);
    if (FreeMap) {
      if (FreeMap[i>>5]&Bit(i)) {
        fputc('.', f);
      }
      else
        fputc(' ', f);
    } else
      fputc('?', f);
  }
}
#endif /*FREEMAP, !SEGMENTED*/

static ptrdiff_t_printf_type pointerDifference(void const *ptr1, void const *ptr2) {
  return (((char const *)ptr1) - ((char const *)ptr2));
}

static void * stepPointer(void *ptr, ptrdiff_t_printf_type step) {
  return (void *)(((char *)ptr) + step);
}

size_t fxfInit(size_t Size) {
#if defined(LOG)
  static char const * const myname= "fxfInit";
#endif
#if defined(SEGMENTED)
  size_t maxSegCnt;
#endif
  if (Size > MAX_HASHTABLE_SIZE)
    Size = MAX_HASHTABLE_SIZE;
#if defined(SEGMENTED)
  maxSegCnt= (Size / ARENA_SEG_SIZE);
  if (maxSegCnt > ARENA_SEG_COUNT)
    maxSegCnt= ARENA_SEG_COUNT;
  while (ArenaSegCnt > maxSegCnt) {
    --ArenaSegCnt;
    free(Arena[ArenaSegCnt]);
    Arena[ArenaSegCnt]= Nil(void);
  }
  while (ArenaSegCnt < maxSegCnt) {
    if (!(Arena[ArenaSegCnt]= nNewUntyped(ARENA_SEG_SIZE, char)))
      break;
    ++ArenaSegCnt;
  }
  CurrentSeg= 0;
  BotFreePtr= Arena[CurrentSeg];
  TopFreePtr= Arena[CurrentSeg];
  if (TopFreePtr)
    TopFreePtr= stepPointer(TopFreePtr, ARENA_SEG_SIZE);
  GlobalSize= ArenaSegCnt*ARENA_SEG_SIZE;
#else
#if defined(FREEMAP)
  if (FreeMap)
  {
    free(FreeMap);
    FreeMap= Nil(FreeMapType);
  }
#endif
  if (Arena)
    free(Arena);
  if (Size < MAX_ALIGNMENT)
    Size= MAX_ALIGNMENT;
  else
    Size&= ~(MAX_ALIGNMENT - 1U);
  if (!(Arena=nNewUntyped(Size, char))) {
    ERROR_LOG2("%s: Sorry, cannot allocate arena of %" SIZE_T_PRINTF_SPECIFIER " bytes\n",
               myname, (size_t_printf_type) Size);
    BotFreePtr= Arena;
    TopFreePtr= Arena;
    GlobalSize= 0;
    return GlobalSize;
  }
  BotFreePtr= Arena;
  TopFreePtr= stepPointer(Arena, (ptrdiff_t_printf_type)Size);
  GlobalSize= Size;

#if defined(FREEMAP)
  /* We aren't using Size again, so we can change it to the value we need here. */
  if (Size > (((size_t)-1)-31))
  {
    Size = (1+(((size_t)-1)>>5));
  }
  else
  {
    Size = ((Size+31)>>5); 
  }

  FreeMap= (FreeMapType *)nNewCallocUntyped(Size, FreeMapType);
#endif /*FREEMAP*/
#endif /*SEGMENTED*/

  memset(SizeData, '\0', sizeof SizeData);

  MIN_ALIGNMENT= MAX_ALIGNMENT;
  while (MIN_ALIGNMENT > fxfMINSIZE)
    MIN_ALIGNMENT>>= 1;

  return GlobalSize;
}

void fxfTeardown(void)
{
#if defined(SEGMENTED)
  while (ArenaSegCnt > 0)
  {
    --ArenaSegCnt;
    free(Arena[ArenaSegCnt]);
    Arena[ArenaSegCnt]= Nil(void);
  }
  CurrentSeg= 0;
#else
#if defined(FREEMAP)
  free(FreeMap);
  FreeMap= Nil(FreeMapType);
#endif /*FREEMAP*/
  free(Arena);
  Arena= Nil(void);
#endif /*SEGMENTED*/
  memset(SizeData, '\0', sizeof SizeData);
  GlobalSize= 0;
  TopFreePtr= Nil(void);
  BotFreePtr= Nil(void);
}

int fxfInitialised(void)
{
#if defined(SEGMENTED)
  return !!Arena[0];
#else
  return !!Arena;
#endif
}

void fxfReset(void)
{
#if defined(SEGMENTED)
  CurrentSeg= 0;
  BotFreePtr= Arena[CurrentSeg];
  TopFreePtr= Arena[CurrentSeg];
  if (TopFreePtr)
    TopFreePtr= stepPointer(TopFreePtr, ARENA_SEG_SIZE);
#else
  BotFreePtr= Arena;
  TopFreePtr= Arena;
  if (TopFreePtr)
    TopFreePtr= stepPointer(TopFreePtr, (ptrdiff_t_printf_type)GlobalSize);

#if defined(FREEMAP)
  if (FreeMap)
    memset(FreeMap, '\0', GlobalSize>>3);
#endif /*FREEMAP*/
#endif /*SEGMENTED*/

#if !defined(NDEBUG)
  {
    size_t i;
    for (i = 0; i<((sizeof SizeData)/(sizeof *SizeData)); ++i)
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
#define PTRMASK            (MAX_ALIGNMENT-1U)
#define ALIGN_TO_MINIMUM(s)  (((s) + (MIN_ALIGNMENT - 1U)) & ~(MIN_ALIGNMENT - 1U))

#define TMDBG(x) if (0) x

void *fxfAlloc(size_t size) {
#if defined(LOG) || defined(DEBUG)
  static char const * const myname= "fxfAlloc";
#endif
  SizeHead *sh;
  void *ptr= Nil(void);

  TMDBG(printf("fxfAlloc - size:%" SIZE_T_PRINTF_SPECIFIER,(size_t_printf_type)size));
  DBG((stderr, "%s(%" SIZE_T_PRINTF_SPECIFIER ") =", myname, (size_t_printf_type)size));

  if (!size)
    return Nil(void);

  if (size<fxfMINSIZE)
    size= fxfMINSIZE;

  if (size>fxfMAXSIZE)
  {
    ERROR_LOG3("%s: size=%" SIZE_T_PRINTF_SPECIFIER " > %" SIZE_T_PRINTF_SPECIFIER "\n",
               myname,
               (size_t_printf_type) size,
               (size_t_printf_type) fxfMAXSIZE);
    return Nil(void);
  }

  // Round up to a multiple of MIN_ALIGNMENT
  size= ALIGN_TO_MINIMUM(size);
  sh= &SizeData[(size - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE];
  if (sh->FreeHead) {
#if defined(SEGMENTED)
    int ptrSegment;
    ptrdiff_t_printf_type ptrIndex= -1; 
#endif
    ptr= sh->FreeHead;
    if (size < sizeof sh->FreeHead)
      sh->FreeHead= Nil(void);
    else
      memcpy(&sh->FreeHead, ptr, sizeof sh->FreeHead);
    sh->FreeCount--;
    sh->MallocCount++;
#if defined(SEGMENTED)
    for (ptrSegment= CurrentSeg; ptrSegment >= 0; --ptrSegment) {
      pointer_to_int_type tmp= (pointer_to_int_type)ptr;
      pointer_to_int_type segment_begin= (pointer_to_int_type)Arena[ptrSegment];
      if ((tmp >= segment_begin) && ((tmp - segment_begin) < ARENA_SEG_SIZE)) {
        ptrIndex= (tmp - segment_begin);
        break;
      }
    }
    TMDBG(printf(" FreeCount:%lu ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",sh->FreeCount,ptrSegment,ptrIndex,sh->MallocCount));
#else
#  if defined(FREEMAP)
    ClrRange(pointerDifference(ptr, Arena), size);
#  endif
    TMDBG(printf(" FreeCount:%lu ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",sh->FreeCount,pointerDifference(ptr, Arena),sh->MallocCount));
#endif
  }
  else {
    /* we have to allocate a new piece */
    size_t sizeCurrentSeg;
#if defined(SEGMENTED)
START_LOOKING_FOR_CHUNK:
#endif
    sizeCurrentSeg = (size_t)pointerDifference(TopFreePtr,BotFreePtr);
    TMDBG(printf(" sizeCurrentSeg:%" SIZE_T_PRINTF_SPECIFIER,(size_t_printf_type)sizeCurrentSeg));
    if (sizeCurrentSeg>=size) {
      if (size&PTRMASK) {
        /* not fully aligned */
        size_t needed_alignment_mask= PTRMASK;
        while (needed_alignment_mask >= size)
          needed_alignment_mask>>= 1;
#if defined(SEGMENTED)
        size_t curBottomIndex= (size_t)pointerDifference(BotFreePtr,Arena[CurrentSeg]);
#else
        size_t curBottomIndex= (size_t)pointerDifference(BotFreePtr,Arena);
#endif
        if (curBottomIndex & needed_alignment_mask) {
          size_t const numBytesToAdd= (needed_alignment_mask - (curBottomIndex & needed_alignment_mask)) + 1U;
          if (numBytesToAdd > (sizeCurrentSeg-size))
            goto NEXT_SEGMENT;
          do {
            size_t const cur_alignment= (curBottomIndex & -curBottomIndex);
#if defined(FREEMAP) && !defined(SEGMENTED)
            SetRange(curBottomIndex,cur_alignment);
#endif
            if (cur_alignment >= fxfMINSIZE) {
              SizeHead *cur_sh= &SizeData[(cur_alignment - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE];
              if ((cur_alignment >= sizeof cur_sh->FreeHead) || !cur_sh->FreeCount) {
                if (cur_alignment >= sizeof cur_sh->FreeHead)
                  memcpy(BotFreePtr, &cur_sh->FreeHead, sizeof cur_sh->FreeHead);
                cur_sh->FreeHead= BotFreePtr;
                ++cur_sh->FreeCount;
                TMDBG(printf(" FreeCount:%lu",cur_sh->FreeCount));
              }
            }
            BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t_printf_type)cur_alignment);
            curBottomIndex+= cur_alignment;
          } while (curBottomIndex & needed_alignment_mask);
        }
        ptr= BotFreePtr;
        BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t_printf_type)size);
      }
      else {
        /* fully aligned */
        ptr= (TopFreePtr= stepPointer(TopFreePtr, -(ptrdiff_t_printf_type)size));
      }
      sh->MallocCount++;
#if defined(SEGMENTED)
      TMDBG(printf(" current seg ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",CurrentSeg,pointerDifference(ptr, Arena[CurrentSeg]),sh->MallocCount));
#else
      TMDBG(printf(" current seg ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",pointerDifference(ptr,Arena),sh->MallocCount));
#endif
    }
    else
    {
NEXT_SEGMENT:
#if defined(SEGMENTED)
      if (CurrentSeg < (ArenaSegCnt-1)) {
        size_t curBottomIndex= (BotFreePtr - Arena[CurrentSeg]);
        while (curBottomIndex & PTRMASK) {
          size_t const cur_alignment= (curBottomIndex & -curBottomIndex);
          if (cur_alignment >= fxfMINSIZE) {
            SizeHead *cur_sh= &SizeData[(cur_alignment - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE];
            if ((cur_alignment >= sizeof cur_sh->FreeHead) || !cur_sh->FreeCount) {
              if (cur_alignment >= sizeof cur_sh->FreeHead)
                memcpy(BotFreePtr, &cur_sh->FreeHead, sizeof cur_sh->FreeHead);
              cur_sh->FreeHead= BotFreePtr;
              ++cur_sh->FreeCount;
              TMDBG(printf(" FreeCount:%lu",cur_sh->FreeCount));
            }
          }
          BotFreePtr= stepPointer(BotFreePtr, cur_alignment);
          curBottomIndex+= cur_alignment;
        }
        if (BotFreePtr < TopFreePtr) {
          size_t cur_size= (size_t)(TopFreePtr-BotFreePtr);
          if (cur_size >= fxfMINSIZE) {
            SizeHead *cur_sh= &SizeData[(cur_size - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE];
            if ((cur_size >= sizeof cur_sh->FreeHead) || !cur_sh->FreeCount) {
              if (cur_size >= sizeof cur_sh->FreeHead)
                memcpy(BotFreePtr, &cur_sh->FreeHead, sizeof cur_sh->FreeHead);
              cur_sh->FreeHead= BotFreePtr;
              ++cur_sh->FreeCount;
              TMDBG(printf(" FreeCount:%lu",cur_sh->FreeCount));
            }
          }
        }
        TMDBG(fputs(" next seg", stdout));
        ++CurrentSeg;
        BotFreePtr= Arena[CurrentSeg];
        TopFreePtr= stepPointer(Arena[CurrentSeg], ARENA_SEG_SIZE);
        goto START_LOOKING_FOR_CHUNK;
      }
      else
        ptr= Nil(void);
#else /*SEGMENTED*/
      ptr= Nil(void);
#endif /*!SEGMENTED*/
      TMDBG(printf(" ptr:%p\n", ptr));
    }
  }
  DBG((df, "%p\n", ptr));
  return ptr;
}

void fxfFree(void *ptr, size_t size)
{
  static char const * const myname= "fxfFree";
  SizeHead *sh;

  ptrdiff_t_printf_type ptrIndex;
#if defined(SEGMENTED)
  int ptrSegment;
#endif
  assert(ptr || !size);
  if (!ptr)
    return;
#if defined(SEGMENTED)
  for (ptrSegment= CurrentSeg; ptrSegment >= 0; --ptrSegment) {
    pointer_to_int_type tmp= (pointer_to_int_type)ptr;
    pointer_to_int_type segment_begin= (pointer_to_int_type)Arena[ptrSegment];
    if (tmp >= segment_begin) {
      ptrIndex= (tmp - segment_begin);
      if (ptrIndex < ARENA_SEG_SIZE)
        goto FOUND_PUTATIVE_SEGMENT;
    }
  }
  ptrIndex= -1;
FOUND_PUTATIVE_SEGMENT:
  TMDBG(printf("fxfFree - ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,ptrSegment,ptrIndex,(size_t_printf_type)size));
#else
  ptrIndex= pointerDifference(ptr,Arena);
  TMDBG(printf("fxfFree - ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,ptrIndex,(size_t_printf_type)size));
#endif
  DBG((df, "%s(%p, %" SIZE_T_PRINTF_SPECIFIER ")\n", myname, (void *)ptr, (size_t_printf_type) size));
  if (size < fxfMINSIZE)
    size= fxfMINSIZE;
  if (size > fxfMAXSIZE) {
    fprintf(stderr, "%s: size=%" SIZE_T_PRINTF_SPECIFIER " >= %" SIZE_T_PRINTF_SPECIFIER "\n",
            myname, (size_t_printf_type) size, (size_t_printf_type) fxfMAXSIZE);
    exit(-5);
  }
  size= ALIGN_TO_MINIMUM(size);
#if !defined(NDEBUG)
  if (ptrIndex > 0) {
    size_t needed_alignment= MAX_ALIGNMENT;
    while (needed_alignment > size)
      needed_alignment>>= 1;
    assert(!(((size_t)ptrIndex) & (needed_alignment - 1U)));
  }
#endif
  sh= &SizeData[(size - fxfMINSIZE)/MIN_ALIGNMENT_UNDERESTIMATE];
  if (size&PTRMASK) {
    /* not fully aligned size */
    TMDBG(printf(" BotFreePtr-ptr:%" PTRDIFF_T_PRINTF_SPECIFIER,pointerDifference(BotFreePtr,ptr)));
    if (stepPointer(ptr, (ptrdiff_t_printf_type)size) == BotFreePtr) {
      BotFreePtr= ptr;
      TMDBG(printf(" BotFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,pointerDifference(TopFreePtr,BotFreePtr)));
      --sh->MallocCount;
    }
    else {
#if defined(FREEMAP) && !defined(SEGMENTED)
      SetRange((pointerDifference(ptr,Arena),size);
#endif
      if ((size >= sizeof sh->FreeHead) || !sh->FreeHead) {
        if (size >= sizeof sh->FreeHead)
          memcpy(ptr, &sh->FreeHead, sizeof sh->FreeHead);
        sh->FreeHead= ptr;
        ++sh->FreeCount;
        --sh->MallocCount;
        TMDBG(printf(" FreeCount:%lu",sh->FreeCount));
      }
    }
  }
  else {
    /* fully aligned size */
    TMDBG(printf(" ptr-TopFreePtr:%" PTRDIFF_T_PRINTF_SPECIFIER,pointerDifference(ptr,TopFreePtr)));
    if (ptr == TopFreePtr) {
      TopFreePtr= stepPointer(TopFreePtr, (ptrdiff_t_printf_type)size);
      TMDBG(printf(" TopFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,pointerDifference(TopFreePtr,BotFreePtr)));
      --sh->MallocCount;
    }
    else {
#if defined(FREEMAP) && !defined(SEGMENTED)
      SetRange(pointerDifference(ptr,Arena),size);
#endif
      if ((size >= sizeof sh->FreeHead) || !sh->FreeCount) {
        if (size >= sizeof sh->FreeHead)
          memcpy(ptr, &sh->FreeHead, sizeof sh->FreeHead);
        sh->FreeHead= ptr;
        ++sh->FreeCount;
        --sh->MallocCount;
        TMDBG(printf(" FreeCount:%lu",sh->FreeCount));
      }
    }
  }
  TMDBG(printf(" MallocCount:%lu",sh->MallocCount));
  TMDBG(putchar('\n'));
}

void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize) {
  void *nptr;
  if (!ptr)
    return fxfAlloc(NewSize);
  if (!NewSize)
    fxfFree(ptr, OldSize);
  nptr= fxfAlloc(NewSize);
  if (NewSize && nptr)
  {
    memcpy(nptr, ptr, ((NewSize < OldSize) ? NewSize : OldSize));
    fxfFree(ptr, OldSize);
  }
  return nptr;
}

#define SIZEDATA_INDEX_TO_SIZE(x) ((size_t)(((x) * MIN_ALIGNMENT_UNDERESTIMATE) + \
                                   ((fxfMINSIZE + (MIN_ALIGNMENT_UNDERESTIMATE - 1U)) & ~(MIN_ALIGNMENT_UNDERESTIMATE - 1U))))

size_t fxfTotal(void) {
  SizeHead const *hd = SizeData;
  size_t UsedBytes = 0;
  size_t FreeBytes = 0;

  size_t i;
  for (i=0; i<((sizeof SizeData)/(sizeof *SizeData)); i++,hd++) {
    if (hd->MallocCount+hd->FreeCount>0) {
      UsedBytes+= hd->MallocCount*SIZEDATA_INDEX_TO_SIZE(i);
      FreeBytes+= hd->FreeCount*SIZEDATA_INDEX_TO_SIZE(i);
    }
  }

  return UsedBytes+FreeBytes;
}

void fxfInfo(FILE *f) {
  size_t const one_kilo = 1<<10;
  size_t const sizeCurrentSeg = (size_t)pointerDifference(TopFreePtr,BotFreePtr);
  size_t const sizeArenaUsed =
          GlobalSize-sizeCurrentSeg
#if defined(SEGMENTED)
          - (ArenaSegCnt-CurrentSeg-1)*ARENA_SEG_SIZE
#endif /*SEGMENTED*/
      ;
  assert(GlobalSize/one_kilo<=ULONG_MAX);
  fprintf(f, "fxfArenaSize = %" SIZE_T_PRINTF_SPECIFIER " kB\n",
          (size_t_printf_type) (GlobalSize/one_kilo));
  assert(sizeArenaUsed/one_kilo<=ULONG_MAX);
  fprintf(f, "fxfArenaUsed = %" SIZE_T_PRINTF_SPECIFIER " kB\n",
          (size_t_printf_type) (sizeArenaUsed/one_kilo));
  fprintf(f, "fxfMAXSIZE   = %" SIZE_T_PRINTF_SPECIFIER " B\n", (size_t_printf_type) fxfMAXSIZE);

  {
    SizeHead const *hd = SizeData;
    unsigned long nrUsed = 0;
    unsigned long nrFree = 0;
    size_t UsedBytes = 0;
    size_t FreeBytes = 0;

    size_t i;
    fprintf(f, "%12s  %10s%10s\n", "Size", "MallocCnt", "FreeCnt");
    for (i=0; i<((sizeof SizeData)/(sizeof *SizeData)); i++,hd++) {
      if (hd->MallocCount+hd->FreeCount>0) {
        fprintf(f, "%12zu  %10lu%10lu\n", SIZEDATA_INDEX_TO_SIZE(i), hd->MallocCount, hd->FreeCount);
        nrUsed+= hd->MallocCount;
        UsedBytes+= hd->MallocCount*(i+1U);
        nrFree+= hd->FreeCount;
        FreeBytes+= hd->FreeCount*(i+1U);
      }
    }
    fprintf(f, "%12s  %10lu%10lu\n", "Total:", nrUsed, nrFree);
    assert(UsedBytes/one_kilo<=ULONG_MAX);
    assert(FreeBytes/one_kilo<=ULONG_MAX);
    fprintf(f, "%12s  %10" SIZE_T_PRINTF_SPECIFIER "%10" SIZE_T_PRINTF_SPECIFIER "\n", "Total kB:",
            (size_t_printf_type) (UsedBytes/one_kilo),
            (size_t_printf_type) (FreeBytes/one_kilo));
  }
}

#endif /*FXF*/
