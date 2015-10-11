#include "platform/beep.h"
#include "platform/beep_impl.h"

#include <stdio.h>
#include <unistd.h>	/* declaration of sleep()  */

void platform_produce_beep(void)
{
  while (nr_beeps>0)
  {
	fputs("\a",stderr);
	--nr_beeps;
  }

  sleep(1);
}
