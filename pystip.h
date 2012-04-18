#if !defined(PYSTIP_H)
#define PYSTIP_H

#include "py.h"
#include "stipulation/slice.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/moves_traversal.h"
#include "stipulation/goals/goals.h"
#include "output/mode.h"
#include "utilities/boolean.h"

#include <stddef.h>

/* This module provides generic declarations and functionality about
 * stipulations */

typedef enum
{
  goal_applies_to_starter,
  goal_applies_to_adversary
} goal_applies_to_starter_or_adversary;

typedef struct
{
    slice_type type;
    Side starter;
    slice_index prev;

    union
    {
        struct /* for types with 1 principal subsequent slice */
        {
            slice_index next;
        } pipe;

        struct
        {
            slice_index next;
            slice_index fork;
        } fork;

        struct /* for type==STOr and type==STAnd */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } binary;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
        } branch;

        struct
        {
            slice_index next;
            move_generation_mode_type mode;
        } move_generator;

        struct
        {
            slice_index next;
            move_generation_mode_type mode;
            Goal goal;
        } goal_reaching_move_generator;

        struct /* for type==STGoalTargetReachedTester */
        {
            slice_index next;
            slice_index tester;
            square target;
        } goal_target_reached_tester;

        struct /* for goal filter types * */
        {
            slice_index next;
            slice_index tester;
            goal_applies_to_starter_or_adversary applies_to_who;
        } goal_filter;

        struct
        {
            slice_index op1;
            slice_index op2;
            stip_length_type threshold; /* without slack */
        } fork_on_remaining;

        struct
        {
            slice_index next;
            unsigned int max_nr_refutations;
        } refutation_collector;

        struct /* for type==STKeepMatingGuard */
        {
            slice_index next;
            Side mating;
        } keepmating_guard;

        struct /* for slices dealing with a single goal */
        {
            slice_index next;
            slice_index tester;
            Goal goal;
        } goal_handler;

        struct
        {
            slice_index next;
            output_mode mode;
        } output_mode_selector;

        struct
        {
            slice_index next;
            slice_index base;
        } derived_pipe;
    } u;
} Slice;


extern Slice slices[max_nr_slices];

/* Create a slice
 * @param type which type
 * @return index of created slice
 */
slice_index create_slice(slice_type type);

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original);

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si);

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred);

/* Does the current stipulation end in a specific goal?
 * @param si identifies slice where to start
 * @param goal identifies the goal
 * @return true iff one of the goals of the stipulation is goal
 */
boolean stip_ends_in(slice_index si, goal_type goal);

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return Goal with no_goal if goal is not unique;
 *         the unique goal otherwise
 */
Goal find_unique_goal(slice_index si);

typedef enum
{
  dont_know_who_decides_on_starter,
  leaf_decides_on_starter
} who_decides_on_starter;

/* Detect the starting side from the stipulation
 * @param si identifies slice whose starter to find
 */
void stip_detect_starter(slice_index si);

/* Impose the starting side on the stipulation
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 */
void stip_impose_starter(slice_index si, Side starter);

/* Set the starting side of the stipulation from within an ongoing iteration
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 * @param st address of structure representing ongoing iteration
 */
void stip_impose_starter_nested(slice_index si,
                                Side starter,
                                struct stip_structure_traversal *st);

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si);

/* Auxiliary data structure for deep_copy: remembers slice copies already made
 */
typedef slice_index stip_deep_copies_type[max_nr_slices];

/* Initialise a structure traversal for a deep copy operation
 * @param st address of the structure to be initialised
 * @param copies address of an array mapping indices of originals
 *        to indices of copies
 * @note initialises all elements of *copies to no_slice
 * @note after this initialisation, *st can be used for a deep copy operation;
 *       or *st can be further modified for some special copy operation
 */
void init_deep_copy(stip_structure_traversal *st,
                    stip_deep_copies_type *copies);

/* structure holding state in traversals for
 * stip_insert_root_slices()
 * stip_insert_intro_slices()
 * stip_apply_setplay()
 */
typedef struct
{
    slice_index spun_off[max_nr_slices];
} spin_off_state_type;

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void stip_insert_root_slices(slice_index si);

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void stip_insert_intro_slices(slice_index si);

/* Attempt to add set play to the stipulation
 * @param si identifies the root from which to apply set play
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(slice_index si);

#endif
