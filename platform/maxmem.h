#if !defined(MAXMEM_H)
#define MAXMEM_H

#include <stddef.h>

/* Adjust the maxmem setting (possibly) indicated by the user to the
 * currently available physical memory.
 * @param usersetting value of maxmem setting (command line or
 *                    option); 0 if the user didn't indicate maxmem
 * @return number of bytes to actually be used
 * @note both usersetting and the return value are in Bytes
 */
extern unsigned long adjustMaxmemory(unsigned long usersetting);

#endif
