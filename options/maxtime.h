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
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type maxtime_guard_defend(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type maxtime_guard_help(slice_index si, stip_length_type n);

/* Instrument a stipulation with STMaxTimeGuard slices
 * @param si identifies slice where to start
 */
void stip_insert_maxtime_guards(slice_index si);

#endif
