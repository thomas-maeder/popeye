#if !defined(STIPULATION_MOVE_INVERTER_H)
#define STIPULATION_MOVE_INVERTER_H

#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with move inverter slices
 */

/* Allocate a STMoveInverter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_inverter_attack(slice_index si, stip_length_type n);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_inverter_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
