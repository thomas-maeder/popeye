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
#  define MAX_POINTER_DIFFERENCE PTRDIFF_MAX
#  if defined(UINTPTR_MAX)
   typedef uintptr_t convert_pointer_to_int_type;
#  else
   typedef uintmax_t convert_pointer_to_int_type;
#  endif
#  define PTRDIFF_T_PRINTF_SPECIFIER "td"
#  define SIZE_T_PRINTF_SPECIFIER "zu"
#else
#  define MAX_POINTER_DIFFERENCE (((size_t)-1)>>1) /* just a guess */
#  if defined(LLONG_MAX) /* We have long long integer types. */
   typedef long long int ptrdiff_t_printf_type;
   typedef unsigned long long int size_t_printf_type;
   typedef unsigned long long int convert_pointer_to_int_type;
#    define PTRDIFF_T_PRINTF_SPECIFIER "lld"
#    define SIZE_T_PRINTF_SPECIFIER "llu"
#  else /* We don't have long long integer types. */
   typedef long int ptrdiff_t_printf_type;
   typedef unsigned long int size_t_printf_type;
   typedef unsigned long int convert_pointer_to_int_type;
#    define PTRDIFF_T_PRINTF_SPECIFIER "ld"
#    define SIZE_T_PRINTF_SPECIFIER "lu"
#  endif
#endif

#if defined(FXF_MAX_ALIGNMENT_TYPE)
#  if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#    define MAX_ALIGNMENT _Alignof(FXF_MAX_ALIGNMENT_TYPE)
#  elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#    define MAX_ALIGNMENT alignof(FXF_MAX_ALIGNMENT_TYPE)
#  else
  struct GET_MAX_ALIGNMENT_TYPE {
    unsigned char c;
    FXF_MAX_ALIGNMENT_TYPE max_alignment_member;
  };
#    define MAX_ALIGNMENT offsetof(struct GET_MAX_ALIGNMENT_TYPE, max_alignment_member)
#  endif
#else /*FXF_MAX_ALIGNMENT_TYPE*/
#  if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#    define MAX_ALIGNMENT _Alignof(max_align_t)
#  elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#    define MAX_ALIGNMENT alignof(max_align_t)
#  else
#    if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#      include <stdint.h>
#    endif
  struct GET_MAX_ALIGNMENT_TYPE {
    unsigned char c;
    union {
#    if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
      uintmax_t unsigned_integer;
#    elif defined(LLONG_MAX) /* We have long long integer types. */
      unsigned long long int unsigned_integer;
#    else
      unsigned long int unsigned_integer;
#    endif
      const volatile void * object_pointer;
      void (*function_pointer)(void);  
      long double floating_point;
    } max_aligned_union;
  };
#    define MAX_ALIGNMENT offsetof(struct GET_MAX_ALIGNMENT_TYPE, max_aligned_union)
#  endif
#endif /*!FXF_MAX_ALIGNMENT_TYPE*/

#if defined(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE)
#  if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#    define NOT_MULTIPLE_ALIGNMENT ((_Alignof(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE) > MAX_ALIGNMENT) ? \
                                    MAX_ALIGNMENT : \
                                    _Alignof(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE))
#  elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#    define NOT_MULTIPLE_ALIGNMENT ((alignof(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE) > MAX_ALIGNMENT) ? \
                                    MAX_ALIGNMENT : \
                                    alignof(FXF_NOT_MULTIPLE_ALIGNMENT_TYPE))
#  else
  struct GET_NOT_EVEN_ALIGNMENT_TYPE {
    unsigned char c;
    FXF_NOT_MULTIPLE_ALIGNMENT_TYPE not_even_alignment_member;
  };
#    define NOT_MULTIPLE_ALIGNMENT ((offsetof(struct GET_NOT_EVEN_ALIGNMENT_TYPE, not_even_alignment_member) > MAX_ALIGNMENT) ? \
                                    MAX_ALIGNMENT : \
                                    offsetof(struct GET_NOT_EVEN_ALIGNMENT_TYPE, not_even_alignment_member))
#  endif
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

#if defined(DOS)
/* MSDOS 16 Bit support (maxmemory <= 1 MB) */
#define SEGMENTED
#define ARENA_SEG_SIZE  (CLIP_TO_MAX_POINTER_DIFFERENCE(32000) & ~(MAX_ALIGNMENT - 1U))
#define ARENA_SEG_COUNT ((1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "MSDOS"
#define OSMAXMEM "1 MB"
#elif defined(_WIN98)
/* Win95/Win98/WinME can only allocate chunks up to 255 MB */
/* maxmemory <= 768 MB */
#define SEGMENTED
#define ARENA_SEG_SIZE  (CLIP_TO_MAX_POINTER_DIFFERENCE(1000000) & ~(MAX_ALIGNMENT - 1U))
#define ARENA_SEG_COUNT ((768*1024*1024)/ARENA_SEG_SIZE)
#define OSNAME "Win95/Win98/WinME"
#define OSMAXMEM "768 MB"
#endif

/* The maximum size an fxfAlloc can handle */
/* TODO: Do the macros really accurately determine the maximum we need (apparently 1024 or 2048)?
         Can we instead compute the needed value(s) with expressions involving, say, sizeof(void *)
         and any other system properties we have access to?
*/
enum 
{
  fxfMINSIZE = sizeof(void *), /* Different size of fxfMINSIZE for 32-/64/Bit compilation */
  fxfMAXSIZE =
#if defined(SEGMENTED) || defined(__TURBOC__)
#  if defined(ARENA_SEG_SIZE)
               (((((1024 > (ARENA_SEG_SIZE)) ? (ARENA_SEG_SIZE) : ((size_t)1024))
#  else
               (((((size_t)1024)
#  endif
#else
               (((((size_t)2048) /* This is needed only when sizeof(void*)==8. */
#endif
                                - 1U) & ~(MAX_ALIGNMENT - 1U)) + MAX_ALIGNMENT) /* Round up if necessary. */
};

enum {
  ENSURE_FXFMINSIZE_GT_0 = 1/(fxfMINSIZE > 0),
  ENSURE_FXFMAXSIZE_GE_FXFMINSIZE = 1/(fxfMAXSIZE >= fxfMINSIZE)
};

#define BOTTOM_BIT_OF_FXFMINSIZE ((size_t)fxfMINSIZE & -(size_t)fxfMINSIZE) /* TODO: The top bit would be better below,
                                                                               but can we determine that at compile time? */
#define MIN_ALIGNMENT_UNDERESTIMATE ((NOT_MULTIPLE_ALIGNMENT < BOTTOM_BIT_OF_FXFMINSIZE) ? NOT_MULTIPLE_ALIGNMENT : \
                                                                                           (BOTTOM_BIT_OF_FXFMINSIZE << ((fxfMINSIZE & (fxfMINSIZE - 1U)) && (NOT_MULTIPLE_ALIGNMENT > BOTTOM_BIT_OF_FXFMINSIZE))))

static size_t min_alignment= NOT_MULTIPLE_ALIGNMENT; /* for now */

#define ROUNDED_MIN_SIZE_UNDERESTIMATE (((fxfMINSIZE - 1U) & ~(MIN_ALIGNMENT_UNDERESTIMATE - 1U)) + MIN_ALIGNMENT_UNDERESTIMATE)
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

#define ALIGN_TO_MINIMUM(s)  ((((s) - 1U) & ~(min_alignment - 1U)) + min_alignment)

size_t fxfInit(size_t Size) {
#if defined(LOG)
  static char const * const myname= "fxfInit";
#endif
#if defined(SEGMENTED)
  size_t maxSegCnt= (Size / ARENA_SEG_SIZE);
  if (maxSegCnt > ARENA_SEG_COUNT)
    maxSegCnt= ARENA_SEG_COUNT;
  while (ArenaSegCnt > maxSegCnt) {
    --ArenaSegCnt;
    free(Arena[ArenaSegCnt]);
    Arena[ArenaSegCnt]= Nil(void);
  }
  while (ArenaSegCnt < maxSegCnt) {
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
  GlobalSize= ArenaSegCnt*ARENA_SEG_SIZE;
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
  Size&= ~(MAX_ALIGNMENT - 1U);
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

  while ((min_alignment>>1) >= fxfMINSIZE)
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
  assert((size >= fxfMINSIZE) &&
         (size <= fxfMAXSIZE) &&
         !(size & (min_alignment - 1U)));
  cur_sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  assert((!cur_sh->FreeHead) == (!cur_sh->FreeCount));
  if ((size >= sizeof cur_sh->FreeHead) || !cur_sh->FreeHead) {
    if (size >= sizeof cur_sh->FreeHead)
      memcpy(ptr, &cur_sh->FreeHead, sizeof cur_sh->FreeHead);
    cur_sh->FreeHead= ptr;
    cur_sh->FreeCount++;
    TMDBG(printf(" FreeCount:%lu",cur_sh->FreeCount));
    return 1;
  }
  return 0;
}

static void * popOffFreeStore(size_t const size)
{
  SizeHead *cur_sh;
  void *ptr;
  assert((size >= fxfMINSIZE) &&
         (size <= fxfMAXSIZE) &&
         !(size & (min_alignment - 1U)));
  cur_sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  ptr= cur_sh->FreeHead;
  assert((!ptr) == (!cur_sh->FreeCount));
  if (ptr) {
    cur_sh->FreeCount--;
    if (size < sizeof cur_sh->FreeHead)
      cur_sh->FreeHead= Nil(void);
    else
      memcpy(&cur_sh->FreeHead, ptr, sizeof cur_sh->FreeHead);
  }
  return ptr;
}

void *fxfAlloc(size_t size) {
#if defined(LOG) || defined(DEBUG)
  static char const * const myname= "fxfAlloc";
#endif
  void *ptr= Nil(void);

  TMDBG(printf("fxfAlloc - size:%" SIZE_T_PRINTF_SPECIFIER,(size_t_printf_type)size));
  DBG((stderr, "%s(%" SIZE_T_PRINTF_SPECIFIER ") =", myname, (size_t_printf_type)size));

  if (!size)
    return Nil(void);

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
#  if defined(FREEMAP)
    ClrRange(pointerDifference(ptr, Arena), size);
#  endif
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
        size_t needed_alignment_mask= (NOT_MULTIPLE_ALIGNMENT-1U);
        while (needed_alignment_mask >= size)
          needed_alignment_mask>>= 1;
        curBottomIndex&= needed_alignment_mask;
        if (curBottomIndex) {
          if ((needed_alignment_mask - curBottomIndex) >= (sizeCurrentSeg - size))
            goto NEXT_SEGMENT;
          do {
            size_t const cur_alignment= (curBottomIndex & -curBottomIndex);
#if defined(FREEMAP) && !defined(SEGMENTED)
            SetRange(curBottomIndex,cur_alignment);
#endif
            pushOntoFreeStore(BotFreePtr, cur_alignment);
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
        size_t curBottomIndex= (size_t)pointerDifference(BotFreePtr,Arena[CurrentSeg]);
        while (curBottomIndex & (NOT_MULTIPLE_ALIGNMENT-1U))
        {
          size_t const cur_alignment= (curBottomIndex & -curBottomIndex);
          pushOntoFreeStore(BotFreePtr, cur_alignment);
          BotFreePtr= stepPointer(BotFreePtr, (ptrdiff_t)cur_alignment);
          curBottomIndex+= cur_alignment;
        }
        curBottomIndex= (size_t)pointerDifference(TopFreePtr,BotFreePtr);
        if (curBottomIndex >= fxfMINSIZE)
        {
          assert(!(curBottomIndex & (NOT_MULTIPLE_ALIGNMENT - 1U)));
          pushOntoFreeStore(BotFreePtr, curBottomIndex);
        }
        else if (curBottomIndex)
          TMDBG(printf(" leaking %" SIZE_T_PRINTF_SPECIFIER " byte(s) moving from segment %d to segment %d\n", (size_t_printf_type)curBottomIndex, CurrentSeg, CurrentSeg+1));
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
  SizeHead *sh;

#if defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG)
  ptrdiff_t ptrIndex;
#endif
#if defined(SEGMENTED) && defined(FXF_ENABLE_TMDBG)
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
        ptrIndex= (tmp - segment_begin);
        if (ptrIndex < ARENA_SEG_SIZE)
          goto FOUND_PUTATIVE_SEGMENT;
      }
    } while (0 <= --ptrSegment);
    ptrIndex= -1;
  } else {
    ptrIndex= pointerDifference(ptr,Arena[0]);
    assert((ptrIndex >= 0) && (ptrIndex < ARENA_SEG_SIZE));
  }
FOUND_PUTATIVE_SEGMENT:
  TMDBG(printf("fxfFree - ptr-Arena[%d]:%" PTRDIFF_T_PRINTF_SPECIFIER " size:%" SIZE_T_PRINTF_SPECIFIER,ptrSegment,(ptrdiff_t_printf_type)ptrIndex,(size_t_printf_type)size));
#  endif /*FXF_ENABLE_TMDBG*/
#else /*SEGMENTED*/
#  if defined(FXF_ENABLE_TMDBG) || !defined(NDEBUG)
  ptrIndex= pointerDifference(ptr,Arena);
#  endif
  assert((ptrIndex >= 0) && (ptrIndex < GlobalSize));
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
    assert(size <= (ARENA_SEG_SIZE - ptrIndex));
    assert(((ptrIndex + size) <= pointerDifference(BotFreePtr,Arena[0])) || (ptr >= TopFreePtr));
#  else
  {
    assert(size <= (GlobalSize - ptrIndex));
    assert(((ptrIndex + size) <= pointerDifference(BotFreePtr,Arena)) || (ptr >= TopFreePtr));
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
#if defined(FREEMAP) && !defined(SEGMENTED)
      SetRange((pointerDifference(ptr,Arena),size);
#endif
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
    else {
#if defined(FREEMAP) && !defined(SEGMENTED)
      SetRange(pointerDifference(ptr,Arena),size);
#endif
      pushOntoFreeStore(ptr, size);
    }
  }
  sh= &SizeData[SIZEDATA_SIZE_TO_INDEX(size)];
  sh->MallocCount--;
  TMDBG(printf(" MallocCount:%lu",sh->MallocCount));
  TMDBG(putchar('\n'));
}

void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize) {
  void *nptr;
  size_t original_allocation;
  if (!ptr)
  {
    assert(!OldSize);
    return fxfAlloc(NewSize);
  }
  assert(OldSize && (OldSize <= fxfMAXSIZE));
#if !defined(NDEBUG)
#  if defined(SEGMENTED)
  if (!CurrentSeg) /* Otherwise we'd be relying on converting to convert_pointer_to_int_type,
                      and such calculations aren't guaranteed to provide exactly what we need. */
  {
    ptrdiff_t const ptrIndex= pointerDifference(ptr,Arena[0]);
    assert(ptrIndex < ARENA_SEG_SIZE);
#  else
  {
    ptrdiff_t const ptrIndex= pointerDifference(ptr,Arena);
    assert(ptrIndex < GlobalSize);
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
      assert(allocatedSize <= (ARENA_SEG_SIZE - ptrIndex));
#  else
      assert(allocatedSize <= (GlobalSize - ptrIndex));
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
  if (!NewSize)
  {
    fxfFree(ptr, OldSize);
    return Nil(void);
  }
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
  nptr= fxfAlloc(NewSize);
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
        UsedBytes+= hd->MallocCount*SIZEDATA_INDEX_TO_SIZE(i);
        nrFree+= hd->FreeCount;
        FreeBytes+= hd->FreeCount*SIZEDATA_INDEX_TO_SIZE(i);
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
