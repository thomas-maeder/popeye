#if defined(FXF)

#include "debugging/assert.h"
#include <limits.h>
#include <stdlib.h>
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
#  include <inttypes.h>
   typedef ptrdiff_t ptrdiff_t_printf_type;
   typedef size_t size_t_printf_type;
   typedef uintmax_t largest_integer_type;
#  define MAX_POINTER_DIFFERENCE PTRDIFF_MAX
#  if defined(UINTPTR_MAX)
   typedef uintptr_t convert_pointer_to_int_type;
#  else
   typedef uintmax_t convert_pointer_to_int_type;
#  endif
#  define PTRDIFF_T_PRINTF_SPECIFIER "td"
#  define SIZE_T_PRINTF_SPECIFIER "zu"
#else
#  if defined(LLONG_MAX) /* We have long long integer types. */
   typedef long long int ptrdiff_t_printf_type;
   typedef unsigned long long int size_t_printf_type;
   typedef unsigned long long int convert_pointer_to_int_type;
   typedef unsigned long long int largest_integer_type;
#    define PTRDIFF_T_PRINTF_SPECIFIER "lld"
#    define SIZE_T_PRINTF_SPECIFIER "llu"
#  else /* We don't have long long integer types. */
   typedef long int ptrdiff_t_printf_type;
   typedef unsigned long int size_t_printf_type;
   typedef unsigned long int convert_pointer_to_int_type;
   typedef unsigned long int largest_integer_type;
#    define PTRDIFF_T_PRINTF_SPECIFIER "ld"
#    define SIZE_T_PRINTF_SPECIFIER "lu"
#  endif
#  define MAX_POINTER_DIFFERENCE ((ptrdiff_t_printf_type)(((size_t)-1)>>1)) /* just a guess */
enum
{
  ENSURE_MAX_POINTER_DIFFERENCE_POSITIVE = 1/(MAX_POINTER_DIFFERENCE > 0) /* I'm not sure what would happen if this fails,
                                                                             but it likely wouldn't be pretty. */
};
#endif

#define BOTTOM_BIT(s) (((size_t) (s)) & -(size_t) (s))

/*
   We want to ensure that every block we hand out is properly aligned for the
   object it will be storing.  Trying to honor all required alignments without
   wasting too much memory would complicate things dramatically, so we make some
   assumptions.  We assume there is a MAX_ALIGNMENT and a NOT_MULTIPLE_ALIGNMENT.
   Any allocation with size that's a multiple of MAX_ALIGNMENT will be assumed
   to require no more than that alignment.  Other sizes will be assumed to require
   no more than NOT_MULTIPLE_ALIGNMENT, though we'll consider that they may require
   less (if they're smaller than NOT_MULTIPLE_ALIGNMENT).
*/

/*
   Determine MAX_ALIGNMENT and NOT_MULTIPLE_ALIGNMENT.  If types
   were provided then use their alignments; otherwise try to infer the maximum
   values on the system.
*/
#if defined(FXF_MAX_ALIGNMENT_TYPE)
#  define MAX_ALIGNMENT ALIGNMENT_OF_TYPE(FXF_MAX_ALIGNMENT_TYPE)
#else /*FXF_MAX_ALIGNMENT_TYPE*/
#  if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#    define MAX_ALIGNMENT _Alignof(max_align_t)
#  elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#    define MAX_ALIGNMENT alignof(max_align_t)
#  else
#    if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#      include <stdint.h>
#    endif
#    include <signal.h>
#    define MAX_ALIGNMENT ALIGNMENT_OF_TYPE(union {largest_integer_type unsigned_integer; \
                                                   const volatile void * object_pointer; \
                                                   void (*function_pointer)(void); \
                                                   long double floating_point; \
                                                   sig_atomic_t atomic_integer;})
#  endif
#endif /*!FXF_MAX_ALIGNMENT_TYPE*/

#if defined(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE)
#  define NOT_MULTIPLE_ALIGNMENT ((ALIGNMENT_OF_TYPE(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE) > MAX_ALIGNMENT) ? \
                                   MAX_ALIGNMENT : \
                                   ALIGNMENT_OF_TYPE(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE))
#else /*FXF_NOT_MULTIPLE_ALIGNMENT_TYPE*/
#  define NOT_MULTIPLE_ALIGNMENT MAX_ALIGNMENT
#endif /*!FXF_NOT_MULTIPLE_ALIGNMENT_TYPE*/

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

#define CLIP_TO_MAX_POINTER_DIFFERENCE(x) (((x) > MAX_POINTER_DIFFERENCE) ? MAX_POINTER_DIFFERENCE : (x))
#define ROUND_DOWN_TO_ALIGNMENT(s, a) (((size_t) (s)) & ~(((size_t) (a)) - 1U))
#define ROUND_UP_TO_ALIGNMENT(s, a) ROUND_DOWN_TO_ALIGNMENT((((size_t) (a)) - 1U) + (size_t) (s), a)
#define CLEAR_BOTTOM_BIT(s) ((((size_t) (s)) - 1U) & (size_t) (s))

#if defined(DOS)
/* MSDOS 16 Bit support (maxmemory <= 1 MB) */
#define SEGMENTED
#define ARENA_SEG_SIZE  ROUND_DOWN_TO_ALIGNMENT(CLIP_TO_MAX_POINTER_DIFFERENCE(32000), MAX_ALIGNMENT)
#define ARENA_SEG_COUNT ((1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "MSDOS"
#define OSMAXMEM "1 MB"
#elif defined(_WIN98)
/* Win95/Win98/WinME can only allocate chunks up to 255 MB */
/* maxmemory <= 768 MB */
#define SEGMENTED
#define ARENA_SEG_SIZE  ROUND_DOWN_TO_ALIGNMENT(CLIP_TO_MAX_POINTER_DIFFERENCE(1000000), MAX_ALIGNMENT)
#define ARENA_SEG_COUNT ((768*1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "Win95/Win98/WinME"
#define OSMAXMEM "768 MB"
#endif

/* The maximum size an fxfAlloc can handle */
#if !defined(FXF_DESIRED_MAX_ALLOC)
#  define FXF_DESIRED_MAX_ALLOC (256U * sizeof(void *))
/* TODO: Is the above a good default, sufficiently large for all of our needs without being excessive? */
#endif

/*
   Determine the maximum amount we'll allow one to allocate.  We need to ensure that it's
   a multiple of its required alignment, and we're OK with it being overaligned, so we
   round it up to the next alignment multiple.
*/
#define DESIRED_MAX_ALLOC_ALIGNMENT ((FXF_DESIRED_MAX_ALLOC < MAX_ALIGNMENT) ? NOT_MULTIPLE_ALIGNMENT : MAX_ALIGNMENT)
#define ROUNDED_DESIRED_MAXIMUM_ALLOC ROUND_UP_TO_ALIGNMENT(FXF_DESIRED_MAX_ALLOC, DESIRED_MAX_ALLOC_ALIGNMENT)

/*
    Now determine the actual minimum and maximum allocations.  The minimum is new.
    We determined the maximum above, but of course we can't allocate more than
    a full segment.  (Segment sizes should already be maximally aligned.)
*/
enum
{
  fxfMINSIZE = sizeof(void *), /* Different size of fxfMINSIZE for 32-/64/Bit compilation */
#if defined(SEGMENTED)
  fxfMAXSIZE = ((ROUNDED_DESIRED_MAXIMUM_ALLOC > ARENA_SEG_SIZE) ? ARENA_SEG_SIZE : ROUNDED_DESIRED_MAXIMUM_ALLOC)
#else
  fxfMAXSIZE = ROUNDED_DESIRED_MAXIMUM_ALLOC
#endif
};

STATIC_ASSERT(fxfMINSIZE > 0, "fxfMINSIZE must be > 0.");
STATIC_ASSERT(fxfMAXSIZE >= fxfMINSIZE, "fxfMAXSIZE must be >= fxfMINSIZE.");
#if defined(SEGMENT)
STATIC_ASSERT(!((ARENA_SEG_SIZE & (((ARENA_SEG_SIZE < MAX_ALIGNMENT) ? NOT_MULTIPLE_ALIGNMENT :
                                                                       MAX_ALIGNMENT)
                                   - 1U)) &&
                (ARENA_SEG_SIZE & (ARENA_SEG_SIZE - 1U))), "Segments must be aligned.");
#endif
STATIC_ASSERT((!CLEAR_BOTTOM_BIT(fxfMAXSIZE)) ||
                                 ((fxfMAXSIZE < MAX_ALIGNMENT) && !(fxfMAXSIZE & (NOT_MULTIPLE_ALIGNMENT - 1U))) ||
                                 !(fxfMAXSIZE & (MAX_ALIGNMENT - 1U)), "fxfMAXSIZE must be aligned.");
STATIC_ASSERT((NOT_MULTIPLE_ALIGNMENT > 0) && (NOT_MULTIPLE_ALIGNMENT <= MAX_ALIGNMENT), "Alignments must be properly ordered.");
STATIC_ASSERT(!(CLEAR_BOTTOM_BIT(NOT_MULTIPLE_ALIGNMENT) || CLEAR_BOTTOM_BIT(MAX_ALIGNMENT)), "Alignments must be powers of 2.");

/*
   We'll be rounding allocation sizes to the next multiple of whatever the minimum
   alignment we will ever need is.  We need to know that minimum alignment to
   create space for the free store.  Unfortunately, it's hard to compute what we
   want under all circumstances at compile-time.  (In particular, if fxfMINSIZE is
   small enough to require a smaller alignment than NOT_MULTIPLE_ALIGNMENT then we
   want the largest power of two that's <= fxfMINSIZE, but I don't see how to determine
   that at compile-time for an arbitrary fxfMINSIZE.)  What we compute here should be
   <= that value, and an underestimate here is OK.
*/
#define MIN_ALIGNMENT_UNDERESTIMATE (((NOT_MULTIPLE_ALIGNMENT>>1) < fxfMINSIZE) ? NOT_MULTIPLE_ALIGNMENT : \
                                                                                  (CLEAR_BOTTOM_BIT(fxfMINSIZE) ? (BOTTOM_BIT(fxfMINSIZE)<<2) : \
                                                                                                                  fxfMINSIZE))
/*
   This will store the actual minimum alignment.  We'll compute it at run-time.
*/
static size_t min_alignment= NOT_MULTIPLE_ALIGNMENT; /* for now */


/*
   This rounds fxfMINSIZE up to a multiple of MIN_ALIGNMENT_UNDERESTIMATE.  This should be <= whatever
   fxfMINSIZE gets rounded to at run-time, since MIN_ALIGNMENT_UNDERESTIMATE will be <= the computed
   min_alignment.
*/
#define ROUNDED_MIN_SIZE_UNDERESTIMATE ROUND_UP_TO_ALIGNMENT(fxfMINSIZE, MIN_ALIGNMENT_UNDERESTIMATE)

/*
   We can now create the array for the free store heads.  We need to have a head for sizes between
   [whatever fxfMINSIZE gets rounded to, fxfMAXSIZE]
   that are multiples of min_alignment.  Additional entries aren't a problem, so we'll create entries
   for every multiple of (the possible
   underestimate) MIN_ALIGNMENT_UNDERESTIMATE in the
   [ROUNDED_MIN_SIZE_UNDERESTIMATE, fxfMAXSIZE].
   SIZEDATA_SIZE_TO_INDEX maps these values into {0, 1, 2, ..., N}, and we use N to determine how many
   heads we need.
*/
#define SIZEDATA_SIZE_TO_INDEX(s) (((s) - ROUNDED_MIN_SIZE_UNDERESTIMATE)/MIN_ALIGNMENT_UNDERESTIMATE)
#define SIZEDATA_INDEX_TO_SIZE(x) ((size_t)(((x) * MIN_ALIGNMENT_UNDERESTIMATE) + ROUNDED_MIN_SIZE_UNDERESTIMATE))
static SizeHead SizeData[1 + SIZEDATA_SIZE_TO_INDEX(fxfMAXSIZE)];

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

static inline ptrdiff_t pointerDifference(void const *ptr1, void const *ptr2) {
  assert(ptr1 && ptr2);
  return (((char const *)ptr1) - ((char const *)ptr2));
}

static inline void * stepPointer(void *ptr, ptrdiff_t step) {
  assert(!!ptr);
  return (void *)(((char *)ptr) + step);
}

size_t fxfMaxAllocation(void) {
  return fxfMAXSIZE;
}

#define ALIGN_TO_MINIMUM(s)  ROUND_UP_TO_ALIGNMENT(s, min_alignment)

size_t fxfInit(size_t Size) {
#if defined(LOG) && !defined(SEGMENTED)
  static char const * const myname= "fxfInit";
#endif
#if defined(SEGMENTED)
  size_t maxSegCnt= (Size / ARENA_SEG_SIZE);
  if (maxSegCnt > ARENA_SEG_COUNT)
    maxSegCnt= ARENA_SEG_COUNT;
  while (maxSegCnt < (size_t)ArenaSegCnt) {
    --ArenaSegCnt;
    free(Arena[ArenaSegCnt]);
    Arena[ArenaSegCnt]= Nil(void);
  }
  while (maxSegCnt > (size_t)ArenaSegCnt) {
    Arena[ArenaSegCnt]= nNewUntyped(ARENA_SEG_SIZE, char);
    if (!Arena[ArenaSegCnt])
      break;
    ++ArenaSegCnt;
  }
  CurrentSeg= 0;
  BotFreePtr= Arena[CurrentSeg];
  TopFreePtr= Arena[CurrentSeg];
  if (TopFreePtr)
    TopFreePtr= stepPointer(TopFreePtr, ARENA_SEG_SIZE);
  GlobalSize= ARENA_SEG_SIZE*(size_t)ArenaSegCnt;
#else
#if defined(FREEMAP)
  if (FreeMap)
  {
    free(FreeMap);
    FreeMap= Nil(FreeMapType);
  }
#endif
#if defined(LOG)
  size_t const orig_Size= Size;
#endif
  if (Arena)
    free(Arena);
  if (Size > MAX_POINTER_DIFFERENCE)
    Size= MAX_POINTER_DIFFERENCE;
  if (Size < NOT_MULTIPLE_ALIGNMENT)
    while (CLEAR_BOTTOM_BIT(Size))
      Size= CLEAR_BOTTOM_BIT(Size);
  else if (Size < MAX_ALIGNMENT)
    Size= ROUND_DOWN_TO_ALIGNMENT(Size, NOT_MULTIPLE_ALIGNMENT);
  else
    Size= ROUND_DOWN_TO_ALIGNMENT(Size, MAX_ALIGNMENT);
  Arena= nNewUntyped(Size, char);
  if (!Arena) {
    ERROR_LOG3("%s: Sorry, cannot allocate arena of %" SIZE_T_PRINTF_SPECIFIER " <= %" SIZE_T_PRINTF_SPECIFIER " bytes\n",
               myname, (size_t_printf_type)Size, (size_t_printf_type)orig_Size);
    BotFreePtr= Arena;
    TopFreePtr= Arena;
    GlobalSize= 0;
    return GlobalSize;
  }
  BotFreePtr= Arena;
  TopFreePtr= stepPointer(Arena, (ptrdiff_t)Size);
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

  for (Size= 0; Size < ((sizeof SizeData)/(sizeof *SizeData)); ++Size)
  {
    SizeData[Size].MallocCount= 0;
    SizeData[Size].FreeCount= 0;
    SizeData[Size].FreeHead= Nil(void);
  }

  if ((NOT_MULTIPLE_ALIGNMENT>>1) >= fxfMINSIZE) /* compile-time check */
    while (min_alignment >= (((size_t)fxfMINSIZE)<<1))
      min_alignment>>= 1;

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
  {
    size_t i;
    for (i= 0; i < ((sizeof SizeData)/(sizeof *SizeData)); ++i)
    {
      SizeData[i].MallocCount= 0;
      SizeData[i].FreeCount= 0;
      SizeData[i].FreeHead= Nil(void);
    }
  }
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
    TopFreePtr= stepPointer(TopFreePtr, (ptrdiff_t)GlobalSize);

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

  {
    size_t i;
    for (i= 0; i < ((sizeof SizeData)/(sizeof *SizeData)); ++i)
    {
      SizeData[i].MallocCount= 0;
      SizeData[i].FreeCount= 0;
      SizeData[i].FreeHead= Nil(void);
    }
  }
}

/* we have to define the following, since some architectures cannot
 * access words on unaligned addresses. Most probably the
 * case if you are compiling on any modern risc architecture:
 * SPARC, HPPA, MIPS. We wouldn't need this when running on an
 * Intel *86 type of CPU, but also there, aligned access is faster.
 */
#define PTRMASK            (MAX_ALIGNMENT-1U)

#if defined(FXF_ENABLE_TMDBG)
#  define TMDBG(x) do {x;} while (0)
#else /*FXF_ENABLE_TMDBG*/
#  define TMDBG(x) do {;} while (0)
#endif /*!FXF_ENABLE_TMDBG*/

static int pushOntoFreeStore(void * const ptr, size_t const size) {
  SizeHead *cur_sh;
  assert((size >= ROUNDED_MIN_SIZE_UNDERESTIMATE) &&
         (size <= fxfMAXSIZE) &&
         !(size & (min_alignment - 1U)));
  cur_sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  assert((!cur_sh->FreeHead) == (!cur_sh->FreeCount));
  if ((ROUNDED_MIN_SIZE_UNDERESTIMATE < sizeof cur_sh->FreeHead) /* compile-time check */ &&
      (size < sizeof cur_sh->FreeHead) &&
      cur_sh->FreeHead)
    TMDBG(printf(" leaking %" SIZE_T_PRINTF_SPECIFIER " byte(s) instead of freeing them\n", (size_t_printf_type)size));
  else
  {
    if ((ROUNDED_MIN_SIZE_UNDERESTIMATE >= sizeof cur_sh->FreeHead) /* compile-time check */ ||
        (size >= sizeof cur_sh->FreeHead))
      memcpy(ptr, &cur_sh->FreeHead, sizeof cur_sh->FreeHead);
    cur_sh->FreeHead= ptr;
    cur_sh->FreeCount++;
    TMDBG(printf(" FreeCount:%lu",cur_sh->FreeCount));
#if defined(FREEMAP) && !defined(SEGMENTED)
    SetRange(pointerDifference(ptr, Arena), size);
#endif
    return 1;
  }
  return 0;
}

static void * popOffFreeStore(size_t const size)
{
  SizeHead *cur_sh;
  void *ptr;
  assert((size >= ROUNDED_MIN_SIZE_UNDERESTIMATE) &&
         (size <= fxfMAXSIZE) &&
         !(size & (min_alignment - 1U)));
  cur_sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  ptr= cur_sh->FreeHead;
  assert((!ptr) == (!cur_sh->FreeCount));
  if (ptr) {
    cur_sh->FreeCount--;
    if ((ROUNDED_MIN_SIZE_UNDERESTIMATE < sizeof cur_sh->FreeHead) /* compile-time check */ &&
        (size < sizeof cur_sh->FreeHead))
      cur_sh->FreeHead= Nil(void);
    else
      memcpy(&cur_sh->FreeHead, ptr, sizeof cur_sh->FreeHead);
#  if defined(FREEMAP) && !defined(SEGMENTED)
    ClrRange(pointerDifference(ptr, Arena), size);
#  endif
  }
  return ptr;
}

void *fxfAllocRaw(size_t size, size_t desired_alignment) {
#if defined(LOG) || defined(DEBUG)
  static char const * const myname= "fxfAllocRaw";
#endif
  void *ptr= Nil(void);

  TMDBG(printf("fxfAllocRaw - size:%" SIZE_T_PRINTF_SPECIFIER,(size_t_printf_type)size));
  DBG((stderr, "%s(%" SIZE_T_PRINTF_SPECIFIER ") =", myname, (size_t_printf_type)size));
  assert(desired_alignment && !CLEAR_BOTTOM_BIT(desired_alignment));

  if (!size)
    return Nil(void);

  assert(desired_alignment <= size);
  if (size<fxfMINSIZE)
    size= fxfMINSIZE;
  else if (size>fxfMAXSIZE)
  {
    ERROR_LOG3("%s: size=%" SIZE_T_PRINTF_SPECIFIER " > %" SIZE_T_PRINTF_SPECIFIER "\n",
               myname,
               (size_t_printf_type) size,
               (size_t_printf_type) fxfMAXSIZE);
    return Nil(void);
  }

  // Round up to a multiple of min_alignment
  size= ALIGN_TO_MINIMUM(size);

#if defined(NDEBUG)
  (void) desired_alignment; /* Ensure we "use" alignment. */
#else /*NDEBUG*/
  /* Check our alignment assumptions, purely for informational purposes. */
  if (size&PTRMASK) {
    if (desired_alignment > NOT_MULTIPLE_ALIGNMENT)
      fprintf(stderr, "WARNING: Not fully aligned allocation of size %" SIZE_T_PRINTF_SPECIFIER "needs %" SIZE_T_PRINTF_SPECIFIER "-byte allocation, > the %" SIZE_T_PRINTF_SPECIFIER " byte(s) that FXF will guarantee\n",
                      (size_t_printf_type) size,
                      (size_t_printf_type) desired_alignment,
                      (size_t_printf_type) NOT_MULTIPLE_ALIGNMENT);
  }
  else {
    if (desired_alignment > MAX_ALIGNMENT)
      fprintf(stderr, "WARNING: fully aligned allocation of size %" SIZE_T_PRINTF_SPECIFIER " needs %" SIZE_T_PRINTF_SPECIFIER "-byte allocation, > the %" SIZE_T_PRINTF_SPECIFIER " byte(s) that FXF will guarantee\n",
                      (size_t_printf_type) size,
                      (size_t_printf_type) desired_alignment,
                      (size_t_printf_type) MAX_ALIGNMENT);
  }
#endif /*!NDEBUG*/

  ptr= popOffFreeStore(size);

  if (ptr) {
    SizeHead *const sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
#if defined(SEGMENTED) && defined(FXF_ENABLE_TMDBG)
    ptrdiff_t ptrIndex;
    int ptrSegment= CurrentSeg;
#endif
    sh->MallocCount++;
#if defined(SEGMENTED)
#  if defined(FXF_ENABLE_TMDBG)
    if (CurrentSeg) {
      convert_pointer_to_int_type tmp= (convert_pointer_to_int_type)ptr;
      do {
        convert_pointer_to_int_type segment_begin= (convert_pointer_to_int_type)Arena[ptrSegment];
        if ((tmp >= segment_begin) && ((tmp - segment_begin) < ARENA_SEG_SIZE)) {
          ptrIndex= (tmp - segment_begin);
          goto FOUND_PUTATIVE_SEGMENT;
        }
      } while (0 <= --ptrSegment);
      ptrIndex= -1;
    } else
      ptrIndex= pointerDifference(ptr, Arena[0]);
FOUND_PUTATIVE_SEGMENT:
    TMDBG(printf(" FreeCount:%lu ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",sh->FreeCount,ptrSegment,(ptrdiff_t_printf_type)ptrIndex,sh->MallocCount));
#  endif /*FXF_ENABLE_TMDBG*/
#else /*SEGMENTED*/
    TMDBG(printf(" FreeCount:%lu ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",sh->FreeCount,(ptrdiff_t_printf_type)pointerDifference(ptr, Arena),sh->MallocCount));
#endif /*!SEGMENTED*/
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
      SizeHead *sh;
      if (size&PTRMASK) {
        /* not fully aligned */
        size_t curBottomIndex=
#if defined(SEGMENTED)
          (size_t)pointerDifference(BotFreePtr,Arena[CurrentSeg]);
#else
          (size_t)pointerDifference(BotFreePtr,Arena);
#endif
        /*
           Determine what alignment this allocation actually requires.
        */
        size_t needed_alignment_mask= (NOT_MULTIPLE_ALIGNMENT-1U);
        while (needed_alignment_mask >= size)
          needed_alignment_mask>>= 1;

        /*
           We only care about our offset modulo the required alignment.
        */
        curBottomIndex&= needed_alignment_mask;
        if (curBottomIndex) {
          /*
             We aren't aligned as strongly as we need to be for this allocation.  Let's step until
             we are, and we'll add what we skip over to the free store.  If we won't have a enough
             space left then move on to the next segment.
          */
          if ((needed_alignment_mask - curBottomIndex) >= (sizeCurrentSeg - size))
            goto NEXT_SEGMENT;
          do {
            size_t const cur_alignment= BOTTOM_BIT(curBottomIndex);
            assert(cur_alignment >= min_alignment);
            if (cur_alignment >= ALIGN_TO_MINIMUM(fxfMINSIZE))
              pushOntoFreeStore(BotFreePtr, cur_alignment);
            else
              TMDBG(printf(" leaking %" SIZE_T_PRINTF_SPECIFIER " byte(s) instead of adding them to the free store\n", (size_t_printf_type)cur_alignment));
            BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t)cur_alignment);
            curBottomIndex+= cur_alignment;
          } while (curBottomIndex & needed_alignment_mask);
        }
        ptr= BotFreePtr;
        BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t)size);
      }
      else {
        /* fully aligned */
        TopFreePtr= stepPointer(TopFreePtr, -(ptrdiff_t)size);
        ptr= TopFreePtr;
      }
      sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
      sh->MallocCount++;
#if defined(SEGMENTED)
      TMDBG(printf(" current seg ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",CurrentSeg,(ptrdiff_t_printf_type)pointerDifference(ptr, Arena[CurrentSeg]),sh->MallocCount));
#else
      TMDBG(printf(" current seg ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " MallocCount:%lu\n",(ptrdiff_t_printf_type)pointerDifference(ptr,Arena),sh->MallocCount));
#endif
    }
    else
    {
NEXT_SEGMENT:
#if defined(SEGMENTED)
      if ((CurrentSeg+1) < ArenaSegCnt) {
        /*
           We're about to move to the next segment, but let's put whatever we can into the free store.
        */
        size_t curBottomIndex= (size_t)pointerDifference(BotFreePtr,Arena[CurrentSeg]);
        /*
           Add small powers of two until we reach NOT_MULTIPLE_ALIGNMENT.
        */
        size_t cur_alignment;
        while (curBottomIndex & (NOT_MULTIPLE_ALIGNMENT-1U))
        {
          cur_alignment= BOTTOM_BIT(curBottomIndex);
          assert(cur_alignment >= min_alignment);
          if (cur_alignment >= ALIGN_TO_MINIMUM(fxfMINSIZE))
            pushOntoFreeStore(BotFreePtr, cur_alignment);
          else
            TMDBG(printf(" leaking %" SIZE_T_PRINTF_SPECIFIER " byte(s) instead of adding them to the free store\n", (size_t_printf_type)cur_alignment));
          BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t)cur_alignment);
          curBottomIndex+= cur_alignment;
        }
        /*
           If there's anything left, we can add it all.  Here we take advantage of the fact that
           the whole segment is fully aligned, so if what's left requires full alignment then
           the pointer must be fully aligned when we get here.
        */
        cur_alignment= (size_t)pointerDifference(TopFreePtr,BotFreePtr); /* Now stores the remaining space. */
        if (cur_alignment)
        {
          assert(!((cur_alignment | curBottomIndex) & (NOT_MULTIPLE_ALIGNMENT-1U))); /* Must be divisible by and aligned for NOT_MULTIPLE_ALIGNMENT. */
          assert((cur_alignment & PTRMASK) || !(curBottomIndex & PTRMASK)); /* If we're divisible by MAX_ALIGNMENT then we must be properly aligned for that. */
          if (cur_alignment >= ALIGN_TO_MINIMUM(fxfMINSIZE))
            pushOntoFreeStore(BotFreePtr, cur_alignment);
          else
            TMDBG(printf(" leaking %" SIZE_T_PRINTF_SPECIFIER " byte(s) moving from segment %d to segment %d\n", (size_t_printf_type)cur_alignment, CurrentSeg, CurrentSeg+1));
        }
        TMDBG(fputs(" next seg", stdout));
        ++CurrentSeg;
        BotFreePtr= Arena[CurrentSeg];
        TopFreePtr= stepPointer(Arena[CurrentSeg], ARENA_SEG_SIZE);
        goto START_LOOKING_FOR_CHUNK;
      }
      else
#endif /*SEGMENTED*/
      {
        ptr= Nil(void);
        /* TODO: Should we try to break apart a larger chunk? */
      }
      TMDBG(printf(" ptr:%p\n", ptr));
    }
  }
  DBG((df, "%p\n", ptr));
  return ptr;
}

void fxfFree(void *ptr, size_t size)
{
#if defined(DEBUG)
  static char const * const myname= "fxfFree";
#endif
  SizeHead *sh;

#if defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG)
  ptrdiff_t ptrIndex;
#endif
#if defined(SEGMENTED) && (defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG))
  int ptrSegment;
#endif
  if (!ptr)
    return;
  assert(!!size);
#if defined(SEGMENTED)
#  if defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG)
  ptrSegment= CurrentSeg;
  if (CurrentSeg) {
    convert_pointer_to_int_type tmp= (convert_pointer_to_int_type)ptr;
    do {
      convert_pointer_to_int_type segment_begin= (convert_pointer_to_int_type)Arena[ptrSegment];
      if (tmp >= segment_begin) {
        ptrIndex= (ptrdiff_t)(tmp - segment_begin);
        if (ARENA_SEG_SIZE > (size_t)ptrIndex)
          goto FOUND_PUTATIVE_SEGMENT;
      }
    } while (0 <= --ptrSegment);
    ptrIndex= -1;
  } else {
    ptrIndex= pointerDifference(ptr,Arena[0]);
    assert((ptrIndex >= 0) && (ARENA_SEG_SIZE > (size_t)ptrIndex));
  }
FOUND_PUTATIVE_SEGMENT:
  TMDBG(printf("fxfFree - ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,ptrSegment,(ptrdiff_t_printf_type)ptrIndex,(size_t_printf_type)size));
#  endif /*FXF_ENABLE_TMDBG*/
#else /*SEGMENTED*/
#  if defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG)
  ptrIndex= pointerDifference(ptr,Arena);
#  endif
  assert((ptrIndex >= 0) && (GlobalSize > (size_t)ptrIndex));
  TMDBG(printf("fxfFree - ptr-Arena:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,(ptrdiff_t_printf_type)ptrIndex,(size_t_printf_type)size));
#endif /*!SEGMENTED*/
  DBG((df, "%s(%p, %" SIZE_T_PRINTF_SPECIFIER ")\n", myname, (void *)ptr, (size_t_printf_type) size));
  if (size < fxfMINSIZE)
    size= fxfMINSIZE;
  else
  {
    assert(size <= fxfMAXSIZE);
  }
  size= ALIGN_TO_MINIMUM(size);
#if !defined(NDEBUG)
#  if defined(SEGMENTED)
  if (!CurrentSeg) /* Otherwise we'd be relying on converting to convert_pointer_to_int_type,
                      and such calculations aren't guaranteed to provide exactly what we need. */
  {
    assert(size <= (ARENA_SEG_SIZE - (size_t)ptrIndex));
    assert(((size + (size_t)ptrIndex) <= (size_t)pointerDifference(BotFreePtr,Arena[0])) || (ptr >= TopFreePtr));
#  else
  {
    assert(size <= (GlobalSize - (size_t)ptrIndex));
    assert(((size + (size_t)ptrIndex) <= (size_t)pointerDifference(BotFreePtr,Arena)) || (ptr >= TopFreePtr));
#endif
    if (ptrIndex > 0)
    {
      size_t needed_alignment;
      if (size&PTRMASK)
      {
        needed_alignment= NOT_MULTIPLE_ALIGNMENT;
        while (needed_alignment > size)
          needed_alignment>>= 1;
      }
      else
        needed_alignment= MAX_ALIGNMENT;
      assert(!(((size_t)ptrIndex) & (needed_alignment - 1U)));
    }
  }
#endif
  if (size&PTRMASK) {
    /* not fully aligned size */
    TMDBG(printf(" BotFreePtr-ptr:%" PTRDIFF_T_PRINTF_SPECIFIER,(ptrdiff_t_printf_type)pointerDifference(BotFreePtr,ptr)));
    if (stepPointer(ptr, (ptrdiff_t)size) == BotFreePtr) {
      BotFreePtr= ptr;
      TMDBG(printf(" BotFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,(ptrdiff_t_printf_type)pointerDifference(TopFreePtr,BotFreePtr)));
    }
    else {
      pushOntoFreeStore(ptr, size);
    }
  }
  else {
    /* fully aligned size */
    TMDBG(printf(" ptr-TopFreePtr:%" PTRDIFF_T_PRINTF_SPECIFIER,(ptrdiff_t_printf_type)pointerDifference(ptr,TopFreePtr)));
    if (ptr == TopFreePtr) {
      TopFreePtr= stepPointer(TopFreePtr, (ptrdiff_t)size);
      TMDBG(printf(" TopFreePtr sizeCurrentSeg:%" PTRDIFF_T_PRINTF_SPECIFIER,(ptrdiff_t_printf_type)pointerDifference(TopFreePtr,BotFreePtr)));
    }
    else
      pushOntoFreeStore(ptr, size);
  }
  sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  sh->MallocCount--;
  TMDBG(printf(" MallocCount:%lu",sh->MallocCount));
  TMDBG(putchar('\n'));
}

void *fxfReAllocRaw(void *ptr, size_t OldSize, size_t NewSize, size_t desired_alignment) {
  void *nptr;
  size_t original_allocation;
  if (!ptr)
  {
    assert(!OldSize);
    return fxfAllocRaw(NewSize, desired_alignment);
  }
  assert(OldSize && (OldSize <= fxfMAXSIZE));
#if !defined(NDEBUG)
#  if defined(SEGMENTED)
  if (!CurrentSeg) /* Otherwise we'd be relying on converting to convert_pointer_to_int_type,
                      and such calculations aren't guaranteed to provide exactly what we need. */
  {
    ptrdiff_t const ptrIndex= pointerDifference(ptr,Arena[0]);
    assert(ARENA_SEG_SIZE > (size_t)ptrIndex);
#  else
  {
    ptrdiff_t const ptrIndex= pointerDifference(ptr,Arena);
    assert(GlobalSize > (size_t)ptrIndex);
#  endif
    assert(ptrIndex >= 0);
    if (ptrIndex > 0)
    {
      size_t allocatedSize= OldSize;
      size_t needed_alignment;
      if (allocatedSize < fxfMINSIZE)
        allocatedSize= fxfMINSIZE;
      else
      {
        assert(allocatedSize <= fxfMAXSIZE);
      }
      allocatedSize= ALIGN_TO_MINIMUM(allocatedSize);
#  if defined(SEGMENTED)
      assert(allocatedSize <= (ARENA_SEG_SIZE - (size_t)ptrIndex));
#  else
      assert(allocatedSize <= (GlobalSize - (size_t)ptrIndex));
#  endif
      if (allocatedSize&PTRMASK)
      {
        needed_alignment= NOT_MULTIPLE_ALIGNMENT;
        while (needed_alignment > allocatedSize)
          needed_alignment>>= 1;
      }
      else
        needed_alignment= MAX_ALIGNMENT;
      assert(!(((size_t)ptrIndex) & (needed_alignment - 1U)));
    }
  }
#endif
  assert(desired_alignment && !CLEAR_BOTTOM_BIT(desired_alignment));
  assert(desired_alignment <= OldSize);
  if (!NewSize)
  {
    fxfFree(ptr, OldSize);
    return Nil(void);
  }
  assert(desired_alignment <= NewSize);
  original_allocation= OldSize;
  if (original_allocation < fxfMINSIZE)
    original_allocation= fxfMINSIZE;
  original_allocation= ALIGN_TO_MINIMUM(original_allocation);
  if (NewSize <= original_allocation)
  {
    size_t needed_allocation= NewSize;
    if (needed_allocation < fxfMINSIZE)
      needed_allocation= fxfMINSIZE;
    needed_allocation= ALIGN_TO_MINIMUM(needed_allocation);
    if (needed_allocation == original_allocation)
      return ptr;
    /* TODO: Should we try to break apart this chunk? */
  }
  nptr= fxfAllocRaw(NewSize, desired_alignment);
  if (nptr)
  {
    memcpy(nptr, ptr, ((NewSize < OldSize) ? NewSize : OldSize));
    fxfFree(ptr, OldSize);
  }
  return nptr;
}

size_t fxfTotal(void) {
  SizeHead const *hd = SizeData;
  size_t UsedBytes = 0;
  size_t FreeBytes = 0;

  size_t i;
  for (i=0; i<((sizeof SizeData)/(sizeof *SizeData)); i++,hd++) {
    size_t const cur_size= SIZEDATA_INDEX_TO_SIZE(i);
    UsedBytes+= hd->MallocCount*cur_size;
    FreeBytes+= hd->FreeCount*cur_size;
  }

  return UsedBytes+FreeBytes;
}

void fxfInfo(FILE *f) {
  size_t const one_kilo = 1<<10;
  size_t const sizeCurrentSeg = (size_t)pointerDifference(TopFreePtr,BotFreePtr);
  size_t const sizeArenaUsed =
          GlobalSize-sizeCurrentSeg
#if defined(SEGMENTED)
          - ARENA_SEG_SIZE*(size_t)(ArenaSegCnt-CurrentSeg-1)
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
      if (hd->MallocCount || hd->FreeCount) {
        size_t const cur_size= SIZEDATA_INDEX_TO_SIZE(i);
        fprintf(f, "%12" SIZE_T_PRINTF_SPECIFIER "  %10lu%10lu\n", (size_t_printf_type)cur_size, hd->MallocCount, hd->FreeCount);
        nrUsed+= hd->MallocCount;
        UsedBytes+= hd->MallocCount*cur_size;
        nrFree+= hd->FreeCount;
        FreeBytes+= hd->FreeCount*cur_size;
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

#else /*FXF*/

extern unsigned char FXF_C_NONEMPTY_TRANSLATION_UNIT;

#endif /*FXF*/
