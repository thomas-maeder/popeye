#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_UNINTERCEPTABLE_CHECK_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_UNINTERCEPTABLE_CHECK_H

#include "position/position.h"
#include "stipulation/stipulation.h"
#include "solving/ply.h"
#include "pieces/attributes/total_invisible/decisions.h"

extern square check_by_uninterceptable_delivered_from;
extern ply check_by_uninterceptable_delivered_in_ply;
extern decision_level_type check_by_uninterceptable_delivered_in_level;

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
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si);

vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                              vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk);

vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked);

vec_index_type is_square_attacked_by_uninterceptable(Side side_under_attack, square sq_attacked);

#endif
