#if !defined(STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H)
#define STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H

#include "pyslice.h"

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
void impose_starter_immobility_tester(slice_index si,
                                      stip_structure_traversal *st);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_immobile_reached_tester_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_king_first_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_non_king_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ohneschach_immobility_tester_has_solution(slice_index si);

/* Generate moves for the king (if any) of a side
 * @param side side for which to generate king moves
 */
void generate_king_moves(Side side);

/* Find a legal move for a side. Start with the king moves that have already
 * been generated
 * @param side side for which to find a legal move
 * @return true iff a legal move has been found
 */
boolean find_any_legal_move_king_first(Side side);

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 * @param type substitute type
 */
void goal_immobile_reached_tester_replace(slice_index si, slice_type type);

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si);

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean ohneschach_immobile(Side isde);

#endif
