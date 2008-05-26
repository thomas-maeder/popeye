#include "../maxmem.h"
#include <windows.h>

size_t adjustMaxmemory(size_t usersetting)
{
  size_t availablePhysicalMemory;
  size_t totalPhysicalMemory;

  MEMORYSTATUSEX MemEx;
  MemEx.dwLength = sizeof MemEx;
  if (GlobalMemoryStatusEx(&MemEx))
  {
    availablePhysicalMemory = MemEx.ullAvailPhys;
    totalPhysicalMemory = MemEx.ullTotalPhys;
  }
  else
  {
    availablePhysicalMemory = 2ull*1024*1024*1024; /* wild guess: 2G */
    totalPhysicalMemory = (size_t)-1;
  }

  if (usersetting==0)
    return availablePhysicalMemory;
  else if (usersetting>totalPhysicalMemory)
    return totalPhysicalMemory;
  else
    return usersetting;
}
