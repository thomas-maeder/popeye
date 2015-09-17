#if !defined(STIPULATION_STIPULATION_H)
#define STIPULATION_STIPULATION_H

#include "stipulation/goals/goals.h"
#include "stipulation/slice_type.h"
#include "position/side.h"
#include "pieces/pieces.h"
#include "output/mode.h"
#include "utilities/boolean.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>

/* This module provides generic declarations and functionality about
 * stipulations */

typedef enum
{
  goal_applies_to_starter,
  goal_applies_to_adversary
} goal_applies_to_starter_or_adversary;

typedef unsigned int slice_index;

typedef unsigned int stip_length_type;

enum
{
  no_stip_length = INT_MAX
};

struct circe_variant_type;
struct position;

typedef enum
{
  twin_zeroposition,
  twin_regular /* add 0 for a), 1 for b) etc. */
} twinning_event_type;

typedef struct
{
    slice_type type;
    Side starter;
    slice_index prev;
    slice_index next1;
    slice_index next2;
    slice_index tester;

    union
    {
        struct
        {
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
        } branch;

        struct /* for goal filter types * */
        {
            goal_applies_to_starter_or_adversary applies_to_who;
        } goal_filter;

        struct
        {
            stip_length_type threshold; /* without slack */
        } fork_on_remaining;

        struct
        {
            unsigned int max_nr_refutations;
        } refutation_collector;

        struct /* for type==STKeepMatingGuard */
        {
            Side mating;
        } keepmating_guard;

        struct /* for slices dealing with a single goal */
        {
            Goal goal;
        } goal_handler;

        struct
        {
            output_mode mode;
        } output_mode_selector;

        struct
        {
            slice_index base;
        } derived_pipe;

        struct
        {
            slice_index condition;
        } if_then_else;

        struct
        {
          struct circe_variant_type const * variant;
        } circe_handler;

        struct
        {
            FILE *file;
        } writer;

        struct
        {
          struct position const *position;
        } position_handler;

        struct
        {
            Goal goal;
            FILE *file;
        } goal_writer;

        struct
        {
            char const *name;
        } input_opener;

        struct
        {
            int argc;
            char **argv;
        } command_line_options_parser;

        struct
        {
            boolean value;
        } flag_handler;

        struct
        {
            unsigned int twin_number; /* 0==a), 1==b) ... */
            boolean continued;
        } twinning_handler;

        struct
        {
            void (*handler)(slice_index si,
                            twinning_event_type event,
                            boolean continued);
        } twinning_event_handler;
    } u;
} Slice;

/* slice identification */
enum
{
  max_nr_slices = 16000,
  no_slice = max_nr_slices
};

extern Slice slices[max_nr_slices];

/* by consequently these macros, client code doesn't depend on the actual
 * slice implementation */
#define SLICE(si) slices[(si)]
#define SLICE_TYPE(si) slices[(si)].type
#define SLICE_PREV(si) slices[(si)].prev
#define SLICE_NEXT1(si) slices[(si)].next1
#define SLICE_NEXT2(si) slices[(si)].next2
#define SLICE_STARTER(si) slices[(si)].starter
#define SLICE_TESTER(si) slices[(si)].tester
#define SLICE_U(si) slices[(si)].u

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si);

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void);

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
void solving_impose_starter(slice_index si, Side starter);

struct stip_structure_traversal;

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
 * @param st_parent parent traversal (0 if there is none)
 * @param copies address of an array mapping indices of originals
 *        to indices of copies
 * @note initialises all elements of *copies to no_slice
 * @note after this initialisation, *st can be used for a deep copy operation;
 *       or st can be further modified for some special copy operation
 */
void init_deep_copy(struct stip_structure_traversal *st,
                    struct stip_structure_traversal *st_parent,
                    stip_deep_copies_type *copies);

/* structure holding state in traversals for
 * solving_insert_root_slices()
 * stip_insert_intro_slices()
 * solving_apply_setplay()
 */
typedef struct
{
    slice_index spun_off[max_nr_slices];
} spin_off_state_type;

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void solving_insert_root_slices(slice_index si);

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void solving_insert_intro_slices(slice_index si);

/* Attempt to add set play to the stipulation
 * @param si identifies the root from which to apply set play
 * @return true iff set play could be added
 */
boolean solving_apply_setplay(slice_index si);

/* Are piece ids required for solving the current stipulation?
 */
boolean stipulation_are_pieceids_required(void);

/* Remember that piece ids are required for solving the current stipulation
 */
void stipulation_remember_pieceids_required(void);

/* Reset the current stipulation before constructing a new one
 */
void stipulation_reset(void);

#endif
