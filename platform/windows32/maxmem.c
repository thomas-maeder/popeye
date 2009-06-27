#include "../maxmem.h"
#include <limits.h>
#include <windows.h>

/* Make a guess for a reasonable amount of memory for the hashtable
 * @return number of kilo-bytes to be used
 */
unsigned long guessReasonableMaxmemory(void)
{
  unsigned long const one_kilo = 1024;
  unsigned long const one_giga = 1024*1024;
  unsigned long result = one_giga;

  /* GlobalMemoryStatusEx() has only been available since Windows 2000
   */
#if defined(_WIN32_WINNT) && _WIN32_WINNT>=0x0500

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
