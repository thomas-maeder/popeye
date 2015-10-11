#if !defined(MAXMEM_H)
#define MAXMEM_H

#include "utilities/boolean.h"
#include "stipulation/stipulation.h"

typedef unsigned long maxmem_kilos_type;

extern maxmem_kilos_type const one_mega;
extern maxmem_kilos_type const one_giga;
extern maxmem_kilos_type const nothing_requested;

/* Make a guess for a reasonable amount of memory for the hashtable.
 * Implemented separately for each platform.
 * Invoked by maxmem module; don't invoke directly.
 * @return number of kilo-bytes to be used
 */
unsigned long platform_guess_reasonable_maxmemory(void);

/* request an amount of memory
 * @param requested number of kilo-bytes requested
 */
void platform_request_memory(maxmem_kilos_type requested);

void hashtable_dimensioner_solve(slice_index si);

/* Retrieve amount of memory actually allocated
 * @return amount
 */
maxmem_kilos_type platform_get_allocated_memory(void);

#endif
