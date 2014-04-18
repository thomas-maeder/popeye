#if !defined(STIPULATION_SLICE_H)
#define STIPULATION_SLICE_H

#include "utilities/boolean.h"
#include "stipulation/slice_type.h"
#include "stipulation/slice_structural_type.h"
#include "stipulation/slice_functional_type.h"

/* Retrieve the structural type of a slice type
 * @param type identifies slice type of which to retrieve structural type
 * @return structural type of slice type type
 */
slice_structural_type slice_type_get_structural_type(slice_type type);

/* Retrieve the functional type of a slice type
 * @param type identifies slice type of which to retrieve structural type
 * @return functional type of slice type type
 */
slice_functional_type slice_type_get_functional_type(slice_type type);

/* Initialise slice properties at start of program */
void initialise_slice_properties(void);

#endif
