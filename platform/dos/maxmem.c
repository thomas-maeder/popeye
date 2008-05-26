#include "../maxmem.h"

size_t adjustMaxmemory(size_t usersetting)
{
  size_t const dosDefaultMaxMem = 256u*1024;
  return usersetting==0 ? dosDefaultMaxMem : usersetting;
}
