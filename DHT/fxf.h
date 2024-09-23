#if !defined(FXF_H)
#define FXF_H

#include <stddef.h>
#include <stdio.h>

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#  define ALIGNMENT_OF_TYPE(type) _Alignof(type)
#elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#  define ALIGNMENT_OF_TYPE(type) alignof(type)
#else
  /* This technique for getting a type's alignment is taken from Steve Jessop's comment at
     https://stackoverflow.com/a/228015/1019990.
     We take the bottom bit in case the calculated value is some multiple of the correct alignment;
     this should at least be closer to the correct value. */
#  define ALIGNMENT_OF_TYPE(type) (offsetof(struct {unsigned char c; type t;}, t) & -offsetof(struct {unsigned char c; type t;}, t))
#endif

size_t fxfInit(size_t Size); /* returns the number of bytes actually allocated */
int fxfInitialised(void);
void *fxfAllocRaw(size_t size, size_t desired_alignment);
void *fxfReAllocRaw(void *ptr, size_t OldSize, size_t NewSize, size_t desired_alignment);
#define fxfAlloc(size, type) ((type *) fxfAllocRaw(size, ALIGNMENT_OF_TYPE(type)))
#define fxfReAlloc(ptr, OldSize, NewSize, type) ((type *) fxfReAllocRaw(ptr, OldSize, NewSize, ALIGNMENT_OF_TYPE(type)))
void fxfFree(void *ptr, size_t size);
void fxfInfo(FILE *);
size_t fxfTotal(void);
size_t fxfMaxAllocation(void);

/* Reset the internal data structures to the state that was reached
 * after the latest call to fxfInit() */
void fxfReset(void);

/* Free all the associated memory. */
void fxfTeardown(void);

#endif
