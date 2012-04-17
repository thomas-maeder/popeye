#include "stipulation/has_solution_type.h"

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                                              \
    ENUMERATOR(has_solution_type_0),                             \
    ENUMERATOR(opponent_self_check),                             \
    ENUMERATOR(has_solution_type_2),                             \
    ENUMERATOR(has_solution),                                    \
    ENUMERATOR(has_solution_type_4),                             \
    ENUMERATOR(has_no_solution),                                 \
    ASSIGNED_ENUMERATOR(length_unspecified=slack_length)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"
