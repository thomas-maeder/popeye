#include "solving/battle_play/threat.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/battle_play/branch.h"
#include "solving/avoid_unsolvable.h"
#include "solving/check.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Table where threats of the various move levels are collected
 */
table threats[maxply+1];

/* Lengths of threats of the various move levels
 */
static stip_length_type threat_lengths[maxply+1];

/* value of threat_lengths[N] that signals that no threats were found
 * in ply N (and that threats[N] may not even refer to a table)
 */
static stip_length_type const no_threats_found = UINT_MAX;

/* count threats not defeated by a defense while we are
 * threat_enforcing
 */
static unsigned int nr_threats_to_be_confirmed;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type threat_defeated_tester_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  ply const threats_ply = parent_ply[parent_ply[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(next,n);

  if (n>=threat_lengths[threats_ply]-2)
  {
    if (is_current_move_in_table(threats[threats_ply]))
    {
      if (slack_length<=result && result<=n)
      {
        --nr_threats_to_be_confirmed;
        if (nr_threats_to_be_confirmed>0)
          /* threats tried so far still work (perhaps shorter than
           * before the current defense), but we haven't tried all
           * threats yet -> don't stop the iteration over the
           * attacking moves
           */
          result = n+2;
      }
      else if (n==threat_lengths[threats_ply]-1)
        /* we have found a defeated threat -> stop the iteration */
        result = n;
    }
    else
      /* not a threat -> don't stop the iteration */
      result = n+2;
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type threat_collector_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(next,n);

  if (slack_length<=result && result<=n)
    append_to_table(threats[parent_ply[parent_ply[nbply]]]);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type threat_solver_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  threats[nbply] = allocate_table();

  if (!is_in_check(slices[si].starter))
    threat_lengths[nbply] = solve(slices[si].next2,n)-1;

  result = solve(next,n);

  free_table(threats[nbply]);
  threat_lengths[nbply] = no_threats_found;
  threats[nbply] = table_nil;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type threat_enforcer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  slice_index const threat_start = slices[si].next2;
  ply const threats_ply = parent_ply[nbply];
  stip_length_type const len_threat = threat_lengths[threats_ply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",len_threat);

  if (len_threat<=slack_length)
    /* the solve has something stronger than threats (typically, it
     * delivers check)
     */
    result = solve(next,n);
  else if (len_threat<=n)
  {
    /* there are >=1 threats - don't report variations shorter than
     * the threats or variations that don't refute any threat
     */
    table const threats_table = threats[threats_ply];
    stip_length_type len_test_threats;

    nr_threats_to_be_confirmed = table_length(threats_table);

    len_test_threats = solve(threat_start,len_threat);

    if (len_test_threats>len_threat)
      /* variation is longer than threat */
      result = solve(next,n);
    else if (len_test_threats>len_threat-2 && nr_threats_to_be_confirmed>0)
      /* variation has same length as the threat(s), but it has
       * defeated at least one threat
       */
      result = solve(next,n);
    else
      /* variation is shorter than threat */
      result = len_test_threats;
  }
  else
    /* zugzwang, or we haven't looked for threats yet */
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* End copying on the visited slice, by moving it to the copy and linking it
 * to a proxy slice that takes its original place
 * @param si visited slice
 * @param st structure representing the copying traversal
 */
static void move_and_stop_copying(slice_index si, stip_structure_traversal *st)
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

/* Create a shallow copy of the visited fork slice
 * @param si visited slice
 * @param st structure representing the copying traversal
 */
static void copy_shallow(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);

  stip_traverse_structure_children_pipe(si,st);

  if (slices[si].next1!=no_slice)
    link_to_branch((*copies)[si],(*copies)[slices[si].next1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_enforcers(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_deep_copies_type copies;
    stip_structure_traversal st_nested;

    slice_index const * const threat_start = st->param;
    slice_index const threat_start_tester = slices[*threat_start].tester;

    assert(*threat_start!=no_slice);
    assert(threat_start_tester!=no_slice);
    assert(slices[threat_start_tester].type==STThreatStart);

    {
      slice_index const prototype = alloc_pipe(STThreatDefeatedTester);
      attack_branch_insert_slices(threat_start_tester,&prototype,1);
    }

    init_deep_copy(&st_nested,st,&copies);
    stip_structure_traversal_override_single(&st_nested,
                                             STThreatDefeatedTester,
                                             &move_and_stop_copying);
    stip_structure_traversal_override_by_function(&st_nested,
                                                  slice_function_conditional_pipe,
                                                  &copy_shallow);
    stip_structure_traversal_override_by_function(&st_nested,
                                                  slice_function_testing_pipe,
                                                  &copy_shallow);
    stip_traverse_structure(threat_start_tester,&st_nested);

    slices[si].next2 = copies[threat_start_tester];
  }

  {
    /* if the threats are short, max_unsolvable might interfere with enforcing
     * them */
    slice_index const prototype = alloc_reset_unsolvable_slice();
    attack_branch_insert_slices(slices[si].next2,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length+1)
  {
    slice_index const prototype = alloc_testing_pipe(STThreatSolver);
    defense_branch_insert_slices(si,&prototype,1);

    stip_traverse_structure_children_pipe(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const threat_start = st->param;
  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  init_deep_copy(&st_nested,st,&copies);
  stip_structure_traversal_override_single(&st_nested,
                                           STThreatEnd,
                                           &end_copying);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_conditional_pipe,
                                                &copy_shallow);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_testing_pipe,
                                                &copy_shallow);
  stip_traverse_structure(*threat_start,&st_nested);

  slices[si].next2 = alloc_proxy_slice();
  link_to_branch(slices[si].next2,copies[*threat_start]);

  {
    slice_index const prototypes[] = {
        alloc_dummy_move_slice(),
        alloc_defense_played_slice(),
        alloc_pipe(STThreatCollector)
    };
    defense_branch_insert_slices_behind_proxy(slices[si].next2,prototypes,3,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_threat_start(slice_index si, stip_structure_traversal *st)
{
  slice_index * const threat_start = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *threat_start = si;

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_threat_start(slice_index si, stip_structure_traversal *st)
{
  slice_index * const threat_start = st->param;
  slice_index const save_threat_start = *threat_start;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  *threat_start = save_threat_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void filter_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_tree)
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_enforcer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_testing_pipe(STThreatEnforcer);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const threat_handler_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                    },
  { STSetplayFork,        &stip_traverse_structure_children_pipe },
  { STReadyForDefense,    &insert_solver                         },
  { STThreatSolver,       &insert_solvers                        },
  { STRefutationsSolver,  &stip_traverse_structure_children_pipe },
  { STNotEndOfBranchGoal, &insert_enforcer                       },
  { STThreatEnforcer,     &insert_enforcers                      },
  { STThreatStart,        &remember_threat_start                 },
  { STAttackAdapter,      &forget_threat_start                   },
  { STDefenseAdapter,     &forget_threat_start                   }
};

enum
{
  nr_threat_handler_inserters = (sizeof threat_handler_inserters
                                 / sizeof threat_handler_inserters[0])
};

/* Instrument the stipulation representation so that it can solve and enforce
 * threats
 * @param si identifies slice where to start
 */
void stip_insert_threat_handlers(slice_index si)
{
  stip_structure_traversal st;
  slice_index threat_start = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&threat_start);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    threat_handler_inserters,
                                    nr_threat_handler_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_insertion_if_too_short(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length+1)
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_start(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_pipe(STThreatStart);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_end(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STThreatEnd);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const threat_boundaries_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                    },
  { STReadyForDefense,    &end_insertion_if_too_short            },
  { STNotEndOfBranchGoal, &insert_start                          },
  { STReadyForAttack,     &insert_end                            },
  { STRefutationsSolver,  &stip_traverse_structure_children_pipe }
};

enum
{
  nr_threat_boundaries_inserters = (sizeof threat_boundaries_inserters
                                    / sizeof threat_boundaries_inserters[0])
};

/* Instrument the stipulation representation with proxy slices marking the
 * beginning and end of the threat
 * @param si identifies slice where to start
 */
void stip_insert_threat_boundaries(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    threat_boundaries_inserters,
                                    nr_threat_boundaries_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
