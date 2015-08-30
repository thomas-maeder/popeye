#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "stipulation/stipulation.h"
#include "stipulation/goals/goals.h"
#include "solving/move_effect_journal.h"
#include "output/output.h"

extern output_engine_type const output_plaintext_engine;
extern output_symbol_table_type const output_plaintext_symbol_table;


/* Write the current move
 */
void output_plaintext_write_move(output_engine_type const *engine,
                                 FILE *file,
                                 output_symbol_table_type const *symbol_table);

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
void output_plaintext_write_proof_target_position(slice_index si);
void output_plaintext_write_atob_positions(slice_index si);

/* Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si);

/* Write an empty line at the end of the output for a twin
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
void output_plaintext_end_of_twin_writer_solve(slice_index si);

#endif
