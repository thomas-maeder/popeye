#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H

#include "pystip.h"

/* This module provides functionality dealing with the defending side in
 * STMoveGenerator stipulation slices.
 * These slices provide unoptimised move generation
 */

/* Reset the enabled state
 */
void reset_countnropponentmoves_defense_move_optimisation(void);

/* Disable the optimisation for defenses by a side
 * @param side side for which to disable the optimisation
 */
void disable_countnropponentmoves_defense_move_optimisation(Side side);

/* Instrument stipulation with optimised move generation based on the number of
 * opponent moves
 * @param si identifies slice where to start
 */
void stip_optimise_with_countnropponentmoves(slice_index si);

#endif
