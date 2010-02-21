#if !defined(PYDIRECT_H)
#define PYDIRECT_H

/* Interface for dynamically dispatching direct play functions to slices
 * depending on the slice type
 */

#include "boolean.h"
#include "pystip.h"

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean direct_defender_root_defend(slice_index si);

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean direct_defender_defend_in_n(slice_index si, stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int direct_defender_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             unsigned int max_result);

#endif
