#if !defined(FXF_H)
#define FXF_H

#include <stddef.h>

int fxfInit(size_t GlobalSize); /* TODO: This currently returns -1 in one specific error case and 0 in all other cases.
                                         Should it signal other error cases? */
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
