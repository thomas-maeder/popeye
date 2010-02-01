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

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
                                                \
  ENUMERATOR(STProxy),                                                  \
                                                                        \
    ENUMERATOR(STBranchDirect),    /* M-N moves of direct play */       \
    ENUMERATOR(STBranchDirectDefender),                                 \
    ENUMERATOR(STBranchHelp),      /* M-N moves of help play */         \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STBranchSeries),    /* M-N moves of series play */       \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
                                                                        \
    ENUMERATOR(STLeafDirect),      /* goal in 1 */                      \
    ENUMERATOR(STLeafHelp),        /* help-goal in 1 */                 \
    ENUMERATOR(STLeafForced),      /* forced goal in 1 half move */     \
                                                                        \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
                                                                        \
    ENUMERATOR(STMoveInverterRootSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSeriesFilter),    /* inverts side to move */ \
                                                                        \
    ENUMERATOR(STDirectRoot),      /* root level of direct play */      \
    ENUMERATOR(STDirectDefenderRoot), /* root level of postkey direct play */ \
    ENUMERATOR(STDirectHashed),    /* direct play with hash table */    \
                                                                        \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
                                                                        \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */      \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
                                                                        \
    ENUMERATOR(STSelfCheckGuardRootSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardRootDefenderFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardAttackerFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardDefenderFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardHelpFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSeriesFilter),  /* stop when a side exposes its king */ \
                                                                        \
    ENUMERATOR(STDirectDefense),   /* direct play, just played defense */ \
    ENUMERATOR(STReflexHelpFilter),/* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexSeriesFilter),     /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexAttackerFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexDefenderFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfAttack),      /* self play, just played attack */  \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */ \
                                                                        \
    ENUMERATOR(STRestartGuardRootDefenderFilter),    /* write move numbers */ \
    ENUMERATOR(STRestartGuardHelpFilter),    /* write move numbers */ \
    ENUMERATOR(STRestartGuardSeriesFilter),    /* write move numbers */ \
                                                                        \
    ENUMERATOR(STGoalReachableGuardHelpFilter), /* deals with intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardSeriesFilter), /* deals with intelligent mode */ \
    ENUMERATOR(STKeepMatingGuardRootDefenderFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardAttackerFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardDefenderFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardHelpFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardSeriesFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
                                                                        \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

#define ENUMERATION_DECLARE

#include "pyenum.h"

typedef struct
{
    SliceType type;
    Side starter;
    slice_index prev;

    union
    {
        struct /* for type==STLeaf* */
        {
            Goal goal;
            square target; /* for goal==goal_target */
        } leaf;

        struct /* for types with 1 principal subsequent slice */
        {
            slice_index next;

            union
            {
                struct
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index towards_goal;
                } branch;

                struct
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index towards_goal;
                    slice_index short_sols;
                } help_root;

                struct
                {
                    slice_index parrying;
                } parry_fork;

                struct /* for type==STKeepMatingGuard */
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    Side mating;
                } keepmating_guard;

                struct /* for type==STReflex* */
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index avoided;
                } reflex_guard;

                struct /* for type==STMaxThreatLength */
                {
                    stip_length_type length;     /* half moves */
                    stip_length_type min_length; /* half moves */
                    slice_index to_attacker;
                } maxthreatlength_guard;
            } u;
        } pipe;

        struct /* for type==STQuodlibet and type==STReciprocal */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } fork;
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
  max_nr_slices = 80,
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


/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_allocator(void);

/* Allocate a slice index
 * @param type which slice type
 * @return a so far unused slice index
 */
slice_index alloc_slice(SliceType type);

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si);

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si);

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

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred);

/* Release all slices
 */
void release_slices(void);

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 */
void stip_insert_root_slices(void);

/* Substitute links to proxy slices by the proxy's target
 */
void resolve_proxies(void);

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

/* Structure used in the stipulation traversal that creates the slices
 * representing set play
 */
typedef struct
{
    slice_index setplay_slice; /* identifies set play slice(s) */
    slice_index sibling;       /* indicates where to install */
} setplay_slice_production;

/* Attempt to apply the postkey play option to the current stipulation
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean stip_apply_postkeyplay(void);

/* Attempt to add set play to the stipulation
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(void);

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

typedef enum
{
  dont_know_who_decides_on_starter,
  leaf_decides_on_starter
} who_decides_on_starter;

/* Detect the starting side from the stipulation
 */
void stip_detect_starter(void);

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
