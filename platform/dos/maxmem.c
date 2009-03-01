#include "../maxmem.h"

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  unsigned long const dosDefaultMaxMem = 256u*1024;
  return usersetting==0 ? dosDefaultMaxMem : usersetting;
}
