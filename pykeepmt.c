#include "pykeepmt.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STKeepMatingGuardAttackerFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_attacker_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardAttackerFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardDefenderFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_defender_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardDefenderFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardHelpFilter slice
 * @param side mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_help_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardHelpFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardSeriesFilter slice
 * @param side mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_series_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardSeriesFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Direct ***************
 */

/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
static boolean is_a_mating_piece_left(Side mating_side)
{
  boolean const is_white_mating = mating_side==White;

  piece p = roib+1;
  while (p<derbla && nbpiece[is_white_mating ? p : -p]==0)
    p++;

  return p<derbla;
}

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
keepmating_guard_direct_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min,
                                          stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
keepmating_guard_direct_solve_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min,
                                   stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = attack_solve_in_n(slices[si].u.pipe.next,n,n_min,n_max_unsolvable);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type keepmating_guard_defend_in_n(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_min,
                                              stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = defense_defend_in_n(next,n,n_min,n_max_unsolvable);
  else
    result = n+4;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
keepmating_guard_can_defend_in_n(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable,
                                 unsigned int max_nr_refutations)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = defense_can_defend_in_n(next,
                                     n,n_max_unsolvable,
                                     max_nr_refutations);
  else
    result = n+4;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_help_solve_in_n(slice_index si,
                                                  stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = help_solve_in_n(slices[si].u.pipe.next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_help_has_solution_in_n(slice_index si,
                                                         stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = help_has_solution_in_n(slices[si].u.pipe.next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_series_solve_in_n(slice_index si,
                                                    stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    result = series_solve_in_n(slices[si].u.pipe.next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_series_has_solution_in_n(slice_index si,
                                                           stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    result = series_has_solution_in_n(slices[si].u.pipe.next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Data structure for remembering the side(s) that needs to keep >= 1
 * piece that could deliver mate
 */
typedef boolean keepmating_type[nr_sides];

static void keepmating_guards_inserter_leaf_forced(slice_index si,
                                                   stip_structure_traversal *st)
{
  keepmating_type * const km = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*km)[advers(slices[si].starter)] = true;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_quodlibet(slice_index si,
                                                 stip_structure_traversal *st)
{
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  stip_traverse_structure(slices[si].u.binary.op1,st);

  st->param = &km2;
  stip_traverse_structure(slices[si].u.binary.op2,st);

  (*km)[White] = km1[White] && km2[White];
  (*km)[Black] = km1[Black] && km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_reciprocal(slice_index si,
                                                  stip_structure_traversal *st)
{
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  stip_traverse_structure(slices[si].u.binary.op1,st);

  st->param = &km2;
  stip_traverse_structure(slices[si].u.binary.op2,st);

  (*km)[White] = km1[White] || km2[White];
  (*km)[Black] = km1[Black] || km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void keepmating_guards_inserter_branch_fork(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order stip_traverse_structure_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);
  stip_traverse_structure_pipe(si,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_defender(slice_index si,
                                                stip_structure_traversal *st)
{
  keepmating_type const * const km = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_attacker_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_attacker_filter(Black);
  
  if (guard!=no_slice)
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    if (next_prev==si)
    {
      slices[guard].starter = advers(slices[si].starter);
      pipe_append(si,guard);
    }
    else
    {
      assert(slices[next_prev].type==STKeepMatingGuardAttackerFilter);
      pipe_set_successor(si,next_prev);
      dealloc_slice(guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void keepmating_guards_inserter_battle_fork(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* towards goal first, to detect the mating side */
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_attack_move(slice_index si,
                                                   stip_structure_traversal *st)
{
  keepmating_type const * const km = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_defender_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_defender_filter(Black);

  if (guard!=no_slice)
  {
    slices[guard].starter = advers(slices[si].starter);
    pipe_append(si,guard);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_help_move(slice_index si,
                                                 stip_structure_traversal *st)
{
  keepmating_type const * const km = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_help_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_help_filter(Black);

  if (guard!=no_slice)
  {
    slices[guard].starter = advers(slices[si].starter);
    pipe_append(si,guard);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_series_move(slice_index si,
                                                   stip_structure_traversal *st)
{
  keepmating_type const * const km = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_series_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_series_filter(Black);

  if (guard!=no_slice)
  {
    slices[guard].starter = advers(slices[si].starter);
    pipe_append(si,guard);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const keepmating_guards_inserters[] =
{
  &stip_traverse_structure_children,       /* STProxy */
  &keepmating_guards_inserter_attack_move, /* STAttackMove */
  &keepmating_guards_inserter_defender,    /* STDefenseMove */
  &keepmating_guards_inserter_help_move,   /* STHelpMove */
  &keepmating_guards_inserter_branch_fork, /* STHelpFork */
  &keepmating_guards_inserter_series_move, /* STSeriesMove */
  &keepmating_guards_inserter_series_move, /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,       /* STSeriesNotLastMove */
  &stip_traverse_structure_children,       /* STSeriesOnlyLastMove */
  &keepmating_guards_inserter_branch_fork, /* STSeriesFork */
  &keepmating_guards_inserter_branch_fork, /* STSeriesOR */
  &keepmating_guards_inserter_leaf_forced, /* STGoalReachedTester */
  &stip_structure_visitor_noop,            /* STLeaf */
  &keepmating_guards_inserter_reciprocal,  /* STReciprocal */
  &keepmating_guards_inserter_quodlibet,   /* STQuodlibet */
  &stip_traverse_structure_children,       /* STNot */
  &stip_traverse_structure_children,       /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,       /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,       /* STMoveInverterSeriesFilter */
  &keepmating_guards_inserter_attack_move, /* STAttackRoot */
  &stip_traverse_structure_children,       /* STDefenseRoot */
  &stip_traverse_structure_children,       /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,       /* STContinuationSolver */
  &stip_traverse_structure_children,       /* STContinuationWriter */
  &stip_traverse_structure_children,       /* STBattlePlaySolver */
  &stip_traverse_structure_children,       /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,       /* STThreatSolver */
  &stip_traverse_structure_children,       /* STZugzwangWriter */
  &stip_traverse_structure_children,       /* STThreatEnforcer */
  &stip_traverse_structure_children,       /* STThreatCollector */
  &stip_traverse_structure_children,       /* STRefutationsCollector */
  &stip_traverse_structure_children,       /* STVariationWriter */
  &stip_traverse_structure_children,       /* STRefutingVariationWriter */
  &stip_traverse_structure_children,       /* STNoShortVariations */
  &stip_traverse_structure_children,       /* STAttackHashed */
  &stip_traverse_structure_children,       /* STHelpRoot */
  &stip_traverse_structure_children,       /* STHelpShortcut */
  &stip_traverse_structure_children,       /* STHelpHashed */
  &stip_traverse_structure_children,       /* STSeriesRoot */
  &stip_traverse_structure_children,       /* STSeriesShortcut */
  &stip_traverse_structure_children,       /* STParryFork */
  &stip_traverse_structure_children,       /* STSeriesHashed */
  &stip_traverse_structure_children,       /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardSeriesFilter */
  &keepmating_guards_inserter_battle_fork, /* STDirectDefenderFilter */
  &stip_traverse_structure_children,       /* STReflexRootFilter */
  &stip_traverse_structure_children,       /* STReflexHelpFilter */
  &stip_traverse_structure_children,       /* STReflexSeriesFilter */
  &keepmating_guards_inserter_battle_fork, /* STReflexAttackerFilter */
  &keepmating_guards_inserter_battle_fork, /* STReflexDefenderFilter */
  &keepmating_guards_inserter_battle_fork, /* STSelfDefense */
  &stip_traverse_structure_children,       /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,       /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,       /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,       /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,       /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,       /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,       /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STMaxFlightsquares */
  &stip_traverse_structure_children,       /* STDegenerateTree */
  &stip_traverse_structure_children,       /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,       /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,       /* STMaxThreatLength */
  &stip_traverse_structure_children,       /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,       /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,       /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,       /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,       /* STRefutationWriter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,       /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,       /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children        /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_keepmating_guards(void)
{
  keepmating_type km = { false, false };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&keepmating_guards_inserters,&km);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
