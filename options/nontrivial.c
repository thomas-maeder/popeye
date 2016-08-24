#include "options/nontrivial.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/avoid_unsolvable.h"
#include "solving/pipe.h"
#include "solving/testing_pipe.h"
#include "solving/ply.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
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
  min_length_nontrivial = 2*maxply+1;
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

  TraceValue("%s",tok);
  TraceEOL();

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

  TraceValue("%s",tok);
  TraceEOL();

  if (tok!=end && requested_min_length_nontrivial<=UINT_MAX)
  {
    result = true;
    min_length_nontrivial = 2*(unsigned int)requested_min_length_nontrivial+1;
    TraceValue("%u",min_length_nontrivial);
    TraceEOL();
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
  return (min_length_nontrivial-1)/2;
}


/* **************** Private helpers ***************
 */

/* Count non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * Stop counting when more than max_nr_nontrivial have been found
 * @return number of defender's non-trivial moves
 */
static unsigned int count_nontrivial_defenses(slice_index si)
{
  unsigned int result;
  stip_length_type const parity = (solve_nr_remaining-slack_length-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  non_trivial_count[nbply+1] = 0;
  testing_pipe_solve_delegate(si,min_length_nontrivial+slack_length+parity);
  result = non_trivial_count[nbply+1];

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

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void max_nr_nontrivial_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_nr_remaining-slack_length>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      pipe_solve_delegate(si);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void max_nr_nontrivial_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (solve_result>MOVE_HAS_SOLVED_LENGTH())
  {
    ++non_trivial_count[nbply];
    if (non_trivial_count[nbply]<=max_nr_nontrivial+1)
      /* found enough non-trivial defenses - stop the iteration
       */
      solve_result = MOVE_HAS_SOLVED_LENGTH();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_nontrivial_guards(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense
      && st->activity==stip_traversal_activity_solving)
  {
    {
      slice_index const prototype = alloc_max_nr_nontrivial_guard();
      defense_branch_insert_slices(si,&prototype,1);
    }
    {
      slice_index const prototype = alloc_max_nr_nontrivial_guard();
      defense_branch_insert_slices(SLICE_TESTER(si),&prototype,1);
    }
  }

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

static slice_index spin_off_counting_slices(slice_index si,
                                            stip_structure_traversal *st)
{
  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_deep_copy(&st_nested,st,&copies);
  stip_structure_traversal_override_single(&st_nested,
                                           STMaxNrNonTrivialCounter,
                                           &stop_copying);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",copies[si]);
  TraceFunctionResultEnd();
  return copies[si];
}

static void alloc_branch_from_tester(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->activity==stip_traversal_activity_solving)
  {
    SLICE_TESTER(si) = SLICE_PREV(SLICE_TESTER(SLICE_NEXT1(si)));

    st->activity = stip_traversal_activity_testing;
    stip_traverse_structure(SLICE_TESTER(si),st);
    st->activity = stip_traversal_activity_solving;

    SLICE_NEXT2(si) = SLICE_NEXT2(SLICE_TESTER(si));
  }
  else
  {
    {
      slice_index const prototype = alloc_max_nr_nontrivial_counter();
      defense_branch_insert_slices(SLICE_NEXT1(si),&prototype,1);
    }

    SLICE_NEXT2(si) = alloc_proxy_slice();
    link_to_branch(SLICE_NEXT2(si),spin_off_counting_slices(SLICE_NEXT1(si),st));

    {
      slice_index const prototype = alloc_reset_unsolvable_slice();
      defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),
                                                &prototype,1,
                                                si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STMaxNrNonTrivial slices
 * @param si identifies slice where to start
 */
void solving_insert_max_nr_nontrivial_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STNotEndOfBranch,
                                           &insert_nontrivial_guards);
  stip_traverse_structure(si,&st);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMaxNrNonTrivial,
                                           &alloc_branch_from_tester);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
