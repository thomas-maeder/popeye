#if !defined(CONDITIONS_CHECK_ZIGZAG_H)
#define CONDITIONS_CHECK_ZIGZAG_H

#include "stipulation/stipulation.h"

/* Instrument a battle branch
 * @param adapter identifies adapter slice into the battle branch
 */
void battle_branch_insert_defense_check_zigzag(slice_index adapter);

/* Instrument a help branch
 * @param adapter identifies adapter slice into the help branch
 */
void help_branch_insert_check_zigzag(slice_index adapter);

#endif
