#include "solving/battle_play/try.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "pymsg.h"
#include "stipulation/branch.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* Table where refutations are collected
 */
table refutations;

/* Maximum number of refutations to look for as indicated by the user
 */
static unsigned int user_set_max_nr_refutations;

/* are we currently solving refutations? */
boolean are_we_solving_refutations;

/* Read the maximum number of refutations that the user is interested
 * to see
 * @param tok input token from which to read the number
 * @return true iff the number could be successfully read
 */
boolean read_max_nr_refutations(char const *tok)
{
  boolean result;
  char *end;
  unsigned long ul;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  ul = strtoul(tok,&end,10);
  if (tok==end || ul>UINT_MAX)
  {
    user_set_max_nr_refutations = 0;
    result = false;
  }
  else
  {
    user_set_max_nr_refutations = ul;
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Set the maximum number of refutations that the user is interested
 * to see to some value
 * @param mnr maximum number of refutations that the user is
 *            interested to see
 */
void set_max_nr_refutations(unsigned int mnr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",mnr);
  TraceFunctionParamListEnd();

  user_set_max_nr_refutations = mnr;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STRefutationsAllocator defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_allocator(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsAllocator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type refutations_allocator_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(refutations==table_nil);
  refutations = allocate_table();
  result = defend(next,n);
  assert(refutations==get_top_table());
  free_table();
  refutations = table_nil;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_solver(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STRefutationsSolver,no_slice,no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type refutations_solver_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (refutations!=table_nil && table_length(refutations)>0)
  {
    defend(slices[si].u.binary.op1,n);

    are_we_solving_refutations = true;
    max_unsolvable = n;
    defend(slices[si].u.binary.op2,n);
    max_unsolvable = save_max_unsolvable;
    are_we_solving_refutations = false;

    result = n;
  }
  else
    result = defend(slices[si].u.binary.op1,n);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsCollector slice.
 * @param max_nr_refutations maximum number of refutations to be allowed
 * @return index of allocated slice
 */
slice_index alloc_refutations_collector_slice(unsigned int max_nr_refutations)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsCollector);
  slices[result].u.refutation_collector.max_nr_refutations = max_nr_refutations;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
refutations_collector_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.refutation_collector.next;
  unsigned int const max_nr_refutations = slices[si].u.refutation_collector.max_nr_refutations;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(next,n);

  if (result>n)
  {
    assert(get_top_table()==refutations);
    TraceValue("%u\n",get_top_table());
    append_to_top_table();
    if (table_length(get_top_table())<=max_nr_refutations)
      result = n;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_collector_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.refutation_collector.next;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (are_we_solving_refutations)
  {
    if (is_current_move_in_table(refutations))
    {
      max_unsolvable = n;
      attack(next,n);
      max_unsolvable = save_max_unsolvable;
    }

    result = n;
  }
  else
  {
    if (is_current_move_in_table(refutations))
      result = n;
    else
      result = attack(next,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void filter_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_tree)
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void filter_postkey_play(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_root) /* i.e. not set play */
    Message(TryPlayNotApplicable);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_try_solvers_attack_adapter(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_root) /* i.e. not constraint */
  {
    slice_index const prototypes[] =
    {
      alloc_refutations_allocator(),
      alloc_refutations_solver(),
      alloc_pipe(STEndOfRefutationSolvingBranch)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void slice_copy(slice_index si, stip_structure_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (*result!=no_slice)
  {
    slice_index const copy = copy_slice(si);
    link_to_branch(copy,*result);
    *result = copy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_try_solvers_refutations_solver(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_refutations_collector_slice(user_set_max_nr_refutations);
    battle_branch_insert_slices(slices[si].u.fork.next,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const try_solver_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                    },
  { STDefenseAdapter,     &filter_postkey_play                   },
  { STAttackAdapter,      &insert_try_solvers_attack_adapter     },
  { STRefutationsSolver,  &insert_try_solvers_refutations_solver }
};

enum
{
  nr_try_solver_inserters = (sizeof try_solver_inserters
                             / sizeof try_solver_inserters[0])
};

/* Instrument the stipulation structure with slices solving tries
 * @param root_slice root slice of the stipulation
 */
void stip_insert_try_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override(&st,try_solver_inserters,nr_try_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_constraint_solver(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  assert(*result!=no_slice);

  {
    slice_index const solver = alloc_constraint_solver_slice(stip_deep_copy(slices[si].u.fork.fork));
    link_to_branch(solver,*result);
    *result = solver;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void serve_as_hook(slice_index si, stip_structure_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*result==no_slice);
  *result = alloc_dead_end_slice();
  slices[*result].u.pipe.next = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const to_refutation_branch_copiers[] =
{
  { STThreatSolver,                 &stip_traverse_structure_pipe },
  { STEndOfBranchForced,            &stip_traverse_structure_pipe },
  { STPlaySuppressor,               &stip_traverse_structure_pipe },
  { STThreatSolver,                 &stip_traverse_structure_pipe },
  { STConstraintTester,             &insert_constraint_solver     },
  { STEndOfRefutationSolvingBranch, &serve_as_hook                }
};

enum
{
  nr_to_refutation_branch_copiers = (sizeof to_refutation_branch_copiers
                                     / sizeof to_refutation_branch_copiers[0])
};

static void spin_off_from_refutations_solver(slice_index si,
                                             stip_structure_traversal *st)
{
  stip_structure_traversal st_nested;
  slice_index spun_off = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st_nested,&spun_off);
  stip_structure_traversal_override_by_structure(&st_nested,
                                                 slice_structure_pipe,
                                                 &slice_copy);
  stip_structure_traversal_override_by_structure(&st_nested,
                                                 slice_structure_branch,
                                                 &slice_copy);
  stip_structure_traversal_override_by_structure(&st_nested,
                                                 slice_structure_fork,
                                                 &slice_copy);
  stip_structure_traversal_override(&st_nested,
                                    to_refutation_branch_copiers,
                                    nr_to_refutation_branch_copiers);
  stip_traverse_structure(slices[si].u.binary.op1,&st_nested);

  assert(spun_off!=no_slice);
  slices[si].u.binary.op2 = alloc_proxy_slice();
  link_to_branch(slices[si].u.binary.op2,spun_off);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin a separate branch for solving refutations off the STRefutationsSolver
 * slice
 * @param si identifies entry branch into stipulation
 */
void stip_spin_off_refutation_solver_slices(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_single(&st,
                                           STRefutationsSolver,
                                           &spin_off_from_refutations_solver);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
