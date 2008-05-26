#include "../maxmem.h"

size_t adjustMaxmemory(size_t usersetting)
{
  size_t const unixDefaultMaxMem = 2*1024*1024;
  return usersetting==0 ? unixDefaultMaxMem : usersetting;
}
