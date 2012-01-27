#include "solving/solving.h"
#include "pydata.h"
#include "pypipe.h"
#include "pyhash.h"
#include "pybrafrk.h"
#include "stipulation/proxy.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/boolean/binary.h"
#include "solving/move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/find_shortest.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/setplay.h"
#include "solving/single_piece_move_generator.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "solving/castling_intermediate_move_generator.h"
#include "solving/single_move_generator.h"
#include "solving/king_move_generator.h"
#include "solving/trivial_end_filter.h"
#include "trace.h"

#include <assert.h>

static void spin_off_testers_threat_enforcer(slice_index si,
                                             stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = alloc_proxy_slice();
    stip_traverse_structure_children(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
    slices[si].u.fork.fork = state->spun_off[slices[si].u.fork.fork];
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void spin_off_testers_threat_collector(slice_index si,
                                              stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = alloc_pipe(STThreatDefeatedTester);
    stip_traverse_structure_children(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void spin_off_testers_max_nr_non_trivial(slice_index si,
                                                stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;
  boolean const save_spinning_off = state->spinning_off;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->spinning_off = true;

  stip_spin_off_testers_testing_pipe(si,st);

  if (state->spun_off[slices[si].prev]!=no_slice)
  {
    /* the testing machinery needs a STMaxNrTrivial slice of its own */
    state->spun_off[si] = copy_slice(si);
    pipe_link(state->spun_off[si],slices[si].u.testing_pipe.tester);
  }

  state->spinning_off = save_spinning_off;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_spinning_off_end_of_root(slice_index si,
                                           stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
    stip_spin_off_testers_pipe(si,st);
  else if (st->context==stip_traversal_context_defense)
  {
    /* we are solving something like #3 option postkey
     * start spinning off testers at the loop entry
     */
    state->spinning_off = true;
    stip_spin_off_testers_pipe(si,st);
    state->spinning_off = false;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_spinning_off_next_branch(slice_index si, stip_structure_traversal *st)
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
    slices[state->spun_off[si]].u.fork.fork = state->spun_off[slices[si].u.fork.fork];
    slices[state->spun_off[si]].u.fork.tester = state->spun_off[slices[si].u.fork.fork];
  }
  else
  {
    stip_traverse_structure_pipe(si,st);

    state->spinning_off = true;
    stip_traverse_structure(slices[si].u.fork.fork,st);
    state->spinning_off = false;
  }

  slices[si].u.fork.tester = alloc_proxy_slice();
  link_to_branch(slices[si].u.fork.tester,state->spun_off[slices[si].u.fork.fork]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_root_max_threat_length_to_spin_off(slice_index si,
                                                       stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    stip_spin_off_testers_pipe(si,st);
    slices[state->spun_off[si]].u.testing_pipe.tester = state->spun_off[slices[si].u.testing_pipe.tester];
  }
  else
  {
    /* trust in our descendants to start spinning off before the traversal
     * reaches our tester */
    stip_traverse_structure_pipe(si,st);
    assert(state->spun_off[slices[si].u.testing_pipe.tester]!=no_slice);
  }

  slices[si].u.testing_pipe.tester = state->spun_off[slices[si].u.testing_pipe.tester];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 */
void stip_spin_off_testers(slice_index si)
{
  spin_off_tester_state_type state;
  stip_structure_traversal st;
  slice_index i;
  slice_structural_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  state.spinning_off = false;
  for (i = 0; i!=max_nr_slices; ++i)
    state.spun_off[i] = no_slice;

  stip_structure_traversal_init(&st,&state);

  for (type = 0; type!=nr_slice_structure_types; ++type)
    if (slice_structure_is_subclass(type,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,
                                                     type,
                                                     &stip_spin_off_testers_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_testing_pipe,
                                                 &stip_spin_off_testers_testing_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &stip_spin_off_testers_fork);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_conditional_pipe,
                                                 &stip_spin_off_testers_conditional_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_leaf,
                                                 &stip_spin_off_testers_leaf);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_binary,
                                                 &stip_spin_off_testers_binary);

  stip_structure_traversal_override_single(&st,STEndOfRoot,&start_spinning_off_end_of_root);
  stip_structure_traversal_override_single(&st,STMaxThreatLength,&connect_root_max_threat_length_to_spin_off);

  stip_structure_traversal_override_single(&st,STAnd,&stip_spin_off_testers_and);

  stip_structure_traversal_override_single(&st,STEndOfBranchForced,&start_spinning_off_next_branch);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&start_spinning_off_next_branch);

  stip_structure_traversal_override_single(&st,STMaxNrNonTrivial,&spin_off_testers_max_nr_non_trivial);
  stip_structure_traversal_override_single(&st,STThreatEnforcer,&spin_off_testers_threat_enforcer);
  stip_structure_traversal_override_single(&st,STThreatCollector,&spin_off_testers_threat_collector);
  stip_structure_traversal_override_single(&st,STTemporaryHackFork,&stip_traverse_structure_pipe);
  stip_structure_traversal_override_single(&st,STAttackHashed,&spin_off_testers_attack_hashed);
  stip_structure_traversal_override_single(&st,STHelpHashed,&spin_off_testers_help_hashed);

  stip_structure_traversal_override_single(&st,STThreatSolver,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STRefutationsSolver,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STPlaySuppressor,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STIntelligentDuplicateAvoider,&stip_spin_off_testers_pipe_skip);

  stip_structure_traversal_override_by_function(&st,
                                                slice_function_writer,
                                                &stip_spin_off_testers_pipe_skip);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_ready_for_move_in_loop(slice_index ready_root)
{
  slice_index result = ready_root;
  do
  {
    result = branch_find_slice(STReadyForHelpMove,result);
  } while ((slices[result].u.branch.length-slack_length_help)%2
           !=(slices[ready_root].u.branch.length-slack_length_help)%2);
  return result;
}

static void insert_solvers_help_adapter(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (length-min_length>=2)
  {
    if (st->level==structure_traversal_level_nested)
    {
      if (st->context==stip_traversal_context_global)
      {
        slice_index const prototype = alloc_find_shortest_slice();
        help_branch_insert_slices(si,&prototype,1);
      }
    }
    else /* root or set play */
    {
      if (length>=slack_length_help+2)
      {
        {
          slice_index const prototype =
              alloc_find_by_increasing_length_slice(length,min_length);
          help_branch_insert_slices(si,&prototype,1);
        }
        {
          slice_index const ready_root = branch_find_slice(STReadyForHelpMove,si);
          slice_index const ready_loop = find_ready_for_move_in_loop(ready_root);
          slice_index const proxy_root = alloc_proxy_slice();
          slice_index const proxy_loop = alloc_proxy_slice();
          pipe_set_successor(proxy_loop,ready_loop);
          pipe_link(slices[ready_root].prev,
                    alloc_fork_on_remaining_slice(proxy_root,proxy_loop,
                                                  length-1-slack_length_help));
          pipe_link(proxy_root,ready_root);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_move_generator_slice();
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void insert_single_move_generator_with_king_capture(slice_index si,
                                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const proto = alloc_single_move_generator_with_king_capture_slice();
    branch_insert_slices(slices[si].u.fork.fork,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_single_piece_move_generator(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const proto = alloc_single_piece_move_generator_slice();
    branch_insert_slices(slices[si].u.fork.fork,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_castling_intermediate_move_generator(slice_index si,
                                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const proto = alloc_castling_intermediate_move_generator_slice();
    branch_insert_slices(slices[si].u.fork.fork,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_single_move_generator(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const generator = branch_find_slice(STMoveGenerator,
                                                    slices[si].u.fork.fork);
    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_single_move_generator_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const solver_inserters[] =
{
  { STHelpAdapter,                            &insert_solvers_help_adapter                    },
  { STGeneratingMoves,                        &insert_move_generator                          },
  { STBrunnerDefenderFinder,                  &insert_single_move_generator_with_king_capture },
  { STIsardamDefenderFinder,                  &insert_single_move_generator_with_king_capture },
  { STCageCirceNonCapturingMoveFinder,        &insert_single_piece_move_generator             },
  { STCastlingIntermediateMoveLegalityTester, &insert_castling_intermediate_move_generator    },
  { STMaximummerCandidateMoveTester,          &insert_single_move_generator                   },
  { STOpponentMovesCounterFork,               &insert_single_move_generator                   }
};

enum
{
  nr_solver_inserters = sizeof solver_inserters / sizeof solver_inserters[0]
};

static void insert_other_solvers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,solver_inserters,nr_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with solving slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_solvers(slice_index root_slice)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_insert_continuation_solvers(root_slice);

  TraceStipulation(root_slice);

  if (OptFlag[solvariantes])
  {
    if (!OptFlag[nothreat])
      stip_insert_threat_solvers(root_slice);
  }
  else
    stip_insert_play_suppressors(root_slice);

  if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    stip_insert_try_solvers(root_slice);

  TraceStipulation(root_slice);

  stip_insert_setplay_solvers(root_slice);

  TraceStipulation(root_slice);

  stip_insert_find_shortest_solvers(root_slice);

  TraceStipulation(root_slice);

  stip_insert_min_length_solvers(root_slice);

  TraceStipulation(root_slice);

  insert_other_solvers(root_slice);

  stip_insert_trivial_varation_filters(root_slice);

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
