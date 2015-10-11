#include "platform/beep.h"
#include "platform/beep_impl.h"

#include <stdio.h>
#include <windows.h>

void platform_produce_beep(void)
{
  while (nr_beeps>0)
  {
	fputs("\a",stderr);
	--nr_beeps;
  }

  Sleep(500); /* ms */
}
