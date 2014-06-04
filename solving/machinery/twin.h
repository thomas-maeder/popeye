#if !defined(SOLVING_MACHINERY_TWIN_H)
#define SOLVING_MACHINERY_TWIN_H

#include "stipulation/stipulation.h"

typedef unsigned long twin_number_type;

/* Sequence number of the current twin.
 * Mainly used for boolean twin-related flags: flag==twin_number means true,
 * and everything else means false. This allows us to not reset the flag at the
 * beginning (or end) of a twin.
 */
extern twin_number_type twin_number;

typedef enum
{
  twin_initial,
  twin_subsequent
} twin_context_type;

/* Solve the stipulation
 * @param stipulation_root_hook identifies the root slice of the stipulation
 * @param context context of the twin to be solved
 */
void twin_solve_stipulation(slice_index stipulation_root_hook);

/* Assign every piece of the position their id
 */
void initialise_piece_ids(void);


/* Assign pieces flags as defined by the conditions
 */
void initialise_piece_flags_from_conditions(void);

#endif
