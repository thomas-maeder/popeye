#include "platform/beep.h"
#include "platform/beep_impl.h"

#include <stdio.h>
#include <unistd.h>	/* declaration of sleep()  */

void produce_beep(void)
{
  while (nr_beeps>0)
  {
	fprintf(stderr,"\a");
	--nr_beeps;
  }

  sleep(1);
}
