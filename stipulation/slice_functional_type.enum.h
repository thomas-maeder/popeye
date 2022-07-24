#if !defined(STIPULATION_SLICE_FUNCTIONAL_TYPE_ENUM_H)
#define STIPULATION_SLICE_FUNCTIONAL_TYPE_ENUM_H

typedef enum
{
 slice_function_unspecified, slice_function_proxy, slice_function_move_generator, slice_function_move_reordering_optimiser, slice_function_move_removing_optimiser, slice_function_writer, slice_function_output_plaintext_position_writer, nr_slice_functional_types
} slice_functional_type;
extern char const *slice_functional_type_names[];
/* include slice_functional_type.enum to make sure that all the dependencies are generated correctly: */
#include "slice_functional_type.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
