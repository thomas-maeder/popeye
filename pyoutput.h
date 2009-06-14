#if !defined(PYOUTPUT_H)
#define PYOUTPUT_H

#include "py.h"
#include "pygoal.h"
#include "pystip.h"
#include "pytable.h"

/* Contains the stipulation slice that was active when the move at a
 * specific ply was played.
 */
extern slice_index active_slice[maxply];


/* Write the appropriate amount of whitespace for the following output
 * to be correctely indented.
 */
void write_indentation(void);


/* Start a new output level with inverted side at move
 */
void output_start_move_inverted_level(void);

/* End the inner-most output level inverted side at move
 */
void output_end_move_inverted_level(void);

/* End half duplex
 */
void output_end_half_duplex(void);

/* Start a new output level consisting of post-key play
 */
void output_start_postkey_level(void);

/* End the inner-most output level (which consists of post-key play)
 */
void output_end_postkey_level(void);


/* Start a new output level consisting of threats
 */
void output_start_threat_level(void);

/* End the inner-most output level (which consists of threats)
 */
void output_end_threat_level(void);


/* Start a new output level consisting of regular continuations
 */
void output_start_continuation_level(void);

/* End the inner-most output level (which consists of regular
 * continuations)
 */
void output_end_continuation_level(void);


/* Start a new output level consisting of forced unsolvabilities
 * (e.g. reflex mates)
 */
void output_start_unsolvability_level(void);

/* End the inner-most output level (which consists of forced
 * unsolvabilities (e.g. reflex mates))
 */
void output_end_unsolvability_level(void);


/* Start a new output level consisting of leaf variations
 */
void output_start_leaf_variation_level(void);

/* End the inner-most output level (which consists of leaf variations)
 */
void output_end_leaf_variation_level(void);


/* Initialize based on the stipulation
 * @param si index of root of subtree that we initialise output for
 */
void init_output(slice_index si);

/* Write a move of the attacking side in direct play
 * @param type of attack
 */
void write_attack(attack_type type);

/* Write a final move of the attacking side in direct play
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 * @param type of attack
 */
void write_final_attack(Goal goal, attack_type type);

/* Write a defender's move that does not reach a goal
 */
void write_defense(void);

/* Write a defender's final move
 * @param goal goal reached by the move (!=no_goal)
 */
void write_final_defense(Goal goal);

/* Write the final move in a help leaf
 * @param goal goal reached by the move (!=no_goal)
 */
void write_final_help_move(Goal goal);

/* Mark the defense about to be written as refutation
 */
void write_refutation_mark(void);

/* Write the refutations stored in a table
 * @param t table containing refutations
 */
void write_refutations(table t);

/* Write the end of a solution
 */
void write_end_of_solution(void);

/* Write the end of a solution phase
 */
void write_end_of_solution_phase(void);


#endif
