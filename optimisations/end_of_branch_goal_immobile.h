#if !defined(OPTIMISATIONS_END_OF_BRANCH_GOAL_IMMOBILE_H)
#define OPTIMISATIONS_END_OF_BRANCH_GOAL_IMMOBILE_H

#include "stipulation/help_play/play.h"

/* This module provides an optimisation for goals that involve the immobility
 * of the side at the move
 */

/* Optimise STEndOfBranchGoal slices
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_with_end_of_branch_goal_immobile(slice_index si);

#endif
