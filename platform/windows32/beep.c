#include "platform/beep.h"
#include "platform/beep_impl.h"

#include <stdio.h>
#include <windows.h>

void produce_beep(void)
{
  while (nr_beeps>0)
  {
	fprintf(stderr,"\a");
	--nr_beeps;
  }

  Sleep(500); /* ms */
}
