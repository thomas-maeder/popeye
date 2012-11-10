#include "solving/move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "solving/move_generator.h"
#include "solving/single_piece_move_generator.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "solving/castling.h"
#include "solving/single_move_generator.h"
#include "solving/king_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMoveGenerator);
  slices[result].u.move_generator.mode = move_generation_not_optimized;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_generator_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generation_mode = slices[si].u.move_generator.mode;
  genmove(slices[si].starter);
  result = solve(slices[si].next1,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
