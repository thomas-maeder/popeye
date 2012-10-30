#include "solving/solving.h"
#include "pydata.h"
#include "stipulation/fork.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/if_then_else.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
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

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 */
void stip_spin_off_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);

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

  stip_structure_traversal_override_single(&st,STTemporaryHackFork,&stip_spin_off_testers_pipe_skip);

  stip_structure_traversal_override_single(&st,STPlaySuppressor,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STIfThenElse,&stip_spin_off_testers_if_then_else);

  stip_traverse_structure(si,&st);

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
