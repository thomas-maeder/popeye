#if !defined(FXF_H)
#define FXF_H

#include <stddef.h>

size_t fxfInit(size_t Size); /* returns the number of bytes actually allocated */
int fxfInitialised(void);
void *fxfAlloc(size_t size);
void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize);
void fxfFree(void *ptr, size_t size);
void fxfInfo(FILE *);
size_t fxfTotal(void);

/* Reset the internal data structures to the state that was reached
 * after the latest call to fxfInit() */
void fxfReset(void);

/* Free all the associated memory. */
void fxfTeardown(void);

#endif
