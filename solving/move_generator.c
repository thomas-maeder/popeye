#include "solving/move_generator.h"
#include "conditions/annan.h"
#include "conditions/beamten.h"
#include "conditions/central.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/facetoface.h"
#include "conditions/madrasi.h"
#include "conditions/disparate.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/messigny.h"
#include "conditions/patrol.h"
#include "conditions/phantom.h"
#include "conditions/singlebox/type3.h"
#include "conditions/sting.h"
#include "conditions/castling_chess.h"
#include "conditions/exchange_castling.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/vaulting_kings.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/walks/generate_moves.h"
#include "solving/single_piece_move_generator.h"
#include "solving/castling.h"
#include "solving/king_move_generator.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/measure.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>
#include <string.h>

move_generation_elmt *curr_generation = &move_generation_stack[toppile];

move_generation_elmt move_generation_stack[toppile + 1];

numecoup current_move[maxply+1];
numecoup current_move_id[maxply+1];

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
    STSuperTransmutingKingsMovesForPieceGenerator,
    STReflectiveKingsMovesForPieceGenerator,
    STStingMovesForPieceGenerator,
    STCastlingChessMovesForPieceGenerator,
    STPlatzwechselRochadeMovesForPieceGenerator,
    STCastlingGenerator,
    STMessignyMovesForPieceGenerator,
    STAnnanMovesForPieceGenerator,
    STFaceToFaceMovesForPieceGenerator,
    STBackToBackMovesForPieceGenerator,
    STCheekToCheekMovesForPieceGenerator,
    STMovesForPieceBasedOnWalkGenerator,
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
 * @param p walk to be used for generating
 */
void generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");

  switch (slices[si].type)
  {
    case STSingleBoxType3TMovesForPieceGenerator:
      singleboxtype3_generate_moves_for_piece(si,p);
      break;

    case STMadrasiMovesForPieceGenerator:
      madrasi_generate_moves_for_piece(si,p);
      break;

    case STEiffelMovesForPieceGenerator:
      eiffel_generate_moves_for_piece(si,p);
      break;

    case STDisparateMovesForPieceGenerator:
      disparate_generate_moves_for_piece(si,p);
      break;

    case STParalysingMovesForPieceGenerator:
      paralysing_generate_moves_for_piece(si,p);
      break;

    case STUltraPatrolMovesForPieceGenerator:
      ultrapatrol_generate_moves_for_piece(si,p);
      break;

    case STCentralMovesForPieceGenerator:
      central_generate_moves_for_piece(si,p);
      break;

    case STBeamtenMovesForPieceGenerator:
      beamten_generate_moves_for_piece(si,p);
      break;

    case STPhantomMovesForPieceGenerator:
      phantom_generate_moves_for_piece(si,p);
      break;

    case STPlusMovesForPieceGenerator:
      plus_generate_moves_for_piece(si,p);
      break;

    case STMarsCirceMovesForPieceGenerator:
      marscirce_generate_moves_for_piece(si,p);
      break;

    case STAntiMarsCirceMovesForPieceGenerator:
      antimars_generate_moves_for_piece(si,p);
      break;

    case STVaultingKingsMovesForPieceGenerator:
      vaulting_kings_generate_moves_for_piece(si,p);
      break;

    case STTransmutingKingsMovesForPieceGenerator:
      transmuting_kings_generate_moves_for_piece(si,p);
      break;

    case STSuperTransmutingKingsMovesForPieceGenerator:
      supertransmuting_kings_generate_moves_for_piece(si,p);
      break;

    case STReflectiveKingsMovesForPieceGenerator:
      reflective_kings_generate_moves_for_piece(si,p);
      break;

    case STStingMovesForPieceGenerator:
      sting_generate_moves_for_piece(si,p);
      break;

    case STCastlingChessMovesForPieceGenerator:
      castlingchess_generate_moves_for_piece(si,p);
      break;

    case STPlatzwechselRochadeMovesForPieceGenerator:
      exchange_castling_generate_moves_for_piece(si,p);
      break;

    case STCastlingGenerator:
      castling_generator_generate_castling(si,p);
      break;

    case STMessignyMovesForPieceGenerator:
      messigny_generate_moves_for_piece(si,p);
      break;

    case STAnnanMovesForPieceGenerator:
      annan_generate_moves_for_piece(si,p);
      break;

    case STFaceToFaceMovesForPieceGenerator:
      facetoface_generate_moves_for_piece(si,p);
      break;

    case STBackToBackMovesForPieceGenerator:
      backtoback_generate_moves_for_piece(si,p);
      break;

    case STCheekToCheekMovesForPieceGenerator:
      cheektocheek_generate_moves_for_piece(si,p);
      break;

    case STMovesForPieceBasedOnWalkGenerator:
      generate_moves_for_piece_based_on_walk(p);
      generate_moves_for_piece(slices[si].next1,p);
      break;

    case STTrue:
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

static void genmove(void)
{
  unsigned int i;
  square square_h = square_h8;
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    unsigned int j;
    curr_generation->departure = square_h;
    for (j = nr_files_on_board; j>0; j--)
    {
      if (TSTFLAG(spec[curr_generation->departure],side))
      {
        TraceSquare(curr_generation->departure);TraceText("\n");
        generate_moves_for_piece(slices[temporary_hack_move_generator[side]].next2,
                                 get_walk_of_piece_on_square(curr_generation->departure));
      }
      curr_generation->departure += dir_left;
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

  nextply(slices[si].starter);
  genmove();
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

static void remove_move_generator(slice_index si, stip_structure_traversal *st)
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
    pipe_remove(generator);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const solver_inserters[] =
{
  { STGeneratingMoves,                        &insert_move_generator                       },
  { STBrunnerDefenderFinder,                  &stip_traverse_structure_children_pipe       },
  { STKingCaptureLegalityTester,              &stip_traverse_structure_children_pipe       },
  { STMoveLegalityTester,                     &stip_traverse_structure_children_pipe       },
  { STCageCirceNonCapturingMoveFinder,        &insert_single_piece_move_generator          },
  { STTakeMakeCirceCollectRebirthSquaresFork, &insert_single_piece_move_generator          },
  { STCastlingIntermediateMoveLegalityTester, &insert_castling_intermediate_move_generator },
  { STOpponentMovesCounterFork,               &remove_move_generator                       }
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
  numecoup new_top = current_move[nbply-1]-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]; i<current_move[nbply]; ++i)
    if ((*criterion)(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  current_move[nbply] = new_top+1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_generator_filter_captures(move_filter_criterion_type criterion)
{
  numecoup i;
  numecoup new_top = current_move[nbply-1]-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]; i<current_move[nbply]; ++i)
    if (is_square_empty(move_generation_stack[i].capture)
        || (*criterion)(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  current_move[nbply] = new_top+1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

DEFINE_COUNTER(add_to_move_generation_stack)

void push_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  TraceSquare(curr_generation->departure);
  TraceSquare(curr_generation->arrival);
  TraceText("\n");

  curr_generation->capture = curr_generation->arrival;
  move_generation_stack[current_move[nbply]] = *curr_generation;
  move_generation_stack[current_move[nbply]].id = current_move_id[nbply];
  ++current_move[nbply];
  ++current_move_id[nbply];
  TraceValue("%u\n",current_move[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void push_move_capture_extra(square sq_capture)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  TraceSquare(curr_generation->departure);
  TraceSquare(curr_generation->arrival);
  TraceText("\n");

  curr_generation->capture = sq_capture;
  move_generation_stack[current_move[nbply]] = *curr_generation;
  move_generation_stack[current_move[nbply]].id = current_move_id[nbply];
  ++current_move[nbply];
  ++current_move_id[nbply];
  TraceValue("%u\n",current_move[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void push_special_move(square sq_special)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",sq_special);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  TraceSquare(curr_generation->departure);
  TraceSquare(curr_generation->arrival);
  TraceText("\n");

  curr_generation->capture = sq_special;
  move_generation_stack[current_move[nbply]] = *curr_generation;
  move_generation_stack[current_move[nbply]].id = current_move_id[nbply];
  ++current_move[nbply];
  ++current_move_id[nbply];
  TraceValue("%u\n",current_move[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef unsigned int mark_type;

static mark_type square_marks[square_h8+1] = { 0 };
static mark_type current_mark = 0;

/* Remove duplicate moves generated for a single piece.
 * @param last_move_of_prev_piece index of last move of previous piece
 */
void remove_duplicate_moves_of_single_piece(numecoup last_move_of_prev_piece)
{
  if (current_mark==UINT_MAX)
  {
    square i;
    for (i = square_a1; i!=square_h8; ++i)
      square_marks[i] = 0;

    current_mark = 1;
  }
  else
    ++current_mark;

  {
    numecoup curr_move;
    numecoup last_unique_move = last_move_of_prev_piece;
    for (curr_move = last_move_of_prev_piece+1;
         curr_move<current_move[nbply];
         ++curr_move)
    {
      square const sq_arrival = move_generation_stack[curr_move].arrival;
      if (square_marks[sq_arrival]==current_mark)
      {
        // skip over duplicate move
      }
      else
      {
        ++last_unique_move;
        move_generation_stack[last_unique_move] = move_generation_stack[curr_move];
        square_marks[sq_arrival] = current_mark;
      }
    }

    current_move[nbply] = last_unique_move+1;
  }
}

/* Priorise a move in the move generation stack
 * @param priorised index in the move generation stack of the move to be
 *                  priorised
 */
void move_generator_priorise(numecoup priorised)
{
  /* we move the priorised move one position too far and then shift back one
   * move too many */
  move_generation_stack[current_move[nbply]] = move_generation_stack[priorised];
  memmove(&move_generation_stack[priorised],
          &move_generation_stack[priorised+1],
          (current_move[nbply]-priorised)
          * sizeof move_generation_stack[priorised]);
}
