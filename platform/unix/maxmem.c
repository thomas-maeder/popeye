#include "../maxmem.h"

unsigned long adjustMaxmemory(unsigned long usersetting)
{
  unsigned long result;
  unsigned long const one_giga = 1024*1024;
  
  if (usersetting==0)
    result = one_giga;
  else
    result = usersetting;

  return result;
}
