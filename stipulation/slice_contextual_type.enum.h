#if !defined(STIPULATION_SLICE_CONTEXTUAL_TYPE_ENUM_H)
#define STIPULATION_SLICE_CONTEXTUAL_TYPE_ENUM_H

typedef enum
{
 slice_contextual_unspecified, slice_contextual_binary, slice_contextual_testing_pipe, slice_contextual_conditional_pipe, slice_contextual_end_of_branch, nr_slice_contextual_types
} slice_contextual_type;
extern char const *slice_contextual_type_names[];
/* include slice_contextual_type.enum to make sure that all the dependencies are generated correctly: */
#include "slice_contextual_type.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
