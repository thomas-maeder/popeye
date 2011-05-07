#if !defined(PYDIRCTG_H)
#define PYDIRCTG_H

/* Deals with direct branches
 */

#include "stipulation/battle_play/attack_play.h"

/* Instrument a branch leading to a goal to be a direct goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_direct_goal_branch(slice_index si);

#endif
