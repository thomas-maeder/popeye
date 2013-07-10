#include "solving/move_generator.h"
#include "solving/move_generator.h"
#include "solving/single_piece_move_generator.h"
#include "solving/castling.h"
#include "solving/single_move_generator.h"
#include "solving/king_move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void genmove(Side side)
{
  unsigned int i;
  square square_a = side==White ? square_a1 : square_h8;
  numvec const next_row = side==White ? onerow : -onerow;
  numvec const next_file = side==White ? dir_right : dir_left;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  trait[nbply]= side;

  /* Don't try to "optimize" by hand. The double-loop is tested as
     the fastest way to compute (due to compiler-optimizations !)
     V3.14  NG
  */
  for (i = nr_rows_on_board; i>0; i--, square_a += next_row)
  {
    square j;
    square z = square_a;
    for (j = nr_files_on_board; j>0; j--, z += next_file)
      if (TSTFLAG(spec[z],side))
        generate_moves_for_piece(side,z,get_walk_of_piece_on_square(z));
  }

  if (side==Black && CondFlag[schwarzschacher])
    add_to_move_generation_stack(nullsquare, nullsquare, nullsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type move_generator_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();
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

static void insert_single_move_generator(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proto = alloc_single_move_generator_slice();
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
  { STGeneratingMoves,                        &insert_move_generator                       },
  { STBrunnerDefenderFinder,                  &insert_single_move_generator                },
  { STKingCaptureLegalityTester,              &insert_single_move_generator                },
  { STCageCirceNonCapturingMoveFinder,        &insert_single_piece_move_generator          },
  { STTakeMakeCirceCollectRebirthSquaresFork, &insert_single_piece_move_generator          },
  { STCastlingIntermediateMoveLegalityTester, &insert_castling_intermediate_move_generator },
  { STOpponentMovesCounterFork,               &substitute_single_move_generator            }
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

void move_generator_filter_moves(move_filter_criterion_type criterion)
{
  numecoup i;
  numecoup new_top = current_move[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]+1; i<=current_move[nbply]; ++i)
  {
    square const sq_departure = move_generation_stack[i].departure;
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_capture = move_generation_stack[i].capture;

    if ((*criterion)(sq_departure,sq_arrival,sq_capture))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }
  }

  current_move[nbply] = new_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
