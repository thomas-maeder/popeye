#if !defined(PYDIRCTG_H)
#define PYDIRCTG_H

/* Deals with direct branches
 */

#include "stipulation/battle_play/attack_play.h"

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index proxy_to_goal);

/* Instrument a branch leading to a goal to be a direct goal branch
 * @param si identifies entry slice of branch
 */
void slice_make_direct_goal_branch(slice_index si);

#endif
