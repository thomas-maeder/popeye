#if !defined(PYSTIP_H)
#define PYSTIP_H

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
  STBranchHelp,   /* M-N moves of help play */
  STBranchSeries, /* M-N moves of series play */

  STLeafDirect,   /* goal in 1 */
  STLeafHelp,     /* help-goal in 1 */
  STLeafSelf,     /* self-goal in 1 */
  STLeafForced,   /* forced goal in 1 half move */

  STReciprocal,   /* logical AND */
  STQuodlibet,    /* logical OR */
  STNot,          /* logical NOT */

  STConstant,     /* logical constant */
  
  STMoveInverter  /* 0 length, inverts side at move */
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

    union
    {
        struct /* for type==STLeaf* */
        {
            Side starter;
            Goal goal;
            square target; /* for goal==goal_target */
        } leaf;

        struct /* for type==STBranch* */
        {
            Side starter;
            /* full moves if type==STBranchDirect, half moves otherw. */
            stip_length_type length;
            stip_length_type min_length; /* of short solutions */
            slice_index next;
            slice_index derived_from;
        } branch;

        struct /* for type==STQuodlibet */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } quodlibet;

        struct /* for type==STReciprocal */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } reciprocal;

        struct /* for type==STNot */
        {
            slice_index op;
        } not;

        struct /* for type==STConstant */
        {
            boolean value;
        } constant;

        struct /* for type==STMoveInverter */
        {
            Side starter;
            slice_index next;
        } move_inverter;
    } u;
} Slice;


/* slice identification */
enum
{
  max_nr_slices = 10,
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

/* Allocate a branch slice.
 * @param type which STBranch* type
 * @param length number of moves of branch (semantics depends on type)
 * @param min_length minimal number of moves
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_slice(SliceType type,
                               stip_length_type length,
                               stip_length_type min_length,
                               slice_index next);

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

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void);

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(Goal const goals[], unsigned int nrGoals);

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in(Goal const goals[], unsigned int nrGoals);

/* Traversal of the stipulation tree up to the next slice with a
 * specific goal. Repeated calls, with start set to the result of the
 * previous call, result in a complete traversal.
 * @param goal defines where to stop traversal
 * @param start traversal starts (continues) at the identified slice
 *              (excluding it, i.e. the result will be different from
 *              start)
 * @return if found, index of the next slice with the requested goal;
 *         no_slice otherwise
 */
slice_index find_next_goal(Goal goal, slice_index start);

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @return no_slice if goal is not unique; index of a slice with the
 *         unique goal otherwise
 */
slice_index find_unique_goal(void);

/* Make the stipulation exact
 * @param si slice index
 */
void stip_make_exact(slice_index si);

#endif
