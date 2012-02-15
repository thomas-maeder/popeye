#if !defined(PYSTIP_H)
#define PYSTIP_H

#include <stddef.h>

#include "stipulation/goals/goals.h"
#include "py.h"
#include "boolean.h"

/* This module provides generic declarations and functionality about
 * stipulations */

#include "stipulation/slice_type.h"
#define ENUMERATION_DECLARE
#include "pyenum.h"

typedef enum
{
  goal_applies_to_starter,
  goal_applies_to_adversary
} goal_applies_to_starter_or_adversary;

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_DECLARE

#include "pyenum.h"

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


#define ENUMERATION_TYPENAME branch_level
#define ENUMERATORS \
  ENUMERATOR(toplevel_branch),                  \
    ENUMERATOR(nested_branch)

#define ENUMERATION_DECLARE

#include "pyenum.h"


/* slice identification */
enum
{
  max_nr_slices = 3000,
  no_slice = max_nr_slices
};

extern Slice slices[max_nr_slices];

/* The length field of series and help branch slices thus gives the
 * number of half moves of the *human-readable* stipulation.
 *
 * This means that the recursion depth of solving the branch slice
 * never reaches the value of length. At (maximal) recursion depth
 * length-2 (help play) resp. length-1 (series play), solving the
 * next slice is started.
 *
 * The following symbols represent the difference of the length and
 * the maximal recursion level:
 */
enum
{
  slack_length = 3    /* half moves */
};


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
  ENUMERATOR(slice_structure_leaf),                              \
    ENUMERATOR(slice_structure_binary),                          \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_branch),                          \
    ENUMERATOR(slice_structure_fork),                            \
    ENUMERATOR(nr_slice_structure_types)

#define ENUMERATION_DECLARE

#include "pyenum.h"


#define ENUMERATION_TYPENAME slice_functional_type
#define ENUMERATORS                                              \
  ENUMERATOR(slice_function_unspecified),                        \
  ENUMERATOR(slice_function_proxy),                              \
  ENUMERATOR(slice_function_move_generator),                     \
  ENUMERATOR(slice_function_testing_pipe),                       \
  ENUMERATOR(slice_function_conditional_pipe),                   \
  ENUMERATOR(slice_function_writer),                             \
  ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Retrieve the structural type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_structural_type slice_get_structural_type(slice_index si);


/* Retrieve the functional type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_functional_type slice_get_functional_type(slice_index si);

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_allocator(void);

/* Allocate a slice index
 * @param type which slice type
 * @return a so far unused slice index
 */
slice_index alloc_slice(slice_type type);

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si);

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si);

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original);

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred);

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si);

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void);

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si);

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal.
 * @param si identifies slice where to start
 * @return true iff quodlibet could be applied
 */
boolean transform_to_quodlibet(slice_index si);

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

struct stip_structure_traversal;

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

/* Type of callback for stipulation traversals
 */
typedef void (*stip_structure_visitor)(slice_index si,
                                       struct stip_structure_traversal *st);

/* Mapping of slice types to structure visitors.
 */
typedef struct
{
    stip_structure_visitor visitors[nr_slice_types];
} structure_visitor_map_type;

typedef enum
{
  slice_not_traversed,
  slice_being_traversed, /* used for avoiding infinite recursion */
  slice_traversed
} stip_structure_traversal_state;

typedef enum
{
  structure_traversal_level_root,
  structure_traversal_level_setplay,
  structure_traversal_level_nested
} structure_traversal_level_type;

typedef enum
{
  stip_traversal_context_global,
  stip_traversal_context_attack,
  stip_traversal_context_defense,
  stip_traversal_context_help
} stip_traversal_context_type;

typedef struct stip_structure_traversal
{
    structure_visitor_map_type map;
    stip_structure_traversal_state traversed[max_nr_slices];
    structure_traversal_level_type level;
    stip_traversal_context_type context;
    void *param;
} stip_structure_traversal;

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st, void *param);

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor (note: subclasses of type
 *             are not affected by
 *             stip_structure_traversal_override_by_structure()! )
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_structure(stip_structure_traversal *st,
                                                    slice_structural_type type,
                                                    stip_structure_visitor visitor);

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_function(stip_structure_traversal *st,
                                                   slice_functional_type type,
                                                   stip_structure_visitor visitor);

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_single(stip_structure_traversal *st,
                                              slice_type type,
                                              stip_structure_visitor visitor);

/* define an alternative visitor for a particular slice type */
typedef struct
{
    slice_type type;
    stip_structure_visitor visitor;
} structure_traversers_visitors;

/* Override some of the visitors of a traversal
 * @param st to be initialised
 * @param visitors overriding visitors
 * @param nr_visitors number of visitors
 */
void
stip_structure_traversal_override(stip_structure_traversal *st,
                                  structure_traversers_visitors const visitors[],
                                  unsigned int nr_visitors);

/* Query the structure traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st);

/* Structure visitor doing nothing. Makes it easier to initialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st);

/* (Approximately) depth-first traversal of the children of a slice
 * @param si slice whose children to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st);

/* (Approximately) depth-first traversal of a stipulation sub-tree
 * @param root entry slice into stipulation
 * @param st address of structure defining traversal
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st);


struct stip_moves_traversal;

/* Type of visitors for stipulation traversals
 */
typedef void (*stip_moves_visitor)(slice_index si,
                                   struct stip_moves_traversal *st);

/* Map a slice type to a visitor */
typedef struct
{
    stip_moves_visitor visitors[nr_slice_types];
} moves_visitor_map_type;

/* hold the state of a moves traversal */
typedef struct stip_moves_traversal
{
    moves_visitor_map_type map;
    stip_traversal_context_type context;
    stip_length_type full_length;
    stip_length_type remaining;
    void *param;
} stip_moves_traversal;

enum
{
  STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED = INT_MAX
};

/* define an alternative visitor for a particular slice type */
typedef struct
{
    slice_type type;
    stip_moves_visitor visitor;
} moves_traversers_visitors;

/* Initialise a move traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st, void *param);

/* Set the number of moves at the start of the traversal. Normally, this is
 * determined while traversing the stipulation. Only invoke
 * stip_moves_traversal_set_remaining() when the traversal is started in the
 * middle of a stipulation.
 * @param st to be initialised
 * @param remaining number of remaining moves (without slack)
 * @param full_length full number of moves of the initial branch (without slack)
 */
void stip_moves_traversal_set_remaining(stip_moves_traversal *st,
                                        stip_length_type remaining,
                                        stip_length_type full_length);

/* Override the behavior of a moves traversal at some slice types
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 */
void stip_moves_traversal_override(stip_moves_traversal *st,
                                   moves_traversers_visitors const visitors[],
                                   unsigned int nr_visitors);

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_structure(stip_moves_traversal *st,
                                                slice_structural_type type,
                                                stip_moves_visitor visitor);

/* Override the behavior of a moves traversal at slices of a functional type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_function(stip_moves_traversal *st,
                                               slice_functional_type,
                                               stip_moves_visitor visitor);

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          slice_type type,
                                          stip_moves_visitor visitor);

/* Traversal of moves of the stipulation
 * @param root identifies start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st);

/* Traversal of the moves beyond some root slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_root(slice_index si, stip_moves_traversal *st);

/* No-op callback for move traversals
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_noop(slice_index si, stip_moves_traversal *st);

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st);

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
