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
    ENUMERATOR(STAttackRoot),      /* root attack level of battle play */ \
    ENUMERATOR(STAttackMove),                                           \
    ENUMERATOR(STAttackFindShortest),                                   \
    ENUMERATOR(STDefenseMove),                                          \
    ENUMERATOR(STReflexAttackerFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexDefenderFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */ \
    ENUMERATOR(STDefenseDealtWith),      /* battle play, half-moves used up */ \
    ENUMERATOR(STRootAttackFork),  /* battle play, continue with subsequent branch */ \
    ENUMERATOR(STAttackFork),      /* battle play, continue with subsequent branch */ \
    ENUMERATOR(STAttackDealtWith),     /* battle play, half-moves used up */ \
    ENUMERATOR(STDefenseFork),     /* battle play, continue with subsequent branch */ \
    ENUMERATOR(STReadyForAttack),     /* proxy mark before we start playing attacks */ \
    ENUMERATOR(STAttackMovePlayed), /* proxy mark after attack moves have been fully played */ \
    ENUMERATOR(STAttackMoveShoeHorningDone), /* proxy mark after slices shoehorning special tests on attack moves */ \
    ENUMERATOR(STAttackMoveLegalityChecked), /* proxy mark after slices that have checked the legality of attack moves */ \
    ENUMERATOR(STReadyForDefense),     /* proxy mark before we start playing defenses */ \
    ENUMERATOR(STDefenseMovePlayed),     /* proxy mark after defense moves have been fully played */ \
    ENUMERATOR(STDefenseMoveShoeHorningDone), /* proxy mark after slices shoehorning special tests on defense moves */ \
    ENUMERATOR(STDefenseMoveLegalityChecked), /* proxy mark after slices that have checked the legality of defense moves */ \
    ENUMERATOR(STDefenseMoveFiltered), /* proxy mark after slices that have filtered irrelevant defense moves */ \
    /* help play structural slices */                                   \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpShortcut),    /* selects branch for solving short solutions */        \
    ENUMERATOR(STHelpMove),      /* M-N moves of help play */           \
    ENUMERATOR(STHelpMoveToGoal),  /* last help move reaching goal */   \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STReadyForHelpMove),                                     \
    ENUMERATOR(STHelpMovePlayed),                                       \
    ENUMERATOR(STHelpMoveLegalityChecked),                              \
    ENUMERATOR(STHelpMoveDealtWith),                                    \
    ENUMERATOR(STReflexHelpFilter),/* stop when wrong side can reach goal */ \
    /* series play structural slices */                                 \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */      \
    ENUMERATOR(STSeriesShortcut),  /* selects branch for solving short solutions */ \
    ENUMERATOR(STSeriesMove),    /* M-N moves of series play */         \
    ENUMERATOR(STSeriesMoveToGoal),   /* last series move reaching goal */ \
    ENUMERATOR(STSeriesDummyMove),    /* dummy move by the side that does *not* play the series */ \
    ENUMERATOR(STReadyForSeriesMove),                                   \
    ENUMERATOR(STSeriesMovePlayed),                                     \
    ENUMERATOR(STSeriesMoveLegalityChecked),                            \
    ENUMERATOR(STSeriesMoveDealtWith),                                  \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    ENUMERATOR(STReflexSeriesFilter),     /* stop when wrong side can reach goal */ \
    /* other structural slices */                                       \
    ENUMERATOR(STSetplayFork),                                          \
    ENUMERATOR(STGoalReachedTesting), /* proxy slice marking the start of goal testing */ \
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
    ENUMERATOR(STGoalReachedTested), /* proxy slice marking the end of goal testing */ \
    ENUMERATOR(STLeaf),            /* leaf slice */                     \
    /* unary and binary operators */                                    \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    /* auxiliary slices */                                              \
    ENUMERATOR(STCheckDetector), /* detect check delivered by previous move */ \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side exposes its king */ \
    ENUMERATOR(STMoveInverter),    /* inverts side to move */ \
    ENUMERATOR(STStipulationReflexAttackSolver), /* solve forced attack after reflex-specific refutation */  \
    /* solver slices */                                                 \
    ENUMERATOR(STTrySolver), /* find battle play solutions */           \
    ENUMERATOR(STPostKeyPlaySuppressor), /* suppresses output of post key play */ \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STRefutationsCollector), /* collections refutations */   \
    /* slices enforcing prerequisites of some stipulations */           \
    ENUMERATOR(STDoubleMateFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STCounterMateFilter),  /* enforces precondition for counter-mate */ \
    /* slices implementing user options */                              \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STRestartGuard),    /* write move numbers */             \
    /* slices implementing optimisations */                             \
    ENUMERATOR(STAttackMoveToGoal),                                     \
    ENUMERATOR(STKillerMoveCollector), /* remember killer moves */      \
    ENUMERATOR(STKillerMoveFinalDefenseMove), /* priorise killer move */ \
    ENUMERATOR(STEnPassantAttackerFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STEnPassantDefenderFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STEnPassantHelpFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STCastlingAttackerFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STCastlingHelpFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STCastlingSeriesFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STAttackHashed),    /* hash table support for attack */  \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
    ENUMERATOR(STIntelligentHelpFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STIntelligentSeriesFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardHelpFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardSeriesFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
    ENUMERATOR(STKeepMatingFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonChecks), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonTrivialCounter), /* deals with option NonTrivial */ \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
    ENUMERATOR(STMaxTimeRootDefenderFilter), /* deals with option maxtime */ \
    ENUMERATOR(STMaxTimeDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeHelpFilter), /* deals with option maxtime */    \
    ENUMERATOR(STMaxTimeSeriesFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxSolutionsRootSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsRootDefenderFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsHelpFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSeriesFilter), /* deals with option maxsolutions */  \
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
    ENUMERATOR(STIllegalSelfcheckWriter), /* indicate illegal self-check in the diagram position */ \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */      \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STContinuationWriter), /* writes battle play continuations */ \
    ENUMERATOR(STKeyWriter), /* write battle play keys */               \
    ENUMERATOR(STTryWriter), /* write "but" */                          \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STVariationWriter), /* writes variations */              \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextTreeDecorationWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextTreeGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextTreeMoveInversionCounter), /* plain text output, tree mode: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineMoveInversionCounter), /* plain text output, line mode: count move inversions */  \
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
            Goal imminent_goal;
        } branch;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
            slice_index threat_start;
        } threat_solver;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
            slice_index towards_goal;
        } branch_fork;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
            slice_index short_sols;
        } shortcut;

        struct
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
            slice_index non_parrying;
        } parry_fork;

        struct /* for type==STKeepMatingGuard */
        {
            slice_index next;
            Side mating;
        } keepmating_guard;

        struct /* for type==STReflex* */
        {
            slice_index next;
            stip_length_type length;     /* half moves */
            stip_length_type min_length; /* half moves */
            slice_index avoided;
        } reflex_guard;

        struct /* for type==STMaxThreatLength */
        {
            slice_index next;
            stip_length_type length_dummy;     /* half moves */
            stip_length_type min_length_dummy; /* half moves */
            slice_index to_attacker;
        } maxthreatlength_guard;

        struct /* for type==STQuodlibet and type==STReciprocal */
        {
            slice_index op1; /* operand 1 */
            slice_index op2; /* operand 2 */
        } binary;

        struct /* for slices writing a goal */
        {
            slice_index next;
            Goal goal;
        } goal_writer;

        struct /* for type==STLineWriter* */
        {
            slice_index next;
            Goal goal;
            slice_index root_slice;
        } line_writer;
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
  max_nr_slices = 400,
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
  slack_length_battle = 2, /* half moves */
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

/* Determine whether a slice is of some structural type
 * @param si identifies slice
 * @param type identifies type
 * @return true iff slice si has (at least) structural type type
 */
boolean slice_has_structure(slice_index si, slice_structural_type type);

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

/* structure holding the state of the traversal started by
 * stip_insert_root_slices()
 */
typedef struct
{
    slice_index result;           /* (temporary) result of root production */
    boolean dealing_with_setplay; /* are we in a setplay */
} root_insertion_state_type;

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void stip_insert_root_slices(slice_index si);

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
 * @param si identifies slice where to start
 * @return true iff quodlibet could be applied
 */
boolean transform_to_quodlibet(slice_index si);

/* Attempt to apply the postkey play option to the current stipulation
 * @param si identifies slice where to start
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean stip_apply_postkeyplay(slice_index si);

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index stip_make_setplay(slice_index si);

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

/* Make the stipulation exact
 * @param si identifies slice where to start
 */
void stip_make_exact(slice_index si);

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
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_type(stip_structure_traversal *st,
                                               slice_structural_type type,
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
 * @param root root of the sub-tree to traverse
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

#endif
