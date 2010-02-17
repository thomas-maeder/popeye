#if !defined(OPTIMISATIONS_MAXSOLUTIONS_ROOT_DEFENDER_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_ROOT_DEFENDER_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with
 * STMaxSolutionsRootDefenderFilter stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsRootDefenderFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_root_defender_filter(void);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean maxsolutions_root_defender_filter_defend(slice_index si);

#endif
