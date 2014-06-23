#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "stipulation/stipulation.h"
#include "stipulation/goals/goals.h"
#include "solving/move_effect_journal.h"

#include <stdio.h>

extern FILE *TraceFile;

extern char versionString[100];

/* Context are used to visually group the output related to one or more effects,
 * i.e. the flags, walk and rebirth square of a Circe rebirth ("[Sb8]").
 */
typedef struct
{
    move_effect_journal_index_type start;
    char const * closing_sequence;
    FILE *file;
} output_plaintext_move_context_type;

/* Open a context
 * @param context address of the structure holding the context's attributes
 * @param start identifies the first effect to be visualized withing the context
 *              to be opened
 * @param opening_sequence e.g. "["
 * @param closing_sequence e.g. "]"
 */
void output_plaintext_context_open(FILE *file,
                                   output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type start,
                                   char const *opening_sequence,
                                   char const *closing_sequence);

/* Close a context
 * @param context address of the structure holding the context's attributes
 */
void output_plaintext_context_close(output_plaintext_move_context_type *context);

/* Close the current context and open the next one
 */
void output_plaintext_next_context(output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type start,
                                   char const *opening_sequence,
                                   char const *closing_sequence);

/* Write a complete piece:
 * - flags incl. color
 * - walk
 * - position
 */
void output_plaintext_write_complete_piece(FILE *file,
                                           Flags spec,
                                           piece_walk_type piece,
                                           square on);

/* Find the piece removal effect of a capturing move
 * @return the effect's id; move_effect_journal_index_null if there isn't any
 */
move_effect_journal_index_type output_plaintext_find_piece_removal(output_plaintext_move_context_type const *context,
                                                                   move_effect_journal_index_type curr,
                                                                   PieceIdType id_added);

/* Write an effect of the current move
 */
void output_plaintext_write_singlebox_type3_promotion(FILE *file);
void output_plaintext_write_flags_change(output_plaintext_move_context_type *context,
                                         move_effect_journal_index_type curr);
void output_plaintext_write_side_change(output_plaintext_move_context_type *context,
                                        move_effect_journal_index_type curr);
void output_plaintext_write_piece_change(output_plaintext_move_context_type *context,
                                         move_effect_journal_index_type curr);
void output_plaintext_write_piece_movement(output_plaintext_move_context_type *context,
                                           move_effect_journal_index_type curr);
void output_plaintext_write_piece_creation(output_plaintext_move_context_type *context,
                                           move_effect_journal_index_type curr);
void output_plaintext_write_piece_readdition(output_plaintext_move_context_type *context,
                                             move_effect_journal_index_type curr);
void output_plaintext_write_piece_removal(output_plaintext_move_context_type *context,
                                          move_effect_journal_index_type curr);
void output_plaintext_write_piece_exchange(output_plaintext_move_context_type *context,
                                           move_effect_journal_index_type curr);
void output_plaintext_write_half_neutral_deneutralisation(output_plaintext_move_context_type *context,
                                                          move_effect_journal_index_type curr);
void output_plaintext_write_half_neutral_neutralisation(output_plaintext_move_context_type *context,
                                                        move_effect_journal_index_type curr);
void output_plaintext_write_imitator_addition(output_plaintext_move_context_type *context);
void output_plaintext_write_imitator_movement(output_plaintext_move_context_type *context,
                                              move_effect_journal_index_type curr);
void output_plaintext_write_bgl_status(output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type curr);

/* Write the "regular part" of a current move which is capturing, i.e.
 * walk from*to e.p.
 */
void output_plaintext_write_capture(FILE *file,
                                    output_plaintext_move_context_type *context,
                                    move_effect_journal_index_type capture,
                                    move_effect_journal_index_type movement);

/* Write the "regular part" of the current move which does not capture, i.e.
 * - walk from-to
 */
void output_plaintext_write_no_capture(FILE *file,
                                       output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type movement);

/* Write the "regular part" of the current move which is a castling, i.e.
 * - 00
 * - 000
 */
void output_plaintext_write_castling(FILE *file,
                                     move_effect_journal_index_type movement);

/* Write the "regular part" of the current move, i.e. dispatch to the
 * above mentioned functions
 */
void output_plaintext_write_regular_move(FILE *file,
                                         output_plaintext_move_context_type *context);

/* Write the current move
 */
void output_plaintext_write_move(FILE *file);

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
