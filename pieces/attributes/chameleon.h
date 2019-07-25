#if !defined(PIECES_ATTRIBUTES_CHAMELEON_H)
#define PIECES_ATTRIBUTES_CHAMELEON_H

/* This module implements Chameleon pieces and Chameleon Chess */

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/twin.h"

typedef piece_walk_type chameleon_sequence_type[nr_piece_walks];

chameleon_sequence_type chameleon_walk_sequence;

extern twin_id_type explicit_chameleon_squence_set_in_twin;

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured walk
 * @param reborn type of reborn walk if a piece with walk captured is captured
 */
void chameleon_set_successor_walk_explicit(twin_id_type *is_explicit,
                                           chameleon_sequence_type* sequence,
                                           piece_walk_type from, piece_walk_type to);

/* Initialise the reborn pieces if they haven't been already initialised
 * from explicit indications.
 * @note chameleon_init_sequence_implicit() resets *is_explicit to false
 */
void chameleon_init_sequence_implicit(twin_id_type *is_explicit,
                                      chameleon_sequence_type* sequence);

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
void chameleon_change_promotee_into_solve(slice_index si);

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
void chameleon_arriving_adjuster_solve(slice_index si);

/* Instrument the solving machinery for solving problems with some
 * chameleon pieces
 * @param si identifies root slice of stipulation
 */
void chameleon_initialise_solving(slice_index si);

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
void chameleon_chess_arriving_adjuster_solve(slice_index si);

/* Instrument the solving machinery for solving problems with the condition
 * Chameleon Chess
 * @param si identifies root slice of stipulation
 */
void chameleon_chess_initialise_solving(slice_index si);

#endif
