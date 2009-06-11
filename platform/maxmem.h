#if !defined(MAXMEM_H)
#define MAXMEM_H

#include <stddef.h>

/* Make a guess for a reasonable amount of memory for the hashtable
 * @return number of kilo-bytes to be used
 */
extern unsigned long guessReasonableMaxmemory(void);

#endif
