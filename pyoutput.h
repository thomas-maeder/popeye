#if !defined(PYOUTPUT_H)
#define PYOUTPUT_H

#include "py.h"
#include "pygoal.h"
#include "pystip.h"

typedef enum
{
  output_mode_tree, /* typical in direct/self/reflex play */
  output_mode_line, /* typical in help/series play */

  output_mode_none
} output_mode;

/* Contains the stipulation slice that was active when the move at a
 * specific ply was played.
 */
extern slice_index active_slice[maxply];


/* Write the appropriate amount of whitespace for the following output
 * to be correctely indented.
 */
void write_indentation(void);


/* Start a new output level consisting of post-key play
 */
void output_start_postkeyonly_level(void);

/* End the inner-most output level (which consists of post-key play)
 */
void output_end_postkeyonly_level(void);


/* Start a new output level consisting of set play
 */
void output_start_setplay_level(void);

/* End the inner-most output level (which consists of set play)
 */
void output_end_setplay_level(void);


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


/* Start a new output level consisting of leaf variations
 */
void output_start_leaf_variation_level(void);

/* End the inner-most output level (which consists of leaf variations)
 */
void output_end_leaf_variation_level(void);


/* Select the inital output mode
 * @param initial_mode initial output mode
 */
void init_output_mode(output_mode initial_mode);

/* Write a move of the attacking side in direct play
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 * @param type of attack
 */
void write_attack(Goal goal, attack_type type);

/* Write a defender's move
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 */
void write_defense(Goal goal);

/* Mark the defense about to be written as refutation
 */
void write_refutation_mark(void);

/* Write the refutations stored in a table
 * @param t table containing refutations
 */
void write_refutations(int t);

/* Write the end of a solution
 */
void write_end_of_solution(void);

#endif
