#include "platform/maxmem.h"

/* Make a guess for a reasonable amount of memory for the hashtable
 * @return number of kilo-bytes to be used
 */
unsigned long guessReasonableMaxmemory(void)
{
  unsigned long const dosDefaultMaxMem = 256u;

  return dosDefaultMaxMem;
}
