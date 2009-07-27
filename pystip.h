#if !defined(PYSTIP_H)
#define PYSTIP_H

#include <stddef.h>

#include "pygoal.h"
#include "py.h"
#include "boolean.h"

/* This module provides generic declarations and functionality about
 * stipulations */

/* To deal with the complexity of stipulations used in chess problems,
 * their representation in Popeye splits them up into "slices". 9
 * different types of slices can be distinguished:
 */

typedef enum
{
  STBranchDirect, /* M-N moves of direct play */
  STBranchDirectDefender,
  STBranchHelp,   /* M-N moves of help play */
  STBranchSeries, /* M-N moves of series play */
  STBranchFork,    /* decides when play in branch is over */

  STLeafDirect,   /* goal in 1 */
  STLeafHelp,     /* help-goal in 1 */
  STLeafSelf,     /* self-goal in 1 */
  STLeafForced,   /* forced goal in 1 half move */

  STReciprocal,   /* logical AND */
  STQuodlibet,    /* logical OR */
  STNot,          /* logical NOT */

  STMoveInverter, /* 0 length, inverts side at move */

  STHelpRoot,     /* root level of help play */
  STHelpAdapter,  /* help play after branch fork */
  STHelpHashed,   /* help play with hash table */

  STReflexGuard,  /* stop attempts where wrong side can reach goal */

  nr_slice_types,
  no_slice_type = nr_slice_types
} SliceType;

/* The structure of a stipulation is similar to that of a tree
 * (admittedly a degenerate one in the case of simple stipulations
 * like 'direct mate in 3').
 *
 * Branch slices have variable length and lead to another slice, the
 * 'next' slice.
 * Leaf slices have fixed length and do not lead to other slices.
 * The other slices are logical operations on 1 or 2 operand slices.
 *
 * Real world stipulations are constructed by combining the slices of
 * the different types.
 *
 * Examples:
 *
 * #3:
 *     type           starter goal
 *     type           starter length  next
 * [0] STLeafDirect   White   goal_mate
 * [1] STBranchDirect White   2       0
 *
 * h=2.5:
 *     type           starter goal
 *     type           starter length  next
 * [0] STLeafHelp     White   goal_stale
 * [1] STBranchHelp   White   5       0
 *
 * s#=2:
 *     type           starter goal
 *     type           op1 op2
 * [0] STLeafSelf     White   goal_stale
 * [1] STLeafSelf     White   goal_mate
 * [2] STQuodlibet    0   0
 *
 * reci-h#3:
 *     type           starter goal
 *     type           op1 op2
 * [0] STLeafHelp     Black   goal_mate
 * [1] STLeafDirect   Black   goal_mate
 * [2] STReciprocal   0   1
 *
 * r#2:
 *     type           starter goal
 *     type           op1 op2
 *     type           starter length  next
 * [0] STLeafDirect   White   goal_mate
 * [1] STNot          0
 * [2] STLeafHelp     White   goal_mate
 * [3] STReciprocal   1   2
 * [4] STBranchDirect White   1       3
 *
 * 8->ser-=3:
 *     type           starter goal
 *     type           starter length  next
 * [0] STLeafDirect   White   goal_stale
 * [1] STBranchSeries White   3       0
 * [2] STBranchSeries Black   9       1
 *
 * In all the examples, the root slice is the one last mentioned.
 *
 * As for the length of branch slices, see below.
 */

typedef struct
{
    SliceType type;
    Side starter;

    union
    {
        struct /* for type==STLeaf* */
        {
            Goal goal;
            square target; /* for goal==goal_target */
        } leaf;

        struct /* for type==STLeafSelf */
        {
            Goal goal;
            square target; /* for goal==goal_target */
            slice_index next;
        } leafself;

        struct /* for types with 1 principal subsequent slice */
        {
            slice_index next;

            union
            {
                struct
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                } branch;

                struct
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index towards_goal;
                } branch_d_defender;

                struct
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index fork;
                } help_adapter;

                struct /* for type==STBranchFork */
                {
                    slice_index towards_goal;
                } branch_fork;

                struct /* for type==STReflexGuard */
                {
                    slice_index not_slice;
                } reflex_guard;
            } u;
        } pipe;

        struct /* for type==STQuodlibet and type==STReciprocal */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } fork;
    } u;
} Slice;


typedef enum
{
  toplevel_branch,
  nested_branch
} branch_level;


/* slice identification */
enum
{
  max_nr_slices = 40,
  no_slice = max_nr_slices
};

extern Slice slices[max_nr_slices];

extern slice_index root_slice;

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
  slack_length_direct = 2, /* half moves */
  slack_length_help = 2,   /* half moves */
  slack_length_series = 1  /* half moves */
};

/* Characterisation of attacking moves:
 */
typedef enum
{
  attack_key,
  attack_try,
  attack_regular
} attack_type;


/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void);

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice_index(slice_index si);

/* Allocate a target leaf slice.
 * @param type which STLeaf* type
 * @param s target square
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(SliceType type, square s);

/* Allocate a (non-target) leaf slice.
 * @param type which STLeaf* type
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(SliceType type, Goal goal);

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original);

/* Release all slices
 */
void release_slices(void);

/* Set the min_length field of a composite slice.
 * @param si index of composite slice
 * @param min_length value to be set
 * @return previous value of min_length field
 */
stip_length_type set_min_length(slice_index si, stip_length_type min_length);

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si);

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void);

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(Goal const goals[], size_t nrGoals);

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in_one_of(Goal const goals[], size_t nrGoals);

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return no_slice if goal is not unique; index of a slice with the
 *         unique goal otherwise
 */
slice_index find_unique_goal(slice_index si);

/* Make the stipulation exact
 */
void stip_make_exact(void);

/* Impose the starting side on the stipulation
 */
void stip_impose_starter(Side starter);


struct slice_traversal;

/* Type of callback for stipulation traversals
 */
typedef boolean (*slice_operation)(slice_index si, struct slice_traversal *st);

/* Mapping of slice types to operations.
 */
typedef slice_operation const (*operation_mapping)[nr_slice_types];

typedef enum
{
  slice_not_traversed,
  slice_being_traversed, /* used for avoiding infinite recursion */
  slice_traversed
} slice_traversal_slice_state;

typedef struct slice_traversal
{
    slice_traversal_slice_state traversed[max_nr_slices];
    operation_mapping ops;
    void *param;
} slice_traversal;

/* Initialise a slice_traversal structure
 * @param st to be initialised
 * @param ops operations to be invoked on slices
 * @param param parameter to be passed t operations
 */
void slice_traversal_init(slice_traversal *st,
                          operation_mapping ops,
                          void *param);

/* Query the traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
slice_traversal_slice_state
get_slice_traversal_slice_state(slice_index si, slice_traversal *st);

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 * @return true
 */
boolean slice_operation_noop(slice_index si, slice_traversal *st);

/* (Approximately) depth-first traversal of the children of a slice
 * @param si slice whose children to traverse
 * @param st address of structure defining traversal
 * @return if the children of si have been successfully traversed
 */
boolean slice_traverse_children(slice_index si, slice_traversal *st);

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 * @return true iff root and its children have been successfully
 *         traversed
 */
boolean traverse_slices(slice_index root, slice_traversal *st);

#endif
