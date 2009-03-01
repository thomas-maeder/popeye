#include "../maxmem.h"
#include <windows.h>

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  unsigned long availablePhysicalMemory;
  unsigned long totalPhysicalMemory;

  MEMORYSTATUSEX MemEx;
  MemEx.dwLength = sizeof MemEx;
  if (GlobalMemoryStatusEx(&MemEx))
  {
    availablePhysicalMemory = MemEx.ullAvailPhys;
    totalPhysicalMemory = MemEx.ullTotalPhys;
  }
  else
  {
    availablePhysicalMemory = 2ul*1024*1024*1024; /* wild guess: 2G */
    totalPhysicalMemory = ULONG_MAX;
  }

  if (usersetting==0)
    return availablePhysicalMemory;
  else if (usersetting>totalPhysicalMemory)
    return totalPhysicalMemory;
  else
    return usersetting;
}
