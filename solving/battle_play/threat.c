#include "solving/battle_play/threat.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/move_played.h"
#include "stipulation/battle_play/branch.h"
#include "solving/avoid_unsolvable.h"
#include "solving/check.h"
#include "solving/fork.h"
#include "solving/testing_pipe.h"
#include "solving/pipe.h"
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
void threat_defeated_tester_solve(slice_index si)
{
  ply const threats_ply = parent_ply[parent_ply[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  /* we are not interested in whether threats are refuted when we are looking
   * for short continuations to defenses
   */
  if (solve_nr_remaining>=threat_lengths[threats_ply]-2)
  {
    if (is_current_move_in_table(threats[threats_ply]))
    {
      if (move_has_solved())
      {
        --nr_threats_to_be_confirmed;
        if (nr_threats_to_be_confirmed>0)
          /* threats tried so far still work (perhaps shorter than
           * before the current defense), but we haven't tried all
           * threats yet -> don't stop the iteration over the
           * attacking moves
           */
          solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
      }
      else if (solve_nr_remaining==threat_lengths[threats_ply]-1)
        /* we have found a defeated threat -> stop the iteration */
        solve_result = MOVE_HAS_SOLVED_LENGTH();
    }
    else
      /* not a threat -> don't stop the iteration */
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void threat_check_detector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!is_in_check(SLICE_STARTER(si)))
  {
    pipe_solve_delegate(si);
    threat_lengths[parent_ply[nbply]] = solve_result-1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void threat_collector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (move_has_solved())
    append_to_table(threats[parent_ply[parent_ply[nbply]]]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void threat_solver_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  threats[nbply] = allocate_table();

  fork_solve_delegate(si);
  pipe_solve_delegate(si);

  free_table(threats[nbply]);
  threat_lengths[nbply] = no_threats_found;
  threats[nbply] = table_nil;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void threat_enforcer_solve(slice_index si)
{
  ply const threats_ply = parent_ply[nbply];
  stip_length_type const len_threat = threat_lengths[threats_ply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",len_threat);
  TraceEOL();

  if (len_threat<=slack_length)
    /* the move has something stronger than threats (typically, it
     * delivers check)
     */
    pipe_solve_delegate(si);
  else if (len_threat<=MOVE_HAS_SOLVED_LENGTH())
  {
    /* there are >=1 threats - don't report variations shorter than
     * the threats or variations that don't refute any threat
     */
    table const threats_table = threats[threats_ply];
    stip_length_type len_test_threats;

    nr_threats_to_be_confirmed = table_length(threats_table);

    len_test_threats = testing_pipe_solve_delegate(si,len_threat);

    if (len_test_threats>len_threat)
      /* variation is longer than threat */
      pipe_solve_delegate(si);
    else if (len_test_threats>len_threat-2 && nr_threats_to_be_confirmed>0)
      /* variation has same length as the threat(s), but it has
       * defeated at least one threat
       */
      pipe_solve_delegate(si);
    else
      /* variation is shorter than threat */
      solve_result = len_test_threats;
  }
  else
    /* zugzwang, or we haven't looked for threats yet */
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  if (SLICE_NEXT1(si)!=no_slice)
    link_to_branch((*copies)[si],(*copies)[SLICE_NEXT1(si)]);

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
    slice_index const threat_start_tester = SLICE_TESTER(*threat_start);

    assert(*threat_start!=no_slice);
    assert(threat_start_tester!=no_slice);
    assert(SLICE_TYPE(threat_start_tester)==STThreatStart);

    {
      slice_index const prototype = alloc_pipe(STThreatDefeatedTester);
      attack_branch_insert_slices(threat_start_tester,&prototype,1);
    }

    init_deep_copy(&st_nested,st,&copies);
    stip_structure_traversal_override_single(&st_nested,
                                             STThreatDefeatedTester,
                                             &move_and_stop_copying);
    stip_structure_traversal_override_by_contextual(&st_nested,
                                                    slice_contextual_conditional_pipe,
                                                    &copy_shallow);
    stip_structure_traversal_override_by_contextual(&st_nested,
                                                    slice_contextual_testing_pipe,
                                                    &copy_shallow);
    stip_traverse_structure(threat_start_tester,&st_nested);

    SLICE_NEXT2(si) = copies[threat_start_tester];
  }

  {
    /* if the threats are short, max_unsolvable might interfere with enforcing
     * them */
    slice_index const prototype = alloc_reset_unsolvable_slice();
    attack_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
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

  if (SLICE_U(si).branch.length>slack_length+1)
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
  stip_structure_traversal_override_by_contextual(&st_nested,
                                                  slice_contextual_conditional_pipe,
                                                  &copy_shallow);
  stip_structure_traversal_override_by_contextual(&st_nested,
                                                  slice_contextual_testing_pipe,
                                                  &copy_shallow);
  stip_traverse_structure(*threat_start,&st_nested);

  SLICE_NEXT2(si) = alloc_proxy_slice();
  link_to_branch(SLICE_NEXT2(si),copies[*threat_start]);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STDummyMove),
        alloc_pipe(STThreatCheckDetector),
        alloc_defense_played_slice(),
        alloc_pipe(STThreatCollector)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),prototypes,nr_prototypes,si);
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

  if (SLICE_U(si).output_mode_selector.mode==output_mode_tree)
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
void solving_insert_threat_handlers(slice_index si)
{
  stip_structure_traversal st;
  slice_index threat_start = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&threat_start);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    threat_handler_inserters,
                                    nr_threat_handler_inserters);
  stip_traverse_structure(si,&st);

  reset_tables();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_insertion_if_too_short(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(si).branch.length>slack_length+1)
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
void solving_insert_threat_boundaries(slice_index si)
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
