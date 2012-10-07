#if !defined(SOLVING_FIND_MOVE_H)
#define SOLVING_FIND_MOVE_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STFindAttack stipulation slices.
 */

/* Allocate a STFindAttack slice.
 * @return index of allocated slice
 */
slice_index alloc_find_attack_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_attack_solve(slice_index si, stip_length_type n);

/* Allocate a STFindDefense slice.
 * @return index of allocated slice
 */
slice_index alloc_find_defense_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_defense_solve(slice_index si, stip_length_type n);

#endif
