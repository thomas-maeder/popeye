#include "platform/maxmem.h"
#include "optimisations/hash.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned long maxmem_kilos_type;

static maxmem_kilos_type const one_mega = 1<<10;
static maxmem_kilos_type const one_giga = 1<<20;

/* Singular value indiciating that the user made no request for a
 * maximal amount of memory to be allocated for the hash table.
 * We don't use 0 because the user may indicate -maxmem 0 to prevent a hash
 * table from being used at all.
 */
static maxmem_kilos_type const nothing_requested = ULONG_MAX;

/* Amount of memory requested by the user
 */
static maxmem_kilos_type amountMemoryRequested = ULONG_MAX;

/* Amount of memory actually allocated, in human-readable format
 */
static char MaxMemoryString[37];

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
 * @return false iff the user requested for an amount of hash table
 *         memory, but we can't allocated that much
 */
boolean dimensionHashtable(void)
{
  boolean result = true;
  maxmem_kilos_type amountMemoryAllocated;

  if (amountMemoryRequested==nothing_requested)
  {
    unsigned int const amountMemoryGuessed = guessReasonableMaxmemory();
    amountMemoryAllocated = allochash(amountMemoryGuessed);
  }
  else
  {
    amountMemoryAllocated = allochash(amountMemoryRequested);
    result = amountMemoryAllocated>=amountMemoryRequested;
  }

  initMaxMemoryString(amountMemoryAllocated);

  return result;
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
