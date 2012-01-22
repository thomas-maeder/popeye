#if !defined(OPTIONS_MAXTIME_H)
#define OPTIONS_MAXTIME_H

/* This module provides functionality dealing with the filter slices
 * implement the maxtime option and command line parameter.
 * Slices of this type make sure that solving stops after the maximum
 * time has elapsed
 */

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxtime_guard_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxtime_guard_can_defend(slice_index si, stip_length_type n);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type maxtime_guard_help(slice_index si, stip_length_type n);

/* Instrument a stipulation with STMaxTimeGuard slices
 * @param si identifies slice where to start
 */
void stip_insert_maxtime_guards(slice_index si);

#endif
