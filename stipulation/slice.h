#if !defined(STIPULATION_SLICE_H)
#define STIPULATION_SLICE_H

#include "py.h"
#include "stipulation/slice_type.h"

#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                                              \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_leaf),                            \
    ENUMERATOR(slice_structure_branch),                          \
    ENUMERATOR(slice_structure_fork),                            \
    ENUMERATOR(nr_slice_structure_types)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"


#define ENUMERATION_TYPENAME slice_functional_type
#define ENUMERATORS                                              \
  ENUMERATOR(slice_function_unspecified),                        \
  ENUMERATOR(slice_function_proxy),                              \
  ENUMERATOR(slice_function_move_generator),                     \
  ENUMERATOR(slice_function_move_reordering_optimiser),          \
  ENUMERATOR(slice_function_move_removing_optimiser),            \
  ENUMERATOR(slice_function_binary),                             \
  ENUMERATOR(slice_function_testing_pipe),                       \
  ENUMERATOR(slice_function_conditional_pipe),                   \
  ENUMERATOR(slice_function_end_of_branch),                      \
  ENUMERATOR(slice_function_writer),                             \
  ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

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
