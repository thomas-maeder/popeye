#include "../maxmem.h"

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  unsigned long const unixDefaultMaxMem = 2*1024*1024;
  return usersetting==0 ? unixDefaultMaxMem : usersetting;
}
