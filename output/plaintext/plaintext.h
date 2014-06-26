#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "stipulation/stipulation.h"
#include "stipulation/goals/goals.h"
#include "solving/move_effect_journal.h"

extern char versionString[100];

/* Context are used to visually group the output related to one or more effects,
 * i.e. the flags, walk and rebirth square of a Circe rebirth ("[Sb8]").
 */
typedef struct
{
    move_effect_journal_index_type start;
    char const * closing_sequence;
} output_plaintext_move_context_type;

/* Write the current move
 */
void output_plaintext_write_move(void);

/* Determine whether a goal writer slice should replace the check writer slice
 * which would normally following the possible check deliverd by the move just
 * played (i.e. if both a possible check and the symbol for the reached goal
 * should be written).
 * @param goal goal written by goal writer
 * @return true iff the check writer should be replaced by the goal writer
 */
boolean output_plaintext_goal_writer_replaces_check_writer(goal_type goal);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void output_plaintext_write_position(slice_index si);

/* Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si);

#endif
