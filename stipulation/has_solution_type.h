#if !defined(STIPULATION_has_solution_type_h)
#define STIPULATION_has_solution_type_h

/* Number of slack half-moves in stipulation length indications:
 */
enum
{
  slack_length = 3
};


/* Enumeration type used to indicate used for parameter and return values of
 * function solve() in cases where no exact length of play is known
 */

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                                              \
    ENUMERATOR(has_solution_type_0),                             \
    ENUMERATOR(previous_move_is_illegal),                        \
    ENUMERATOR(immobility_on_next_move),                         \
    ENUMERATOR(previous_move_has_solved),                        \
    ENUMERATOR(next_move_has_solution),                          \
    ENUMERATOR(previous_move_has_not_solved),                    \
    ENUMERATOR(next_move_has_no_solution),                       \
    ASSIGNED_ENUMERATOR(length_unspecified=next_move_has_solution), \
    ASSIGNED_ENUMERATOR(this_move_is_illegal=previous_move_is_illegal+1)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

#endif
