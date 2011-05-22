#if !defined(PYSTIP_H)
#define PYSTIP_H

#include <stddef.h>

#include "stipulation/goals/goals.h"
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
  ENUMERATOR(STProxy),                                                  \
    /* battle play structural slices */                                 \
    ENUMERATOR(STAttackAdapter),   /* switch from generic play to attack play */ \
    ENUMERATOR(STDefenseAdapter),  /* switch from generic play to defense play */ \
    ENUMERATOR(STAttackMoveGenerator), /* unoptimised move generator */ \
    ENUMERATOR(STDefenseMoveGenerator), /* unoptimised move generator */ \
    ENUMERATOR(STReadyForAttack),     /* proxy mark before we start playing attacks */ \
    ENUMERATOR(STReadyForDefense),     /* proxy mark before we start playing defenses */ \
    ENUMERATOR(STMinLengthOptimiser), /* don't even try attacks in less than min_length moves */ \
    /* help play structural slices */                                   \
    ENUMERATOR(STHelpAdapter), /* switch from generic play to help play */ \
    ENUMERATOR(STHelpMoveGenerator), /* unoptimised move generator */ \
    ENUMERATOR(STReadyForHelpMove),                                     \
    /* series play structural slices */                                 \
    ENUMERATOR(STSeriesAdapter), /* switch from generic play to series play */ \
    ENUMERATOR(STSeriesMoveGenerator), /* unoptimised move generator */ \
    ENUMERATOR(STSeriesDummyMove),    /* dummy move by the side that does *not* play the series */ \
    ENUMERATOR(STReadyForSeriesMove),                                   \
    ENUMERATOR(STReadyForSeriesDummyMove),                              \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    /* other structural slices */                                       \
    ENUMERATOR(STSetplayFork),                                          \
    ENUMERATOR(STEndOfBranch), /* end of branch, general case (not reflex, not goal) */ \
    ENUMERATOR(STEndOfBranchForced),  /* side at the move is forced to solve fork if possible */ \
    ENUMERATOR(STEndOfBranchGoal), /* end of branch leading to immediate goal */ \
    ENUMERATOR(STConstraint),  /* stop unless some condition is met */ \
    ENUMERATOR(STEndOfRoot), /* proxy slice marking the end of the root branch */ \
    ENUMERATOR(STEndOfIntro), /* proxy slice marking the end of the intro branch */ \
    ENUMERATOR(STDeadEnd), /* stop solving if there are no moves left to be played */ \
    ENUMERATOR(STMove),                                                \
    ENUMERATOR(STShortSolutionsStart), /* proxy slice marking where we start looking for short battle solutions in line mode */ \
    ENUMERATOR(STGoalReachedTester), /* proxy slice marking the start of goal testing */ \
    ENUMERATOR(STGoalMateReachedTester), /* tests whether a mate goal has been reached */ \
    ENUMERATOR(STGoalStalemateReachedTester), /* tests whether a stalemate goal has been reached */ \
    ENUMERATOR(STGoalDoubleStalemateReachedTester), /* tests whether a double stalemate goal has been reached */ \
    ENUMERATOR(STGoalTargetReachedTester), /* tests whether a target goal has been reached */ \
    ENUMERATOR(STGoalCheckReachedTester), /* tests whether a check goal has been reached */ \
    ENUMERATOR(STGoalCaptureReachedTester), /* tests whether a capture goal has been reached */ \
    ENUMERATOR(STGoalSteingewinnReachedTester), /* tests whether a steingewinn goal has been reached */ \
    ENUMERATOR(STGoalEnpassantReachedTester), /* tests whether an en passant goal has been reached */ \
    ENUMERATOR(STGoalDoubleMateReachedTester), /* tests whether a double mate goal has been reached */ \
    ENUMERATOR(STGoalCounterMateReachedTester), /* tests whether a counter-mate goal has been reached */ \
    ENUMERATOR(STGoalCastlingReachedTester), /* tests whether a castling goal has been reached */ \
    ENUMERATOR(STGoalAutoStalemateReachedTester), /* tests whether an auto-stalemate goal has been reached */ \
    ENUMERATOR(STGoalCircuitReachedTester), /* tests whether a circuit goal has been reached */ \
    ENUMERATOR(STGoalExchangeReachedTester), /* tests whether an exchange goal has been reached */ \
    ENUMERATOR(STGoalCircuitByRebirthReachedTester), /* tests whether a circuit by rebirth goal has been reached */ \
    ENUMERATOR(STGoalExchangeByRebirthReachedTester), /* tests whether an "exchange B" goal has been reached */ \
    ENUMERATOR(STGoalAnyReachedTester), /* tests whether an any goal has been reached */ \
    ENUMERATOR(STGoalProofgameReachedTester), /* tests whether a proof game goal has been reached */ \
    ENUMERATOR(STGoalAToBReachedTester), /* tests whether an "A to B" goal has been reached */ \
    ENUMERATOR(STGoalMateOrStalemateReachedTester), /* just a placeholder - we test using the mate and stalemate testers */ \
    ENUMERATOR(STGoalImmobileReachedTester), /* auxiliary slice testing whether a side is immobile */ \
    ENUMERATOR(STGoalNotCheckReachedTester), /* auxiliary slice enforcing that a side is not in check */ \
    /* boolean logic */                                                 \
    ENUMERATOR(STTrue),            /* true leaf slice */                \
    ENUMERATOR(STFalse),           /* false leaf slice */               \
    ENUMERATOR(STAnd),      /* logical AND */                           \
    ENUMERATOR(STOr),       /* logical OR */                            \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    /* auxiliary slices */                                              \
    ENUMERATOR(STCheckDetector), /* detect check delivered by previous move */ \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side exposes its king */ \
    ENUMERATOR(STMoveInverter),    /* inverts side to move */ \
    ENUMERATOR(STMinLengthGuard), /* make sure that the minimum length of a branch is respected */  \
    ENUMERATOR(STForkOnRemaining),     /* fork depending on the number of remaining moves */ \
    /* solver slices */                                                 \
    ENUMERATOR(STFindShortest), /* find the shortest continuation(s) */                                  \
    ENUMERATOR(STFindByIncreasingLength), /* find all solutions */  \
    ENUMERATOR(STRefutationsAllocator), /* (de)allocate the table holding the refutations */ \
    ENUMERATOR(STTrySolver), /* find battle play tries */           \
    ENUMERATOR(STRefutationsSolver), /* find battle play refutations */           \
    ENUMERATOR(STPostKeyPlaySuppressor), /* suppresses output of post key play */ \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatStart), /* proxy slice marking where to start solving threats */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STRefutationsCollector), /* collections refutations */   \
    /* slices enforcing prerequisites of some stipulations */           \
    ENUMERATOR(STDoubleMateFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STCounterMateFilter),  /* enforces precondition for counter-mate */ \
    ENUMERATOR(STPrerequisiteOptimiser), /* optimise if prerequisites are not met */ \
    /* slices implementing user options */                              \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STRestartGuard),    /* write move numbers */             \
    ENUMERATOR(STMaxTimeGuard), /* deals with option maxtime */  \
    ENUMERATOR(STMaxSolutionsInitialiser), /* initialise solution counter for option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsGuard), /* deals with option maxsolutions */  \
    /* slices implementing optimisations */                             \
    ENUMERATOR(STEndOfBranchGoalImmobile), /* end of branch leading to "immobile goal" (#, =, ...) */ \
    ENUMERATOR(STDeadEndGoal), /* like STDeadEnd, but all ends are goals */ \
    ENUMERATOR(STOrthodoxMatingMoveGenerator),                          \
    ENUMERATOR(STKillerMoveCollector), /* remember killer moves */      \
    ENUMERATOR(STKillerMoveMoveGenerator), /* generate attack moves, prioritise killer move (if any) */ \
    ENUMERATOR(STKillerMoveFinalDefenseMove), /* priorise killer move */ \
    ENUMERATOR(STCountNrOpponentMovesMoveGenerator), \
    ENUMERATOR(STEnPassantFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STCastlingFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STAttackHashed),    /* hash table support for attack */  \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
    ENUMERATOR(STIntelligentHelpFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STIntelligentSeriesFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
    ENUMERATOR(STKeepMatingFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonChecks), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonTrivialCounter), /* deals with option NonTrivial */ \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
    ENUMERATOR(STMaxThreatLengthHook), /* where should STMaxThreatLength start looking for threats */ \
    ENUMERATOR(STStopOnShortSolutionsInitialiser), /* intialise stoponshortsolutions machinery */  \
    ENUMERATOR(STStopOnShortSolutionsFilter), /* enforce option stoponshortsolutions */  \
    ENUMERATOR(STAmuMateFilter), /* detect whether AMU prevents a mate */ \
    ENUMERATOR(STUltraschachzwangGoalFilter), /* suspend Ultraschachzwang when testing for mate */ \
    ENUMERATOR(STCirceSteingewinnFilter), /* is 'won' piece reborn? */ \
    ENUMERATOR(STCirceCircuitSpecial), /* has reborn capturee made a circuit? */ \
    ENUMERATOR(STCirceExchangeSpecial), /* has reborn capturee made an exchange? */ \
    ENUMERATOR(STAnticirceTargetSquareFilter), /* target square is not reached by capture */ \
    ENUMERATOR(STAnticirceCircuitSpecial), /* special circuit by rebirth */ \
    ENUMERATOR(STAnticirceExchangeSpecial), /* special exchange by rebirth */ \
    ENUMERATOR(STAnticirceExchangeFilter), /* only apply special test after capture in Anticirce */ \
    ENUMERATOR(STPiecesParalysingMateFilter), /* goal not reached because of special rule? */ \
    ENUMERATOR(STPiecesParalysingStalemateSpecial), /* stalemate by special rule? */ \
    ENUMERATOR(STPiecesKamikazeTargetSquareFilter), /* target square not reached because of capture by Kamikaze piece? */ \
    /* output slices */                                                 \
    ENUMERATOR(STOutputModeSelector), /* select an output mode for the subsequent play */ \
    ENUMERATOR(STIllegalSelfcheckWriter), /* indicate illegal self-check in the diagram position */ \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */      \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STMoveWriter), /* writes moves */ \
    ENUMERATOR(STKeyWriter), /* write battle play keys */               \
    ENUMERATOR(STTryWriter), /* write "but" */                          \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STTrivialEndFilter), /* don't write trivial variations */  \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextTreeDecorationWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextTreeGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextMoveInversionCounter), /* plain text output: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineEndOfIntroSeriesMarker), /* handles the end of the intro series */  \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

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
    SliceType type;
    Side starter;
    slice_index prev;

    union
    {
        struct /* for types with 1 principal subsequent slice */
        {
            slice_index next;
        } pipe;

        struct /* for type==STGoalTargetReachedTester */
        {
            slice_index next;
            square target;
        } goal_target_reached_tester;

        struct /* for goal filter types * */
        {
            slice_index next;
            goal_applies_to_starter_or_adversary applies_to_who;
        } goal_filter;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
        } branch;

        struct
        {
            slice_index next;
            slice_index fork;
        } fork;

        struct
        {
            slice_index next;
            slice_index fork;
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

        struct /* for type==STOr and type==STAnd */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } binary;

        struct /* for slices dealing with a single goal */
        {
            slice_index next;
            Goal goal;
        } goal_handler;

        struct /* for slices dealing with a single goal */
        {
            slice_index next;
            slice_index fork;
            Goal goal;
        } goal_tester;

        struct
        {
            slice_index next;
            output_mode mode;
        } output_mode_selector;
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
  max_nr_slices = 500,
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
  slack_length_battle = 3, /* half moves */
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
#define ENUMERATORS                             \
  ENUMERATOR(slice_function_unspecified),                        \
    ENUMERATOR(slice_function_move_generator),                   \
    ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Provide a subclass relationship between the values of slice_structural_type
 * @param derived
 * @param base
 * @return true iff derived is a subclass of base
 */
boolean slice_structure_is_subclass(slice_structural_type derived,
                                    slice_structural_type base);

/* Determine whether a slice is of some structural type
 * @param si identifies slice
 * @param type identifies type
 * @return true iff slice si has (at least) structural type type
 */
boolean slice_has_structure(slice_index si, slice_structural_type type);

/* Retrieve the structural type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_structural_type slice_get_structural_type(slice_index si);

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

/* Attempt to add set play to the stipulation
 * @param si identifies the root from which to apply set play
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(slice_index si);

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


struct stip_structure_traversal;

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

typedef struct stip_structure_traversal
{
    structure_visitor_map_type map;
    stip_structure_traversal_state traversed[max_nr_slices];
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
                                              SliceType type,
                                              stip_structure_visitor visitor);

/* define an alternative visitor for a particular slice type */
typedef struct
{
    SliceType type;
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
    unsigned int level;
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
    SliceType type;
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
void stip_moves_traversal_override_by_function(stip_moves_traversal *st,
                                               slice_functional_type,
                                               stip_moves_visitor visitor);

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          SliceType type,
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

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void stip_insert_root_slices(slice_index si);

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void stip_insert_intro_slices(slice_index si);

#endif
