#include "../maxmem.h"
#include <windows.h>

size_t adjustMaxmemory(size_t usersetting)
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
    return availablePhysicalMemory;
  else if (usersetting>totalPhysicalMemory)
    return totalPhysicalMemory;
  else
    return usersetting;
}
