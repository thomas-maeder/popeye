#include "platform/maxmem.h"
#include <limits.h>
#include <windows.h>

/* Make a guess for a reasonable amount of memory for the hashtable
 * @return number of kilo-bytes to be used
 */
unsigned long platform_guess_reasonable_maxmemory(void)
{
  unsigned long const one_giga = 1024*1024;
  unsigned long result = one_giga;

  MEMORYSTATUSEX memstatEx;
  memstatEx.dwLength = sizeof memstatEx;
  if (GlobalMemoryStatusEx(&memstatEx))
  {
    unsigned long const one_kilo = 1024;
    unsigned long long const available_kilos = memstatEx.ullAvailPhys/one_kilo;
    /* we are really erring on the safe side here :-) */
    if (available_kilos<=ULONG_MAX)
      result = (unsigned long)available_kilos;
  }

  return result;
}
