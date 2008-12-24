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

/* Increase the indentation of output in tree mode
 */
void output_indent(void);

/* Decrease the indentation of output in tree mode
 */
void output_outdent(void);

/* Write the appropriate amount of whitespace for the following output
 * to be correctely indented.
 */
void write_indentation(void);

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

typedef enum
{
  attack_with_zugzwang,
  attack_without_zugzwang
} attack_conclusion_type;

/* Conclude writing an attacker's move; each call to write_attack()
 * should be followed by a call to write_attack_conclusion(),
 * indicating whether we want to signal zugzwang in the ouptut.
 * @param type should we signal zugzwang in the output?
 */
void write_attack_conclusion(attack_conclusion_type type);

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
