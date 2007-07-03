int fxfInit(unsigned long GlobalSize);
void *fxfAlloc(int size);
void *fxfReAlloc(void *ptr, int OldSize, int NewSize);
int fxfFree(void *ptr, int size);
void fxfInfo(FILE *);
unsigned long fxfTotal();
