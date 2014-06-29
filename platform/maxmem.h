#if !defined(MAXMEM_H)
#define MAXMEM_H

#include "utilities/boolean.h"

typedef unsigned long maxmem_kilos_type;

extern maxmem_kilos_type const one_mega;
extern maxmem_kilos_type const one_giga;
extern maxmem_kilos_type const nothing_requested;

/* Allocate memory for the hash table, based on the -maxmem command
 * line value (if any) and information retrieved from the operating
 * system.
 * @return false iff the user requested for an amount of hash table
 *         memory, but we can't allocated that much
 */
boolean dimensionHashtable(void);

/* Make a guess for a reasonable amount of memory for the hashtable.
 * Implemented separately for each platform.
 * Invoked by maxmem module; don't invoke directly.
 * @return number of kilo-bytes to be used
 */
unsigned long guessReasonableMaxmemory(void);

/* request an amount of memory
 * @param requested number of kilo-bytes requested
 */
void requestMemory(maxmem_kilos_type requested);

/* Retrieve amount of memory actually allocated
 * @return amount
 */
maxmem_kilos_type getAllocatedMemory(void);

#endif
