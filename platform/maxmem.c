#include "maxmem.h"
#include "../pyhash.h"

#include <stdio.h>
#include <stdlib.h>

typedef unsigned long maxmem_kilos_type;

static maxmem_kilos_type const one_mega = 1<<10;
static maxmem_kilos_type const one_giga = 1<<20;

/* Singular value indiciating that the user made no request for a
 * maximal amount of memory to be allocated for the hash table.
 */
static maxmem_kilos_type const nothing_requested = 0;

/* Amount of memory requested by the user
 */
static maxmem_kilos_type amountMemoryRequested;

/* Amount of memory actually allocated, in human-readable format
 */
static char MaxMemoryString[37];

/* Initialize maxmem module.
 * To be called before any other function of this module.
 */
void initMaxmem(void)
{
  amountMemoryRequested = nothing_requested;
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
void readMaxmem(char const *commandLineValue)
{
  char *end;
  amountMemoryRequested = strtoul(commandLineValue,&end,10);
  if (commandLineValue==end)
    /* conversion failure
     * -> set to 0 now and to default value further down */
    amountMemoryRequested = nothing_requested;
  else if (*end=='G')
    amountMemoryRequested *= one_giga;
  else if (*end=='M')
    amountMemoryRequested *= one_mega;
}

/* Initialise string containing hash table memory actually allocated
 * in human-readable form
 * @param amountMemoryAllocated hash table memory actually allocated
 */
static void initMaxMemoryString(maxmem_kilos_type amountMemoryAllocated)
{
  /* We do not issue our startup message via the language
     dependant Msg-Tables, since there the version is
     too easily changed, or not updated.
  */
  if (amountMemoryAllocated>=10*one_giga)
    sprintf(MaxMemoryString, " (%lu GB)\n", amountMemoryAllocated/one_giga);
  else if (amountMemoryAllocated>=10*one_mega)
    sprintf(MaxMemoryString, " (%lu MB)\n", amountMemoryAllocated/one_mega);
  else
    sprintf(MaxMemoryString, " (%lu KB)\n", amountMemoryAllocated);
}

/* Allocate memory for the hash table, based on the -maxmem command
 * line value (if any) and information retrieved from the operating
 * system.
 */
void dimensionHashtable(void)
{
  maxmem_kilos_type amountMemoryAllocated;

  if (amountMemoryRequested==nothing_requested)
    amountMemoryRequested = guessReasonableMaxmemory();

  amountMemoryAllocated = allochash(amountMemoryRequested);
  
  initMaxMemoryString(amountMemoryAllocated);
}

/* Retrieve a human-readable indication of the maximal amount of
 * memory used by the hash table.
 * Only invoke after dimensionHashtable().
 * @return maximal amount of hash table memory, in human readable format
 */
char const *maxmemString(void)
{
  return MaxMemoryString;
}
