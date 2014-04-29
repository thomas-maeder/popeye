#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H

#include "position/side.h"
#include "stipulation/stipulation.h"

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

/* Determine whether the optimisation is enabled for a side
 * @param side
 * @return true iff the optimisation is enabled for side
 */
boolean is_countnropponentmoves_defense_move_optimisation_enabled(Side side);

/* Instrument stipulation with optimised move generation based on the number of
 * opponent moves
 * @param si identifies slice where to start
 */
void solving_optimise_with_countnropponentmoves(slice_index si);

#endif
