#if !defined(STIPULATION_SLICE_STRUCTURAL_TYPE_ENUM_H)
#define STIPULATION_SLICE_STRUCTURAL_TYPE_ENUM_H

typedef enum
{
 slice_structure_pipe, slice_structure_leaf, slice_structure_branch, slice_structure_fork, nr_slice_structure_types
} slice_structural_type;
extern char const *slice_structural_type_names[];
/* include slice_structural_type.enum to make sure that all the dependencies are generated correctly: */
#include "slice_structural_type.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
