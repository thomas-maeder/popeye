#include "platform/maxmem.h"
#include <limits.h>
#include <windows.h>

/* Some header <sdkddkver.h> defines symbols like _WIN32_WINNT_WIN2K,
 * but this header isn't available with all C implementations
 * targeting Windows; so let's define the symbol here:
 */
#define _WIN32_WINNT_WIN2K 0x0500

/* Make a guess for a reasonable amount of memory for the hashtable
 * @return number of kilo-bytes to be used
 */
unsigned long platform_guess_reasonable_maxmemory(void)
{
  unsigned long const one_kilo = 1024;
  unsigned long const one_giga = 1024ul*1024ul;
  unsigned long result = one_giga;

  /* GlobalMemoryStatusEx() has only been available since Windows 2000
   */
#if defined(_WIN32_WINNT) && _WIN32_WINNT>=_WIN32_WINNT_WIN2K

  MEMORYSTATUSEX memstatEx;
  memstatEx.dwLength = sizeof memstatEx;
  if (GlobalMemoryStatusEx(&memstatEx))
  {
    unsigned long long const available_kilos = memstatEx.ullAvailPhys/one_kilo;
    /* we are really erring on the safe side here :-) */
    if (available_kilos<=ULONG_MAX)
      result = (unsigned long)available_kilos;
  }

#else

  MEMORYSTATUS memstat;
  GlobalMemoryStatus(&memstat);
  result = memstat.dwAvailPhys/one_kilo;

#endif

  return result;
}
