#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_REPLAY_FLESHED_OUT_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_REPLAY_FLESHED_OUT_H

#include "pieces/attributes/total_invisible.h"
#include "solving/ply.h"

extern ply ply_replayed;

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
void total_invisible_move_repeater_solve(slice_index si);

/* Undo all moves, then replay the fleshed out move sequence to find out if it
 * is replayable at all given the way we have fleshed out (eg have we made a
 * castling impossible by guarding the intermediate square?) and if it reaches
 * the goal
 * @return false iff undoing failed (and therefore nothing was replayed)
 * @note primarily communicates the result via mate_validation_result
 */
boolean replay_fleshed_out_move_sequence(play_phase_type phase_replay);

#endif
