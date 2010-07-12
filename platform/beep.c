#include "platform/beep.h"
#include "platform/beep_impl.h"

#include <limits.h>
#include <stdlib.h>

/* Number of beeps to be produced
 */
unsigned int nr_beeps = 1;

/* Interpret beep command line parameter value
 * @param optionValue value of option beep
 */
boolean read_nr_beeps(char const *optionValue)
{
  boolean result = false;

  char *end;
  unsigned long const nr_beepsRequested = strtoul(optionValue,&end,10);
  if (optionValue!=end && nr_beepsRequested<=UINT_MAX)
  {
    nr_beeps = (unsigned int)nr_beepsRequested;
    result = true;
  }
  else
    nr_beeps = 1;

  return result;
}
