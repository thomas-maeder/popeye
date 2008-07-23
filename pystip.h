#if !defined(PYSTIP_H)
#define PYSTIP_H

#include "pygoal.h"
#include "py.h"
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
  EReflex,       /* reflex-goal in 1 */
  ESemireflex,   /* semireflex-goal in 1 */
  EDouble,       /* help-double-goal in 1 (mate only) */
  ECounter       /* help-counter-goal in 1 (mate only) */
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
      int length; /* full moves if play==PDirect, half moves otherwise */
      boolean is_exact; /* true iff length is to be considered exact */
      Play play;
      slice_index op1; /* operand 1 */
      slice_index op2; /* operand 2 */
    } composite;
  } u;
} Slice;

enum
{
  max_nr_slices = 10,
  no_slice = max_nr_slices
};

extern Slice slices[max_nr_slices];

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
void release_slices();

/* Transform a sequence slice to quodlibet
 * @param quodlibet_slice index of slice to be transformed
 */
void transform_sequence_to_quodlibet(slice_index quodlibet_slice);

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

#endif
