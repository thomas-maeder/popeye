#if !defined(STIPULATION_HAS_SOLUTION_TYPE_h)
#define STIPULATION_HAS_SOLUTION_TYPE_h

/* Number of slack half-moves in stipulation length indications:
 */
enum
{
  slack_length = 3
};


/* Enumeration type used to indicate a fictional number of moves in situations
 * where no number of moves is known (yet). E.g.
 * attack(some_slice_index,length_unspecified) returns
 * - a check to the side not to move has been detected
 * - has_solution if >=1 solution has been found
 * - has_no_solution otherwise
 */

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                                              \
    ENUMERATOR(has_solution_type_0),                             \
    ENUMERATOR(opponent_self_check),                             \
    ENUMERATOR(has_solution_type_2),                             \
    ENUMERATOR(has_solution),                                    \
    ENUMERATOR(has_solution_type_4),                             \
    ENUMERATOR(has_no_solution),                                 \
    ASSIGNED_ENUMERATOR(length_unspecified=slack_length)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

#endif
