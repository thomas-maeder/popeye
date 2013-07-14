#include "solving/move_generator.h"
#include "conditions/beamten.h"
#include "conditions/central.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/madrasi.h"
#include "conditions/disparate.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/patrol.h"
#include "conditions/phantom.h"
#include "conditions/singlebox/type3.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "solving/single_piece_move_generator.h"
#include "solving/castling.h"
#include "solving/single_move_generator.h"
#include "solving/king_move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

#include <assert.h>

static slice_index const slice_rank_order[] =
{
    STGeneratingMovesForPiece,
    STSingleBoxType3TMovesForPieceGenerator,
    STMadrasiMovesForPieceGenerator,
    STEiffelMovesForPieceGenerator,
    STDisparateMovesForPieceGenerator,
    STParalysingMovesForPieceGenerator,
    STUltraPatrolMovesForPieceGenerator,
    STCentralMovesForPieceGenerator,
    STBeamtenMovesForPieceGenerator,
    STPhantomMovesForPieceGenerator,
    STPlusMovesForPieceGenerator,
    STMarsCirceMovesForPieceGenerator,
    STAntiMarsCirceMovesForPieceGenerator,
    STVaultingKingsMovesForPieceGenerator,
    STTransmutingKingsMovesForPieceGenerator,
    STReflectiveKingsMovesForPieceGenerator,
    STMovesForPieceGeneratorOrtho,
    STTrue
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static void insert_slice(slice_index testing, slice_type type)
{
  slice_index const prototype = alloc_pipe(type);
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    &prototype,1,
    slice_rank_order, nr_slice_rank_order_elmts,
    branch_slice_rank_order_nonrecursive,
    0,
    testing,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(slices[testing].type,&state);
  assert(state.base_rank!=no_slice_rank);
  init_slice_insertion_traversal(&st,&state,stip_traversal_context_intro);
  stip_traverse_structure_children_pipe(testing,&st);
  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    Side side;
    slice_type type;
} insertion_configuration;

static void instrument_generating(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    insertion_configuration const * config = st->param;
    if (config->side==nr_sides || config->side==slices[si].starter)
      insert_slice(si,config->type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument move generation with a slice type
 * @param identifies where to start instrumentation
 * @param side which side (pass nr_sides for both sides)
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_move_generation(slice_index si,
                                        Side side,
                                        slice_type type)
{
  stip_structure_traversal st;
  insertion_configuration config = { side, type };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&config);
  stip_structure_traversal_override_single(&st,
                                           STGeneratingMovesForPiece,
                                           &instrument_generating);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void generate_moves_for_piece(slice_index si, square sq_departure, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STSingleBoxType3TMovesForPieceGenerator:
      singleboxtype3_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STMadrasiMovesForPieceGenerator:
      madrasi_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STEiffelMovesForPieceGenerator:
      eiffel_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STDisparateMovesForPieceGenerator:
      disparate_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STParalysingMovesForPieceGenerator:
      paralysing_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STUltraPatrolMovesForPieceGenerator:
      ultrapatrol_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STCentralMovesForPieceGenerator:
      central_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STBeamtenMovesForPieceGenerator:
      beamten_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STPhantomMovesForPieceGenerator:
      phantom_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STPlusMovesForPieceGenerator:
      plus_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STMarsCirceMovesForPieceGenerator:
      marscirce_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STAntiMarsCirceMovesForPieceGenerator:
      antimars_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STVaultingKingsMovesForPieceGenerator:
      vaulting_kings_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STTransmutingKingsMovesForPieceGenerator:
      transmuting_kings_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STReflectiveKingsMovesForPieceGenerator:
      reflective_kings_generate_moves_for_piece(si,sq_departure,p);
      break;

    case STMovesForPieceGeneratorOrtho:
      generate_moves_for_piece_ortho(sq_departure,p);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


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
  square square_h = square_h8;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  trait[nbply]= side;

  for (i = nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    unsigned int j;
    square sq_departure = square_h;
    for (j = nr_files_on_board; j>0; j--)
    {
      if (TSTFLAG(spec[sq_departure],side))
        generate_moves_for_piece(slices[temporary_hack_move_generator[side]].next2,
                                 sq_departure,
                                 get_walk_of_piece_on_square(sq_departure));
      sq_departure += dir_left;
    }
  }

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
