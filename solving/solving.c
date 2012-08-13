#include "solving/solving.h"
#include "pydata.h"
#include "stipulation/fork.h"
#include "stipulation/proxy.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/if_then_else.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_generator.h"
#include "solving/for_each_move.h"
#include "solving/find_shortest.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/setplay.h"
#include "solving/single_piece_move_generator.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "solving/castling.h"
#include "solving/single_move_generator.h"
#include "solving/king_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

enum
{
  max_nr_additional_visitors = 10
};

static structure_traversers_visitor additional_spin_off_visitor[max_nr_additional_visitors];

static unsigned int nr_additional_visitors;

/* Register a call-back for the next run of stip_spin_off_testers()
 * @param type slice type for which to call back visitor
 * @param visitor address to function to invoke for each visited slice of type type
 */
void register_spin_off_testers_visitor(slice_type type,
                                       stip_structure_visitor visitor)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_additional_visitors; ++i)
    if (type==additional_spin_off_visitor[i].type)
    {
      assert(visitor==additional_spin_off_visitor[i].visitor);
      break;
    }

  if (i==nr_additional_visitors)
  {
    assert(nr_additional_visitors<max_nr_additional_visitors);
    additional_spin_off_visitor[nr_additional_visitors].type = type;
    additional_spin_off_visitor[nr_additional_visitors].visitor = visitor;
    ++nr_additional_visitors;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_spin_off_visitors(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_additional_visitors = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_spin_off_traversal(stip_structure_traversal *st)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_additional_visitors; ++i)
    stip_structure_traversal_override_single(st,
                                             additional_spin_off_visitor[i].type,
                                             additional_spin_off_visitor[i].visitor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_spinning_off_end_of_root(slice_index si,
                                           stip_structure_traversal *st)
{
  boolean * const spinning_off = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*spinning_off)
    stip_spin_off_testers_pipe(si,st);
  else if (st->context==stip_traversal_context_attack)
  {
    /* we are solving something like #3 option postkey
     * start spinning off testers at the loop entry
     */
    *spinning_off = true;
    stip_spin_off_testers_pipe(si,st);
    *spinning_off = false;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_spinning_off_end_of_intro(slice_index si,
                                            stip_structure_traversal *st)
{
  boolean * const spinning_off = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*spinning_off)
    stip_spin_off_testers_pipe(si,st);
  else if (st->context==stip_traversal_context_attack
           || st->context==stip_traversal_context_defense)
  {
    /* we are solving a nested battle play branch */
    *spinning_off = true;
    stip_spin_off_testers_pipe(si,st);
    *spinning_off = false;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 */
void stip_spin_off_testers(slice_index si)
{
  stip_structure_traversal st;
  boolean spinning_off = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&spinning_off);

  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &stip_spin_off_testers_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &stip_spin_off_testers_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &stip_spin_off_testers_fork);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_leaf,
                                                 &stip_spin_off_testers_leaf);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &stip_spin_off_testers_binary);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_spin_off_testers_testing_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_spin_off_testers_conditional_pipe);

  stip_structure_traversal_override_single(&st,STEndOfRoot,&start_spinning_off_end_of_root);
  stip_structure_traversal_override_single(&st,STEndOfIntro,&start_spinning_off_end_of_intro);

  stip_structure_traversal_override_single(&st,STAnd,&stip_spin_off_testers_and);
  stip_structure_traversal_override_single(&st,STPlaySuppressor,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STIfThenElse,&stip_spin_off_testers_if_then_else);

  instrument_spin_off_traversal(&st);

  stip_traverse_structure(si,&st);

  forget_spin_off_visitors();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_ready_for_move_in_loop(slice_index ready_root)
{
  slice_index result = ready_root;
  do
  {
    result = branch_find_slice(STReadyForHelpMove,
                               result,
                               stip_traversal_context_help);
  } while ((slices[result].u.branch.length-slack_length)%2
           !=(slices[ready_root].u.branch.length-slack_length)%2);
  return result;
}

static void insert_solvers_help_adapter(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length+2>=min_length)
  {
    if (st->level==structure_traversal_level_nested)
    {
      if (st->context==stip_traversal_context_intro)
      {
        slice_index const prototype = alloc_find_shortest_slice(length,min_length);
        branch_insert_slices(si,&prototype,1);
      }
    }
    else /* root or set play */
    {
      if (!OptFlag[restart] && length>=slack_length+2)
      {
        {
          slice_index const prototype =
              alloc_find_by_increasing_length_slice(length,min_length);
          branch_insert_slices(si,&prototype,1);
        }
        {
          slice_index const ready_root = branch_find_slice(STReadyForHelpMove,
                                                           si,
                                                           st->context);
          slice_index const ready_loop = find_ready_for_move_in_loop(ready_root);
          slice_index const proxy_root = alloc_proxy_slice();
          slice_index const proxy_loop = alloc_proxy_slice();
          pipe_set_successor(proxy_loop,ready_loop);
          pipe_link(slices[ready_root].prev,
                    alloc_fork_on_remaining_slice(proxy_root,proxy_loop,
                                                  length-1-slack_length));
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

  stip_traverse_structure_children_pipe(si,st);

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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proto = alloc_single_move_generator_with_king_capture_slice();
    branch_insert_slices(slices[si].next2,&proto,1);
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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proto = alloc_single_piece_move_generator_slice();
    branch_insert_slices(slices[si].next2,&proto,1);
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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proto = alloc_castling_intermediate_move_generator_slice();
    branch_insert_slices(slices[si].next2,&proto,1);
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
                                                    slices[si].next2,
                                                    stip_traversal_context_intro);
    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_single_move_generator_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const solver_inserters[] =
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

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
