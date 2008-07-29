#if !defined(PYLEAF_H)
#define PYLEAF_H

#include "boolean.h"
#include "pystip.h"
#include "pycompos.h"
#include "py.h"
#include "pyhash.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param just_moved side that has just moved
 * @param leaf slice index of leaf slice
 * @return true iff just_moved has reached leaf's goal
 */
boolean leaf_is_goal_reached(couleur just_moved, slice_index leaf);

/* Determine whether attacker has an end in 1.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
boolean leaf_is_end_in_1_possible(couleur side_at_move, slice_index leaf);

/* Detect a priori unsolvability of a leaf in direct play
 * (e.g. because of forced reflex mates)
 * @param attacker attacking side
 * @param leaf leaf's slice index
 * @return true iff leaf is unsolvable
 */
boolean d_leaf_is_unsolvable(couleur attacker, slice_index leaf);

/* Determine whether the defender has directly lost with his move just
 * played. 
 * Assumes that there is no short win for the defending side.
 * @param attacker attacking side
 * @param si slice identifier
 * @return true iff the defending side has directly lost
 */
boolean d_leaf_has_defender_lost(couleur attacker, slice_index leaf);

/* Write a priori unsolvability (if any) of a leaf in direct play
 * (e.g. forced reflex mates).
 * Assumes d_leaf_is_unsolvable()
 * @param attacker attacking side
 * @param leaf leaf's slice index
 */
void d_leaf_write_unsolvability(couleur attacker, slice_index leaf);

/* Find and write the solution(s) of a leaf.
 * Assumes that unsolvability (e.g. because of a forced reflex move)
 * has already been dealt with.
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index
 * @param solutions table where to add solutions
 * @return true iff >=1 solution was found
 */
boolean d_leaf_solve(couleur attacker,
                     boolean restartenabled,
                     slice_index leaf,
                     int solutions);

/* Find and write defender's set play
 * @param defender defending side
 * @param leaf slice index
 */
void d_leaf_solve_setplay(couleur defender, slice_index leaf);

/* Find and write defender's set play in direct play if every set move
 * leads to end
 * @param defender defending side
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean d_leaf_solve_complete_set(couleur defender, slice_index leaf);

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param defender attacking side
 * @param leaf slice index
 */
void d_leaf_solve_variations(couleur defender, slice_index leaf);

/* Write the key just played, solve the post key play (threats,
 * variations) and write the refutations (if any)
 * @param attacker attacking side (has just played key)
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 * @param is_try true iff what we are solving is a try
 */
void d_leaf_write_key_solve_postkey(couleur attacker,
                                    int refutations,
                                    slice_index leaf,
                                    boolean is_try);

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param attacker attacking side
 * @param continuations table where to append continuations found and
 *                      written
 * @param leaf slice index
 */
void d_leaf_solve_continuations(couleur attacker,
                                int continuations,
                                slice_index leaf);

/* Determine whether the defending side (at the move) wins
 * @param defender defending side
 * @param leaf slice identifier
 * @return "how much or few" the defending side wins
 */
d_composite_win_type d_leaf_does_defender_win(couleur defender,
                                              slice_index leaf);

/* Determine whether the attacking side has immediately lost with its
 * move just played.
 * @param defender defending side
 * @param leaf slice identifier
 * @return true iff attacker has lost
 */
boolean d_leaf_has_attacker_lost(couleur defender, slice_index leaf);

/* Determine whether the attacking side has immediately won with its
 * move just played.
 * @param defender defending side
 * @param leaf slice identifier
 * @return true iff attacker has lost
 */
boolean d_leaf_has_attacker_won(couleur defender, slice_index leaf);

/* Determine whether the attacker wins in a direct/self/reflex
 * stipulation in 1. 
 * @param attacker attacking side (at move)
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
boolean d_leaf_does_attacker_win(couleur attacker, slice_index leaf);

/* Determine and find final moves in a help stipulation
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true if >=1 ending move was found
 */
boolean h_leaf_h_solve_ending_move(couleur side_at_move, slice_index leaf);

/* Determine and write the solution of a leaf slice in help play.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean h_leaf_solve(couleur side_at_move,
                     hashwhat no_succ_hash_category,
                     boolean restartenabled,
                     slice_index leaf);

/* Solve the set play in a help stipulation
 * @param side_at_move side at move (going to play set move)
 * @param leaf slice index
 * @return true iff >= 1 set play was found
 */
boolean h_leaf_solve_setplay(couleur side_at_move, slice_index leaf);

/* Determine and write the solution of a leaf slice in series play.
 * @param series_side side doing the series
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean ser_leaf_solve(couleur series_side,
                       hashwhat no_succ_hash_category,
                       boolean restartenabled,
                       slice_index leaf);

/* Determine whether there is >= 1 solution for the leaf
 * @param side_at_move side at the move
 * @param leaf slice index of leaf slice
 * @return true iff side_at_move has >=1 solution
 */
boolean leaf_is_solvable(couleur side_at_move, slice_index leaf);

/* Solve a slice (i.e. find and write all solutions)
 * @param side_at_move side at the move
 * @param leaf slice index of leaf slice
 * @return true iff side_at_move has >=1 solution
 */
boolean leaf_solve(couleur side_at_move, slice_index leaf);

#endif
