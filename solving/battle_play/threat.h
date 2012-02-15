#if !defined(SOLVING_BATTLE_PLAY_THREAT_H)
#define SOLVING_BATTLE_PLAY_THREAT_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with threats
 */

/* Represent the current threat related activity in a ply
 */
typedef enum
{
  threat_idle,
  threat_solving,
  threat_enforcing
} threat_activity;

/* Exposed for read access only */
extern threat_activity threat_activities[maxply+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_attack(slice_index si, stip_length_type n);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a threat enforcer slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_threat_enforcer(slice_index si,
                                           stip_structure_traversal *st);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type threat_collector_defend(slice_index si, stip_length_type n);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a threat collector slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_threat_collector(slice_index si,
                                            stip_structure_traversal *st);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type threat_defeated_tester_defend(slice_index si,
                                               stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
stip_length_type threat_solver_defend(slice_index si, stip_length_type n);

/* Instrument the stipulation representation so that it can deal with
 * threats
 * @param si identifies slice where to start
 */
void stip_insert_threat_solvers(slice_index si);

#endif
