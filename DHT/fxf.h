#if !defined(FXF_H)
#define FXF_H

#include <stddef.h>

int fxfInit(unsigned long GlobalSize);
void *fxfAlloc(size_t size);
void *fxfReAlloc(void *ptr, size_t OldSize, size_t NewSize);
void fxfFree(void *ptr, size_t size);
void fxfInfo(FILE *);
unsigned long fxfTotal();

/* Reset the internal data structures to the state that was reached
 * after the latest call to fxfInit() */
void fxfReset(void);

#endif
