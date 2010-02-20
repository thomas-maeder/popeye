#include "pypipe.h"
#include "pykeepmt.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pyleaf.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STKeepMatingGuardRootDefenderFilter slice
 * @param side mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_root_defender_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardRootDefenderFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardAttackerFilter slice
 * @param side mating side
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
 * @param side mating side
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

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type
keepmating_guard_direct_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_has_solution_in_n(slices[si].u.pipe.next,n,n_min);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean keepmating_guard_are_threats_refuted_in_n(table threats,
                                                  stip_length_type len_threat,
                                                  slice_index si,
                                                  stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_are_threats_refuted_in_n(threats,len_threat,
                                             slices[si].u.pipe.next,
                                             n);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void keepmating_guard_direct_solve_continuations_in_n(slice_index si,
                                                      stip_length_type n,
                                                      stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(is_a_mating_piece_left(slices[si].u.keepmating_guard.mating));
  direct_solve_continuations_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
keepmating_guard_direct_solve_threats_in_n(table threats,
                                           slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (is_a_mating_piece_left(mating))
  {
    slice_index const next = slices[si].u.pipe.next;
    result = direct_solve_threats_in_n(threats,next,n,n_min);
  }
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean keepmating_guard_root_defend(slice_index si)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_root_defend(slices[si].u.pipe.next);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean keepmating_guard_defend_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_defend_in_n(next,n);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int keepmating_guard_can_defend_in_n(slice_index si,
                                              stip_length_type n,
                                              unsigned int max_result)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_can_defend_in_n(next,n,max_result);
  else
    result = max_result+1;

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
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (is_a_mating_piece_left(mating)
            && help_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_help_has_solution_in_n(slice_index si,
                                                stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (is_a_mating_piece_left(mating)
            && help_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_help_solve_threats_in_n(table threats,
                                              slice_index si,
                                              stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    help_solve_threats_in_n(threats,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (is_a_mating_piece_left(mating)
            && series_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_series_has_solution_in_n(slice_index si,
                                                  stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (is_a_mating_piece_left(mating)
            && series_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_series_solve_threats_in_n(table threats,
                                                slice_index si,
                                                stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    series_solve_threats_in_n(threats,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */


/* **************** Stipulation instrumentation ***************
 */

/* Data structure for remembering the side(s) that needs to keep >= 1
 * piece that could deliver mate
 */
typedef boolean keepmating_type[nr_sides];

static boolean keepmating_guards_inserter_leaf(slice_index si,
                                               slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*km)[slices[si].starter] = true;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_quodlibet(slice_index si,
                                                    slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  traverse_slices(slices[si].u.binary.op1,st);

  st->param = &km2;
  traverse_slices(slices[si].u.binary.op2,st);

  (*km)[White] = km1[White] && km2[White];
  (*km)[Black] = km1[Black] && km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_reciprocal(slice_index si,
                                                     slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  traverse_slices(slices[si].u.binary.op1,st);

  st->param = &km2;
  traverse_slices(slices[si].u.binary.op2,st);

  (*km)[White] = km1[White] || km2[White];
  (*km)[Black] = km1[Black] || km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch_fork(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order slice_traverse_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  traverse_slices(slices[si].u.branch_fork.towards_goal,st);
  traverse_slices(slices[si].u.pipe.next,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch_direct_root(slice_index si,
                                                             slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_root_defender_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_root_defender_filter(Black);

  if (guard!=no_slice)
  {
    pipe_link(si,guard);

    if (slices[next].prev==si)
      pipe_link(guard,next);
    else
      pipe_set_successor(guard,next);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_defender(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_attacker_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_attacker_filter(Black);

  if (guard!=no_slice)
  {
    pipe_link(si,guard);

    if (slices[next].prev==si)
      pipe_link(guard,next);
    else
      pipe_set_successor(guard,next);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch_direct(slice_index si,
                                                        slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_defender_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_defender_filter(Black);

  if (guard!=no_slice)
  {
    pipe_link(si,guard);

    if (slices[next].prev==si)
      pipe_link(guard,next);
    else
      pipe_set_successor(guard,next);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch_help(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_help_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_help_filter(Black);

  if (guard!=no_slice)
  {
    pipe_link(si,guard);

    if (slices[next].prev==si)
      pipe_link(guard,next);
    else
      pipe_set_successor(guard,next);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch_series(slice_index si,
                                                        slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
    guard = alloc_keepmating_guard_series_filter(White);

  if ((*km)[Black])
    guard = alloc_keepmating_guard_series_filter(Black);

  if (guard!=no_slice)
  {
    pipe_link(si,guard);

    if (slices[next].prev==si)
      pipe_link(guard,next);
    else
      pipe_set_successor(guard,next);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const keepmating_guards_inserters[] =
{
  &slice_traverse_children,                /* STProxy */
  &keepmating_guards_inserter_branch_direct,  /* STBranchDirect */
  &keepmating_guards_inserter_defender,    /* STBranchDirectDefender */
  &keepmating_guards_inserter_branch_help,      /* STBranchHelp */
  &keepmating_guards_inserter_branch_fork, /* STHelpFork */
  &keepmating_guards_inserter_branch_series, /* STBranchSeries */
  &keepmating_guards_inserter_branch_fork, /* STSeriesFork */
  &keepmating_guards_inserter_leaf,        /* STLeafDirect */
  &keepmating_guards_inserter_leaf,        /* STLeafHelp */
  &keepmating_guards_inserter_leaf,        /* STLeafForced */
  &keepmating_guards_inserter_reciprocal,  /* STReciprocal */
  &keepmating_guards_inserter_quodlibet,   /* STQuodlibet */
  &slice_traverse_children,                /* STNot */
  &slice_traverse_children,                /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,                /* STMoveInverterSolvableFilter */
  &slice_traverse_children,                /* STMoveInverterSeriesFilter */
  &keepmating_guards_inserter_branch_direct_root, /* STDirectRoot */
  &keepmating_guards_inserter_defender,    /* STDirectDefenderRoot */
  &slice_traverse_children,                /* STDirectHashed */
  &slice_traverse_children,                /* STHelpRoot */
  &slice_traverse_children,                /* STHelpShortcut */
  &slice_traverse_children,                /* STHelpHashed */
  &slice_traverse_children,                /* STSeriesRoot */
  &slice_traverse_children,                /* STSeriesShortcut */
  &slice_traverse_children,                /* STParryFork */
  &slice_traverse_children,                /* STSeriesHashed */
  &slice_traverse_children,                /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,                /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,                /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,                /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,                /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,                /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,                /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,                /* STDirectDefense */
  &slice_traverse_children,                /* STReflexHelpFilter */
  &slice_traverse_children,                /* STReflexSeriesFilter */
  &slice_traverse_children,                /* STReflexAttackerFilter */
  &slice_traverse_children,                /* STReflexDefenderFilter */
  &slice_traverse_children,                /* STSelfAttack */
  &slice_traverse_children,                /* STSelfDefense */
  &slice_traverse_children,                /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,                /* STRestartGuardHelpFilter */
  &slice_traverse_children,                /* STRestartGuardSeriesFilter */
  &slice_traverse_children,                /* STIntelligentHelpFilter */
  &slice_traverse_children,                /* STIntelligentSeriesFilter */
  &slice_traverse_children,                /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,                /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,                /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,                /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,                /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,                /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,                /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,                /* STMaxFlightsquares */
  &slice_traverse_children,                /* STDegenerateTree */
  &slice_traverse_children,                /* STMaxNrNonTrivial */
  &slice_traverse_children,                /* STMaxThreatLength */
  &slice_traverse_children,                /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,                /* STMaxTimeDefenderFilter */
  &slice_traverse_children,                /* STMaxTimeHelpFilter */
  &slice_traverse_children,                /* STMaxTimeSeriesFilter */
  &slice_traverse_children,                /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,                /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,                /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,                /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,                /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,                /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children                 /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_keepmating_guards(void)
{
  keepmating_type km = { false, false };
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&keepmating_guards_inserters,&km);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
