#include "pynontrv.h"
#include "pydata.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
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
  min_length_nontrivial = 2*maxply+slack_length_battle+1;
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
                             +slack_length_battle+1);
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
  return (min_length_nontrivial-slack_length_battle-1)/2;
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
  slice_index const next = slices[si].u.pipe.next;
  unsigned int const nr_refutations_allowed = max_nr_nontrivial+1;
  stip_length_type const parity = ((n-slack_length_battle-1)%2);
  stip_length_type const n_next = min_length_nontrivial+parity;
  stip_length_type const n_max_unsolvable = slack_length_battle-2+parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  non_trivial_count[nbply+1] = 0;
  defense_can_defend_in_n(next,n_next,n_max_unsolvable,nr_refutations_allowed);
  result = non_trivial_count[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Count non-checking moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * Stop counting when more than max_nr_nontrivial have been found
 * @return number of defender's non-trivial moves
 */
static unsigned int count_noncheck_defenses(slice_index si, stip_length_type n)
{
  unsigned int result = 0;
  unsigned int const nr_refutations_allowed = max_nr_nontrivial+1;
  Side const attacker = slices[si].starter; 

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  genmove(attacker);

  while (encore() && result<=nr_refutations_allowed)
  {
    if (jouecoup(nbply,first_play) && !echecc(nbply,attacker))
      ++result;

    repcoup();
  }

  finply();

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

  result = alloc_pipe(STMaxNrNonTrivial);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
max_nr_nontrivial_guard_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si,n);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defense_defend_in_n(next,n,n_max_unsolvable);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

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
max_nr_nontrivial_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable,
                                        unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si,n);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defense_can_defend_in_n(next,
                                       n,n_max_unsolvable,
                                       max_nr_refutations);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_can_defend_in_n(next,
                                     n,n_max_unsolvable,
                                     max_nr_refutations);

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

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
max_nr_nontrivial_counter_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);

  if (result>=min_length_nontrivial)
    ++non_trivial_count[nbply];

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
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
max_nr_nontrivial_counter_solve_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = attack_solve_in_n(slices[si].u.pipe.next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STMaxNrNonChecks slice
 * @return identifier of allocated slice
 */
static slice_index alloc_max_nr_noncheck_guard(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxNrNonChecks);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
max_nr_noncheck_guard_defend_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_noncheck = count_noncheck_defenses(si,n);
    if (max_nr_nontrivial+1>=nr_noncheck)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_noncheck;
      result = defense_defend_in_n(next,n,n_max_unsolvable);
      max_nr_nontrivial += nr_noncheck;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

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
max_nr_noncheck_guard_can_defend_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable,
                                      unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_noncheck = count_noncheck_defenses(si,n);
    if (max_nr_nontrivial+1>=nr_noncheck)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_noncheck;
      result = defense_can_defend_in_n(next,
                                       n,n_max_unsolvable,
                                       max_nr_refutations);
      max_nr_nontrivial += nr_noncheck;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_can_defend_in_n(next,
                                     n,n_max_unsolvable,
                                     max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Stipulation instrumentation ***************
 */

static void nontrivial_guard_inserter_attack_move(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (min_length_nontrivial<=slack_length_battle+1)
    pipe_append(slices[si].prev,alloc_max_nr_noncheck_guard());
  else
    pipe_append(slices[si].prev,alloc_max_nr_nontrivial_guard());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_nontrivial_counter(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (min_length_nontrivial>slack_length_battle+1)
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    if (next_prev==si)
      pipe_append(si,alloc_max_nr_nontrivial_counter());
    else
    {
      assert(slices[next_prev].type==STMaxNrNonTrivialCounter);
      pipe_set_successor(si,next_prev);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors max_nr_nontrivial_guards_inserters[] =
{
  { STDefenseMovePlayed,  &append_nontrivial_counter             },
  { STAttackMoveFiltered, &nontrivial_guard_inserter_attack_move }
};

enum
{
  nr_max_nr_nontrivial_guards_inserters =
  (sizeof max_nr_nontrivial_guards_inserters
   / sizeof max_nr_nontrivial_guards_inserters[0])
};

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
  stip_structure_traversal_override(&st,
                                    max_nr_nontrivial_guards_inserters,
                                    nr_max_nr_nontrivial_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
