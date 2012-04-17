#if !defined(STIPULATION_SLICE_H)
#define STIPULATION_SLICE_H

#include "py.h"
#include "stipulation/slice_type.h"

#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                                              \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_leaf),                            \
    ENUMERATOR(slice_structure_binary),                          \
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
  ENUMERATOR(slice_function_testing_pipe),                       \
  ENUMERATOR(slice_function_conditional_pipe),                   \
  ENUMERATOR(slice_function_writer),                             \
  ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

/* Retrieve the structural type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_structural_type slice_get_structural_type(slice_index si);


/* Retrieve the functional type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_functional_type slice_get_functional_type(slice_index si);

/* Allocate a slice index
 * @param type which slice type
 * @return a so far unused slice index
 */
slice_index alloc_slice(slice_type type);

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si);

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si);

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original);

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred);

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void);

#endif
