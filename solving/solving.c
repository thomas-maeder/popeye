#include "solving/solving.h"
#include "pydata.h"
#include "stipulation/fork.h"
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

static void insert_move_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_move_generator_slice();
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

static void substitute_single_move_generator(slice_index si,
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
  { STGeneratingMoves,                        &insert_move_generator                          },
  { STBrunnerDefenderFinder,                  &insert_single_move_generator_with_king_capture },
  { STKingCaptureLegalityTester,              &insert_single_move_generator_with_king_capture },
  { STCageCirceNonCapturingMoveFinder,        &insert_single_piece_move_generator             },
  { STCastlingIntermediateMoveLegalityTester, &insert_castling_intermediate_move_generator    },
  { STMaximummerCandidateMoveTester,          &substitute_single_move_generator               },
  { STOpponentMovesCounterFork,               &substitute_single_move_generator               }
};

enum
{
  nr_solver_inserters = sizeof solver_inserters / sizeof solver_inserters[0]
};

/* Instrument a stipulation with move generator slices
 * @param si root of branch to be instrumented
 */
void stip_insert_move_generators(slice_index si)
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
