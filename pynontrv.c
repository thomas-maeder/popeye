#include "pynontrv.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solving.h"
#include "solving/avoid_unsolvable.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type min_length_nontrivial;
unsigned int max_nr_nontrivial;

/* Lengths of threats of the various move levels
 */
static unsigned int non_trivial_count[maxply+1];

/* Reset the non-trivial optimisation setting to off
 */
void reset_nontrivial_settings(void)
{
  max_nr_nontrivial = UINT_MAX;
  min_length_nontrivial = 2*maxply+slack_length+1;
}

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read maximum number of
 *            acceptable non-trivial variations (apart from main variation)
 * @return true iff setting was successfully read
 */
boolean read_max_nr_nontrivial(char const *tok)
{
  boolean result;
  char *end;
  unsigned long const requested_max_nr_nontrivial = strtoul(tok,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%s\n",tok);

  if (tok!=end && requested_max_nr_nontrivial<=UINT_MAX)
  {
    result = true;
    max_nr_nontrivial = (unsigned int)requested_max_nr_nontrivial;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read minimimal length of what
 *            is to be considered a non-trivial variation
 * @return true iff setting was successfully read
 */
boolean read_min_length_nontrivial(char const *tok)
{
  boolean result;
  char *end;
  unsigned long const requested_min_length_nontrivial = strtoul(tok,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  TraceValue("%s\n",tok);

  if (tok!=end && requested_min_length_nontrivial<=UINT_MAX)
  {
    result = true;
    min_length_nontrivial = (2*(unsigned int)requested_min_length_nontrivial
                             +slack_length+1);
    TraceValue("%u\n",min_length_nontrivial);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the current minimum length (in full moves) of what is to
 * be considered a non-trivial variation
 * @return maximum acceptable number of non-trivial variations
 */
stip_length_type get_min_length_nontrivial(void)
{
  return (min_length_nontrivial-slack_length-1)/2;
}


/* **************** Private helpers ***************
 */

/* Count non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * Stop counting when more than max_nr_nontrivial have been found
 * @return number of defender's non-trivial moves
 */
static unsigned int count_nontrivial_defenses(slice_index si,
                                              stip_length_type n)
{
  unsigned int result;
  slice_index const tester = slices[si].u.fork.fork;
  stip_length_type const parity = ((n-slack_length-1)%2);
  stip_length_type const n_next = min_length_nontrivial+parity;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n_next<=max_unsolvable)
    /* we may get here in solving mode when we know that there is now short
     * solution */
    max_unsolvable = n_next-1;
  TraceValue("->%u\n",max_unsolvable);

  non_trivial_count[nbply+1] = 0;
  defend(tester,n_next);
  result = non_trivial_count[nbply+1];

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Initialisation ***************
 */

/* Allocate a STMaxNrNonTrivial slice
 * @return identifier of allocated slice
 */
static slice_index alloc_max_nr_nontrivial_guard(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_testing_pipe(STMaxNrNonTrivial);

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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type
max_nr_nontrivial_guard_defend(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.fork.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si,n);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defend(next,n);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = n+2;
  }
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STMaxNrNonTrivialCounter slice
 * @return identifier of allocated slice
 */
static slice_index alloc_max_nr_nontrivial_counter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxNrNonTrivialCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type max_nr_nontrivial_counter_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(next,n);

  if (result>n)
  {
    ++non_trivial_count[nbply];
    if (non_trivial_count[nbply]<=max_nr_nontrivial+1)
      result = n;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static void nontrivial_guard_inserter(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototypes[] =
    {
      alloc_max_nr_nontrivial_guard(),
      alloc_max_nr_nontrivial_counter()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STMaxNrNonTrivial slices
 * @param si identifies slice where to start
 */
void stip_insert_max_nr_nontrivial_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STNotEndOfBranch,
                                           &nontrivial_guard_inserter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);
  pipe_substitute(si,alloc_proxy_slice());
  link_to_branch((*copies)[si],si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_shallow(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);

  stip_traverse_structure_pipe(si,st);

  if (slices[si].u.pipe.next!=no_slice)
    link_to_branch((*copies)[si],(*copies)[slices[si].u.pipe.next]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void switch_to_testing(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = alloc_proxy_slice();

  stip_traverse_structure(slices[si].u.fork.fork,st);
  link_to_branch((*copies)[si],(*copies)[slices[si].u.fork.fork]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index spin_off_counting_slices(slice_index si)
{
  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_deep_copy(&st_nested,&copies);
  st_nested.context = stip_traversal_context_defense;
  stip_structure_traversal_override_single(&st_nested,
                                           STMaxNrNonTrivialCounter,
                                           &stop_copying);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_conditional_pipe,
                                                &copy_shallow);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_testing_pipe,
                                                &switch_to_testing);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",copies[si]);
  TraceFunctionResultEnd();
  return copies[si];
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off an end of a STMaxNrNonTrivial slice
 * @param si identifies the STMaxNrNonTrivial slice
 * @param st address of structure representing traversal
 */
void spin_off_testers_max_nr_non_trivial(slice_index si,
                                         stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = copy_slice(si);
    stip_traverse_structure_children(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
    slices[si].u.fork.fork = spin_off_counting_slices(state->spun_off[slices[si].u.fork.next]);
    slices[state->spun_off[si]].u.fork.fork = slices[si].u.fork.fork;
  }
  else
  {
    stip_traverse_structure_children(si,st);
    slices[si].u.fork.fork = alloc_proxy_slice();
    link_to_branch(slices[si].u.fork.fork,spin_off_counting_slices(slices[si].u.fork.next));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
