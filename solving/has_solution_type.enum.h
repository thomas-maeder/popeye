#if !defined(SOLVING_HAS_SOLUTION_TYPE_ENUM_H)
#define SOLVING_HAS_SOLUTION_TYPE_ENUM_H

typedef enum
{
 has_solution_type_0, previous_move_is_illegal, immobility_on_next_move, previous_move_has_solved, next_move_has_solution, previous_move_has_not_solved, next_move_has_no_solution, length_unspecified=previous_move_has_solved, this_move_is_illegal=previous_move_is_illegal+1
} has_solution_type;
extern char const *has_solution_type_names[];
/* include has_solution_type.enum to make sure that all the dependencies are generated correctly: */
#include "has_solution_type.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
