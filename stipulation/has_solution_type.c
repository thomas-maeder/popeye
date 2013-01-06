#include "stipulation/has_solution_type.h"

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                                              \
    ENUMERATOR(has_solution_type_0),                             \
    ENUMERATOR(previous_move_is_illegal),                        \
    ENUMERATOR(immobility_on_next_move),                         \
    ENUMERATOR(previous_move_has_solved),                        \
    ENUMERATOR(next_move_has_solution),                          \
    ENUMERATOR(previous_move_has_not_solved),                    \
    ENUMERATOR(next_move_has_no_solution),                       \
    ASSIGNED_ENUMERATOR(length_unspecified=next_move_has_solution)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"
