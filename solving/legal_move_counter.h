#if !defined(SOLVING_LEGAL_MOVE_COUNTER_H)
#define SOLVING_LEGAL_MOVE_COUNTER_H

#include "solving/solve.h"
#include "solving/ply.h"

/* This module provides functionality dealing with the attacking side
 * in STLegalAttackCounter stipulation slices.
 */

/* current value of the count */
extern unsigned int legal_move_counter_count[maxply];

/* stop the move iteration once legal_move_counter_count exceeds this number */
extern unsigned int legal_move_counter_interesting[maxply];

/* Allocate a STLegalAttackCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_attack_counter_slice(void);

/* Allocate a STLegalDefenseCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_defense_counter_slice(void);

/* Allocate a STAnyMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_any_move_counter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type legal_attack_counter_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type legal_defense_counter_solve(slice_index si, stip_length_type n);

#endif
