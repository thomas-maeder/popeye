#include "platform/maxmem.h"
#include "optimisations/hash.h"
#include "solving/pipe.h"

#include <limits.h>

maxmem_kilos_type const one_mega = 1<<10;
maxmem_kilos_type const one_giga = 1<<20;

/* Singular value indiciating that the user made no request for a
 * maximal amount of memory to be allocated for the hash table.
 * We don't use 0 because the user may indicate -maxmem 0 to prevent a hash
 * table from being used at all.
 */
maxmem_kilos_type const nothing_requested = ULONG_MAX;

/* Amount of memory requested by the user
 */
static maxmem_kilos_type amountMemoryRequested = ULONG_MAX;

/* Amount of memory actually allocated
 */
static maxmem_kilos_type amountMemoryAllocated;

/* request an amount of memory
 * @param requested number of kilo-bytes requested
 */
void platform_request_memory(maxmem_kilos_type requested)
{
  amountMemoryRequested = requested;
}

/* Allocate memory for the hash table, based on the -maxmem command
 * line value (if any) and information retrieved from the operating
 * system.
 * @return false iff the user requested for an amount of hash table
 *         memory, but we can't allocated that much
 */
static boolean dimensionHashtable(void)
{
  boolean result = true;

  if (amountMemoryRequested==nothing_requested)
  {
    unsigned int const amountMemoryGuessed = platform_guess_reasonable_maxmemory();
    amountMemoryAllocated = allochash(amountMemoryGuessed);
  }
  else
  {
    amountMemoryAllocated = allochash(amountMemoryRequested);
    result = amountMemoryAllocated>=amountMemoryRequested;
  }

  return result;
}

void hashtable_dimensioner_solve(slice_index si)
{
  if (dimensionHashtable())
    pipe_solve_delegate(si);
  else
    fputs("Couldn't allocate the requested amount of memory\n",stdout);
}

/* Retrieve amount of memory actually allocated
 * @return amount
 */
maxmem_kilos_type platform_get_allocated_memory(void)
{
  return amountMemoryAllocated;
}

