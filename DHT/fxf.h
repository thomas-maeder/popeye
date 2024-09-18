#if !defined(FXF_H)
#define FXF_H

#include <stddef.h>
#include <stdio.h>

size_t fxfInit(size_t Size); /* returns the number of bytes actually allocated */
int fxfInitialised(void);
void *fxfAllocWithAlignment(size_t size, size_t alignment);
void *fxfReAllocWithAlignment(void *ptr, size_t OldSize, size_t NewSize, size_t alignment);
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#  define fxfAlloc(size, type) fxfAllocWithAlignment(size, _Alignof(type))
#  define fxfReAlloc(ptr, OldSize, NewSize, type) fxfReAllocWithAlignment(ptr, OldSize, NewSize, _Alignof(type))
#elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#  define fxfAlloc(size, type) fxfAllocWithAlignment(size, alignof(type))
#  define fxfReAlloc(size, type) fxfReAllocWithAlignment(ptr, OldSize, NewSize, alignof(type))
#else
#  define fxfAlloc(size, type) fxfAllocWithAlignment(size, offsetof(struct {unsigned char c; type t;}, t))
#  define fxfReAlloc(size, type) fxfReAllocWithAlignment(ptr, OldSize, NewSize, offsetof(struct {unsigned char c; type t;}, t))
#endif
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
