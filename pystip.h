#if !defined(PYSTIP_H)
#define PYSTIP_H

#include "pygoal.h"
#include "py.h"
#include "pyhash.h"
#include "boolean.h"

typedef enum
{
  PDirect,       /* alternate play attacker against defender */
  PHelp,         /* alternate collaborative play */
  PSeries        /* series play */
} Play;

typedef enum
{
  EDirect,       /* goal in 1 */
  EHelp,         /* help-goal in 1 */
  ESelf,         /* self-goal in 1 */
  EReflex        /* reflex-goal in 1 */
} End;

typedef enum
{
  STLeaf,
  STReciprocal,  /* reciprocal help-goal(recigoal) in 1 */
  STQuodlibet,   /* goal or self/reflex-goal in 1 */
  STSequence     /* continue play with next Slice */
} SliceType;

typedef struct
{
    SliceType type;
    Side starter;

    union U
    {
        struct /* for type==STLeaf */
        {
            End end;
            Goal goal;
            square target; /* for goal==goal_target */
        } leaf;

        struct /* for other values of type */
        {
            /* full moves if play==PDirect, half moves otherw. */
            stip_length_type length;
            boolean is_exact; /* true iff length is to be considered exact */
            Play play;
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } composite;
    } u;
} Slice;

/* slice identification */
enum
{
  max_nr_slices = 10,
  no_slice = max_nr_slices
};

extern Slice slices[max_nr_slices];

/* Example contents of slices:
 *
 * #3:
 *     type         starter length  play       op1 op2 (composite)
 *     type         starter end     goal               (leaf)
 * [0] STSequence   White   3       PDirect    1
 * [1] STLeaf       White   EDirect goal_mate
 *
 * h=2.5:
 *     type         starter length  play       op1 op2
 *     type         starter end     goal
 * [0] STSequence   White   5       PHelp      1
 * [1] STLeaf       White   EHelp   goal_stale
 *
 * s#=2:
 *     type         starter length  play       op1 op2
 *     type         starter end     goal
 * [0] STQuodlibet  White   2       PDirect    1   2
 * [1] STLeaf       White   ESelf   goal_mate
 * [2] STLeaf       White   ESelf   goal_stale
 *
 * reci-h#3:
 *     type         starter length  play       op1 op2
 *     type         starter end     goal
 * [0] STReciprocal Black   6       PHelp      1   2
 * [1] STLeaf       Black   EDirect goal_mate
 * [2] STLeaf       Black   EHelp   goal_mate
 *
 * 8->ser-=3:
 *     type         starter length  play       op1 op2
 *     type         starter end     goal
 * [0] STSequence   Black   9       PSeries    1
 * [1] STSequence   White   3       PSeries    2
 * [2] STLeaf       White   EDirect goal_stale
 */

/* Currently(?), the length field of a composite slice thus gives the
 * number of (half) moves of the human-readable stipulation.
 *
 * This means that the recursion depth of solving the composite slice
 * never reaches the value of length. At (maximal) recursion depth
 * length-2 (help play) rexp. length-1 (non-help play), solving the
 * operands is started.
 *
 * The following symbols represent the number the difference of length
 * and the maximal recursion level:
 */
enum
{
  slack_length_direct = 1, /* moves */
  slack_length_help = 2,   /* half moves */
  slack_length_series = 1  /* moves */
};

/* "Regular starting side" according to stipulation, i.e. starting
 * side were it not for option "WhiteBegins" and set play checking */
extern Side regular_starter;

/* Allocate a composite slice.
 * Initializes type to STSequence and composite fields to null values
 * @return index of allocated slice
 */
slice_index alloc_composite_slice(SliceType type, Play play);

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(End end, square s);

/* Allocate a (non-target) leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(End end, Goal goal);

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original);

/* Release all slices
 */
void release_slices(void);

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

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean slice_is_unsolvable(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si);

/* Determine whether a slice has >=1 solution; only to be called when
 * play has reached the start of the slice, with the appropriate side
 * at the move. 
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_is_solvable(slice_index si);

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void d_slice_solve_continuations(int table, slice_index si);

/* Find and write set play
 * @param si slice index
 */
void slice_root_solve_setplay(slice_index si);

/* Find and write set play provided every set move leads to end
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
void d_slice_root_write_key_solve_postkey(slice_index si,
                                          attack_type type);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si);

/* Solve a slice at root level
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve(boolean restartenabled, slice_index si);

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

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_slice_has_defender_won(slice_index si);

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_slice_has_attacker_lost(slice_index si);

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_slice_has_attacker_won(slice_index si);

/* Find and write variations
 * @param si slice index
 */
void d_slice_solve_variations(slice_index si);

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_slice_is_threat_refuted(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void slice_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of leaf
 */
void slice_impose_starter(slice_index si, Side s);

#endif
