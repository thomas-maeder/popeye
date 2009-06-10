#include "../maxmem.h"

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  unsigned long const dosDefaultMaxMem = 256u;
  return usersetting==0 ? dosDefaultMaxMem : usersetting;
}
