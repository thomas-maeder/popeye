#if !defined(STIPULATION_BATTLE_PLAY_CONTINUATION_H)
#define STIPULATION_BATTLE_PLAY_CONTINUATION_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with continuations.
 */

/* Solve postkey play play after the move that has just been played in
 * the current ply 
 * @param si slice index
 * @return true iff >=1 variation or a threat was found
 */
boolean continuation_writer_solve_postkey(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean continuation_writer_root_defend(slice_index si);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean continuation_writer_defend_in_n(slice_index si, stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int continuation_writer_can_defend_in_n(slice_index si,
                                                 stip_length_type n,
                                                 unsigned int max_result);

/* Instrument the stipulation representation so that it can deal with
 * continuations
 */
void stip_insert_continuation_handlers(void);

#endif
