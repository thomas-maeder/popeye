#if !defined(SOLVING_MACHINERY_TWIN_H)
#define SOLVING_MACHINERY_TWIN_H

#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"

typedef unsigned long twin_number_type;

/* Sequence number of the current twin.
 * Mainly used for boolean twin-related flags: flag==twin_number means true,
 * and everything else means false. This allows us to not reset the flag at the
 * beginning (or end) of a twin.
 */
extern twin_number_type twin_number;

/* is the current twin a continued twin? */
extern boolean twin_is_continued;

/* Validate whether shifting the entire position would move >=1 piece off board
 * @return true iff it doesn't
 */
boolean twin_twinning_shift_validate(square from, square to);

/* Solve the stipulation
 * @param stipulation_root_hook identifies the root slice of the stipulation
 * @param context context of the twin to be solved
 */
void twin_solve_stipulation(slice_index stipulation_root_hook);

/* Assign every piece of the position their id
 */
void initialise_piece_ids(void);

/* Initialise piece flags from conditions, the pieces themselve etc.
 */
void initialise_piece_flags(void);

#endif
