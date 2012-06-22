#if !defined(STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H)
#define STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H

#include "pystip.h"
#include "solving/battle_play/attack_play.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Allocate a system of slices that tests whether the side to be immobilised has
 * been
 * @return index of entry slice
 */
slice_index alloc_goal_immobile_reached_tester_system(void);

/* Allocate a STGoalImmobileReachedTester slice.
 * @param starter_or_adversary is the starter immobilised or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary);

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_goal_immobile_tester(slice_index si,
                                         stip_structure_traversal *st);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_immobile_reached_tester_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type immobility_tester_attack(slice_index si, stip_length_type n);

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si);

#endif
