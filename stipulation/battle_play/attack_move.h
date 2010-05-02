#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_MOVE_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_MOVE_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

/* This module provides functionality dealing with the attacking side
 * in STAttackMove stipulation slices.
 */

/* Allocate a STAttackMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_move_slice(stip_length_type length,
                                    stip_length_type min_length);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void attack_move_insert_root(slice_index si, stip_structure_traversal *st);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean attack_move_are_threats_refuted_in_n(table threats,
                                             stip_length_type len_threat,
                                             slice_index si,
                                             stip_length_type n);

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_has_solution_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_min);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type attack_move_solve_threats_in_n(table threats,
                                                slice_index si,
                                                stip_length_type n,
                                                stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void attack_move_detect_starter(slice_index si, stip_structure_traversal *st);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void attack_move_apply_setplay(slice_index si, stip_structure_traversal *st);

#endif
