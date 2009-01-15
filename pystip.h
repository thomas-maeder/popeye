#if !defined(PYSTIP_H)
#define PYSTIP_H

#include "pygoal.h"
#include "py.h"
#include "boolean.h"

typedef enum
{
  STReciprocal,   /* logical AND */
  STQuodlibet,    /* logical OR */
  STNot,          /* logical NOT */
  STBranchDirect, /* M-N moves of direct play */
  STBranchHelp,   /* M-N moves of help play */
  STBranchSeries, /* M-N moves of series play */
  STLeafDirect,   /* goal in 1 */
  STLeafHelp,     /* help-goal in 1 */
  STLeafSelf      /* self-goal in 1 */
} SliceType;

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
        } branch;

        struct
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } quodlibet;

        struct
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } reciprocal;

        struct
        {
            slice_index op;
        } not;
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

/* Example contents of slices:
 *
 * #3:
 *     type           starter length  next
 *     type           starter goal
 * [0] STBranchDirect White   2       1
 * [1] STLeafDirect   White   goal_mate
 *
 * h=2.5:
 *     type           starter length  next
 *     type           starter goal
 * [0] STBranchHelp   White   5       1
 * [1] STLeafHelp     White   goal_stale
 *
 * s#=2:
 *     type           op1 op2
 *     type           starter goal
 * [0] STQuodlibet    1   2
 * [1] STLeafSelf     White   goal_mate
 * [2] STLeafSelf     White   goal_stale
 *
 * reci-h#3:
 *     type           op1 op2
 *     type           starter goal
 * [0] STReciprocal   1   2
 * [1] STLeafDirect   Black   goal_mate
 * [2] STLeafHelp     Black   goal_mate
 *
 * 8->ser-=3:
 *     type           starter length  next
 *     type           starter goal
 * [0] STBranchSeries Black   9       1
 * [1] STBranchSeries White   3       2
 * [2] STLeafDirect   White   goal_stale
 */

/* The length field of a series and help branch slices thus gives the
 * number of (half) moves of the *human-readable* stipulation, to
 * nicely cope with set play.
 *
 * This means that the recursion depth of solving the branch slice
 * never reaches the value of length. At (maximal) recursion depth
 * length-2 (help play) rexp. length-1 (series play), solving the
 * operands resp. next slice is started.
 *
 * The following symbols represent the difference of the length and
 * the maximal recursion level:
 */
enum
{
  slack_length_help = 2,   /* half moves */
  slack_length_series = 1  /* moves */
};

/* "Regular starting side" according to stipulation, i.e. starting
 * side were it not for option "WhiteBegins" and set play checking */
extern Side regular_starter;

/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void);

/* Allocate a branch slice.
 * @return index of allocated slice
 */
slice_index alloc_branch_slice(SliceType type);

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

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean slice_must_starter_resign(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si);

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of slice
 */
void slice_solve_continuations(int table, slice_index si);

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean slice_root_solve_setplay(slice_index si);

/* Find and write set play provided every set move solves
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean slice_root_solve_complete_set(slice_index si);

typedef enum
{
  attack_key,
  attack_try,
  attack_regular
} attack_type;

/* Write the key just played, then continue solving in the slice
 * to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void slice_root_write_key_solve_postkey(slice_index si, attack_type type);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 */
void slice_root_solve(slice_index si);

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void slice_root_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean slice_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean slice_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean slice_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean slice_has_starter_won(slice_index si);

/* Find and write variations
 * @param si slice index
 */
void slice_solve_variations(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void slice_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void slice_impose_starter(slice_index si, Side s);

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side slice_get_starter(slice_index si);

#endif
