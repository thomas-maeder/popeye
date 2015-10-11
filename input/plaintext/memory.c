#include "input/plaintext/memory.h"
#include "platform/maxmem.h"

#include <stdlib.h>

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
void input_plaintext_read_requested_memory(char const *commandLineValue)
{
  char *end;
  maxmem_kilos_type requested = strtoul(commandLineValue,&end,10);
  if (commandLineValue==end)
    /* conversion failure
     * -> set to 0 now and to default value further down */
    requested = nothing_requested;
  else if (*end=='G')
    requested *= one_giga;
  else if (*end=='M')
    requested *= one_mega;

  platform_request_memory(requested);
}
