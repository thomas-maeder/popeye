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

extern Slice slices[];

#endif
