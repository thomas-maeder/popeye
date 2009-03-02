#include "../maxmem.h"
#include <windows.h>

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  size_t availablePhysicalMemory;
  size_t totalPhysicalMemory;

  MEMORYSTATUS Mem;
  GlobalMemoryStatus(&Mem);
  availablePhysicalMemory = Mem.dwAvailPhys;
  totalPhysicalMemory = Mem.dwTotalPhys;

#if defined(_WIN98)
  /* WIN98 cannot handle more than 768MB */
  totalPhysicalMemory = 700ul*1024*1024;
  if (availablePhysicalMemory>totalPhysicalMemory)
    availablePhysicalMemory = totalPhysicalMemory;
#endif  /* _WIN98 */

  if (usersetting==0)
    return (unsigned long)availablePhysicalMemory;
  else if (usersetting>totalPhysicalMemory)
    return (unsigned long)totalPhysicalMemory;
  else
    return usersetting;
}
