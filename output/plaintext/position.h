#if !defined(OUTPUT_PLAINTEXT_POSITION_H)
#define OUTPUT_PLAINTEXT_POSITION_H

#include "position/position.h"
#include "stipulation/stipulation.h"

#include <stdio.h>

void WriteBoard(position const *pos);

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
void output_plaintext_write_meta(slice_index si);
void output_plaintext_write_board(slice_index si);
void output_plaintext_write_piece_counts(slice_index si);
void output_plaintext_write_atob_intra(slice_index si);

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
void output_plaintext_write_stipulation(slice_index si);
void output_plaintext_write_stipulation_options(slice_index si);
void output_plaintext_write_royal_piece_positions(slice_index si);
void output_plaintext_write_non_royal_attributes(slice_index si);
void output_plaintext_write_conditions(slice_index si);
void output_plaintext_write_mutually_exclusive_castlings(slice_index si);
void output_plaintext_write_duplex(slice_index si);
void output_plaintext_write_quodlibet(slice_index si);
void output_plaintext_write_grid(slice_index si);

void output_plaintext_position_writer_builder_solve(slice_index si);
void output_plaintext_proof_position_writer_builder_solve(slice_index si);
void output_plaintext_atob_position_writer_builder_solve(slice_index si);

#endif
