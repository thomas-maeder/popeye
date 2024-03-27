#include "solving/move_generator.h"
#include "solving/temporary_hacks.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/binary.h"
#include "conditions/circe/circe.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"

#include "debugging/measure.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <string.h>
#include <stdio.h>

move_generation_elmt *curr_generation = &move_generation_stack[toppile];

move_generation_elmt move_generation_stack[toppile + 1];

numecoup current_move[maxply+1];
numecoup current_move_id[maxply+1];

piece_walk_type move_generation_current_walk;

static void write_history_recursive(ply ply)
{
  if (parent_ply[ply]>ply_retro_move)
    write_history_recursive(parent_ply[ply]);

  printf(" %u:",ply);
  WriteSquare(&output_plaintext_engine,stdout,move_generation_stack[CURRMOVE_OF_PLY(ply)].departure);
  putchar('-');
  WriteSquare(&output_plaintext_engine,stdout,move_generation_stack[CURRMOVE_OF_PLY(ply)].arrival);
}

void move_generator_write_history(void)
{
  if (nbply>ply_retro_move)
  {
//    putchar('\n');
    write_history_recursive(nbply);
//    putchar('\n');
  }
}

static slice_index const slice_rank_order[] =
{
    STGeneratingMovesForPiece,
    STFuddledMenMovesForPieceGenerator,
    STCASTMovesForPieceGenerator,
    STCASTInverseMovesForPieceGenerator,
    STTransmissionMenaceMovesForPieceGenerator,
    STPowerTransferMovesForPieceGenerator,
    STSingleBoxType3TMovesForPieceGenerator,
    STMadrasiMovesForPieceGenerator,
    STPartialParalysisMovesForPieceGenerator,
    STEiffelMovesForPieceGenerator,
    STDisparateMovesForPieceGenerator,
    STParalysingMovesForPieceGenerator,
    STPepoMovesForPieceGenerator,
    STUltraPatrolMovesForPieceGenerator,
    STCentralMovesForPieceGenerator,
    STBeamtenMovesForPieceGenerator,
    STTotalInvisibleSpecialMoveGenerator,
    STPointReflectionMovesForPieceGenerator,
    STCastlingGenerator,
    STAnnanMovesForPieceGenerator,
    STNannaMovesForPieceGenerator,
    STFaceToFaceMovesForPieceGenerator,
    STBackToBackMovesForPieceGenerator,
    STCheekToCheekMovesForPieceGenerator,
    STVaultingKingsMovesForPieceGenerator,
    STTransmutingKingsMovesForPieceGenerator,
    STSuperTransmutingKingsMovesForPieceGenerator,
    STReflectiveKingsMovesForPieceGenerator,
    STRokagogoMovesForPieceGeneratorFilter,
    STCastlingChessMovesForPieceGenerator,
    STPlatzwechselRochadeMovesForPieceGenerator,
    STMessignyMovesForPieceGenerator,
    STMoveForPieceGeneratorTwoPaths,
    STMoveForPieceGeneratorStandardPath,
    STMoveForPieceGeneratorAlternativePath,
    STMarsCirceMoveGeneratorEnforceRexInclusive,
    STMakeTakeGenerateCapturesWalkByWalk,
    STBoleroGenerateMovesWalkByWalk,
    STMarsCirceFixDeparture,
    STPhantomAvoidDuplicateMoves,
    STMarsCirceConsideringRebirth,
    STAntimarsCirceConsideringRebirth,
    STCirceDoneWithRebirth,
    STMarsCirceGenerateFromRebirthSquare,
    STPlusAdditionalCapturesForPieceGenerator,
    STMarsCirceRememberRebirth,
    STMarsCirceRememberNoRebirth,
    STMoveGeneratorRejectCaptures,
    STMoveGeneratorRejectNoncaptures,
    STMoveForPieceGeneratorPathsJoint,
    STMovesForPieceBasedOnWalkGenerator,
    STGeneratedMovesForPiece
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static void move_generation_branch_insert_slices_impl(slice_index generating,
                                                      slice_index const prototypes[],
                                                      unsigned int nr_prototypes,
                                                      slice_index base)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    prototypes,nr_prototypes,
    slice_rank_order, nr_slice_rank_order_elmts, 1,
    branch_slice_rank_order_nonrecursive,
    0,
    generating,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",generating);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(SLICE_TYPE(base),&state);
  assert(state.base_rank!=no_slice_rank);

  slice_insertion_init_traversal(&st,&state,stip_traversal_context_intro);
  circe_init_slice_insertion_traversal(&st);

  stip_traverse_structure(generating,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a move generation branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void move_generation_branch_insert_slices(slice_index si,
                                          slice_index const prototypes[],
                                          unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  move_generation_branch_insert_slices_impl(si,prototypes,nr_prototypes,si);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

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
    if (config->side==nr_sides || config->side==SLICE_STARTER(si))
    {
      slice_index const prototype = alloc_pipe(config->type);
      move_generation_branch_insert_slices_impl(si,&prototype,1,si);
      dealloc_slice(prototype);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument move generation with a slice type
 * @param identifies where to start instrumentation
 * @param side which side (pass nr_sides for both sides)
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_moves_for_piece_generation(slice_index si,
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

static void insert_separator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy_standard = alloc_proxy_slice();
    slice_index const standard = alloc_pipe(STMoveForPieceGeneratorStandardPath);

    slice_index const proxy_alternative = alloc_proxy_slice();
    slice_index const alternative = alloc_pipe(STMoveForPieceGeneratorAlternativePath);

    slice_index const generator = alloc_binary_slice(STMoveForPieceGeneratorTwoPaths,
                                                     proxy_standard,
                                                     proxy_alternative);

    pipe_link(SLICE_PREV(si),generator);

    pipe_link(proxy_standard,standard);
    pipe_link(standard,si);

    pipe_link(proxy_alternative,alternative);
    pipe_link(alternative,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the move generation machinery so that there are two paths which
 * can be adapted separately.
 * @param si root slice of solving machinery
 * @param side side for which to instrument; pass nr_sides for both sides
 * @note inserts proxy slices STMoveForPieceGeneratorStandardPath and
 *       STMoveForPieceGeneratorAlternativePath that can be used for adjusting the move
 *       generation
 */
void move_generator_instrument_for_alternative_paths(slice_index si, Side side)
{
  stip_structure_traversal st;

  solving_instrument_moves_for_piece_generation(si,
                                     side,
                                     STMoveForPieceGeneratorPathsJoint);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMoveForPieceGeneratorPathsJoint,
                                           &insert_separator);
  stip_traverse_structure(si,&st);
}

static boolean always_reject(numecoup n)
{
  return false;
}

/* Reject generated captures
 * @param si identifies the slice
 */
void move_generation_reject_captures(slice_index si)
{
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  pipe_move_generation_delegate(si);
  move_generator_filter_captures(base,&always_reject);
}

/* Reject generated non-captures
 * @param si identifies the slice
 */
void move_generation_reject_non_captures(slice_index si)
{
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  pipe_move_generation_delegate(si);
  move_generator_filter_noncaptures(base,&always_reject);
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 */
void generate_moves_for_piece_two_paths(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);
  fork_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for the current piece, but based on a different walk
 * @param si identifies the generator slices
 * @param walk the basis to be used for the move generation
 */
void generate_moves_different_walk(slice_index si, piece_walk_type walk)
{
  piece_walk_type const save_current_walk = move_generation_current_walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  move_generation_current_walk = walk;
  generate_moves_delegate(si);
  move_generation_current_walk = save_current_walk;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initiate the generation of moves for the piece occupying a specific square
 * @param sq_departure square occupied by the piece for which to generate moves
 */
void generate_moves_for_piece(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  curr_generation->departure = sq_departure;
  move_generation_current_walk = get_walk_of_piece_on_square(sq_departure);
  generate_moves_delegate(SLICE_NEXT2(temporary_hack_move_generator[trait[nbply]]));

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
    square sq_departure = square_h;
    for (j = nr_files_on_board; j>0; j--, sq_departure += dir_left)
      if (TSTFLAG(being_solved.spec[sq_departure],side))
        generate_moves_for_piece(sq_departure);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean observing_move_generator_is_in_check(slice_index si,
                                             Side side_observed)
{
  boolean result;
  square const save_generation_departure = curr_generation->departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_observed);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);

  genmove();

  result = pipe_is_in_check_recursive_delegate(si,side_observed);

  finply();

  curr_generation->departure = save_generation_departure;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void move_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));
  genmove();
  pipe_solve_delegate(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  boolean is_insertion_skipped;
  move_generation_instrumentation_callback *callback;
  void *param;
} move_generator_insertion_status;

static void insert_move_generator(slice_index si,
                                  stip_structure_traversal *st,
                                  void *param)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_move_generator_slice();
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_generating_moves(slice_index si, stip_structure_traversal *st)
{
  move_generator_insertion_status *status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (status->is_insertion_skipped)
    status->is_insertion_skipped = false;
  else
    (*status->callback)(si,st,status->param);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void skip_insertion(slice_index si, stip_structure_traversal *st)
{
  move_generator_insertion_status *status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  assert(!status->is_insertion_skipped);
  status->is_insertion_skipped = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const solver_inserters[] =
{
  { STGeneratingMoves,    &instrument_generating_moves },
  { STSkipMoveGeneration, &skip_insertion        }
};

enum
{
  nr_solver_inserters = sizeof solver_inserters / sizeof solver_inserters[0]
};

/* Instrument the solving machinery
 * @param si identifies root the solving machinery
 * @param callback called back at each STGeneratingMoves slice which is not
 *                 deactivated by a STSkipMoveGeneration slice
 */
void solving_instrument_move_generation(slice_index si,
                                        move_generation_instrumentation_callback *callback,
                                        void *param)
{
  stip_structure_traversal st;
  move_generator_insertion_status status = { false, callback, param };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&status);
  stip_structure_traversal_override(&st,solver_inserters,nr_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with move generator slices
 * @param si identifies root the solving machinery
 */
void solving_insert_move_generators(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,&insert_move_generator,0);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Only keep generated moves that fulfill some criterion
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_moves(numecoup start,
                                 move_filter_criterion_type criterion)
{
  numecoup i;
  numecoup new_top = start;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
    if ((*criterion)(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Only keep generated captures that fulfill some criterion; non-captures are all kept
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_captures(numecoup start,
                                    move_filter_criterion_type criterion)
{
  numecoup i;
  numecoup new_top = start;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
    if (is_no_capture(move_generation_stack[i].capture)
        || (*criterion)(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Only keep generated non-captures that fulfill some criterion; captures are all kept
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_noncaptures(numecoup start,
                                       move_filter_criterion_type criterion)
{
  numecoup i;
  numecoup new_top = start;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
    if (!is_no_capture(move_generation_stack[i].capture)
        || (*criterion)(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Invert the order of the moves generated for a ply
 * @param ply the ply
 */
void move_generator_invert_move_order(ply ply)
{
  unsigned int const nr_moves = current_move[ply]-current_move[ply-1];
  numecoup hi = CURRMOVE_OF_PLY(ply);
  numecoup low = hi-nr_moves+1;

  while (low<hi)
  {
    move_generation_elmt const temp = move_generation_stack[low];
    move_generation_stack[low] = move_generation_stack[hi];
    move_generation_stack[hi] = temp;

    ++low;
    --hi;
  }
}

void pop_move(void)
{
  assert(current_move[nbply]>0);
  --current_move[nbply];
}

DEFINE_COUNTER(add_to_move_generation_stack)

void push_move_no_capture(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  TraceSquare(curr_generation->departure);
  TraceSquare(curr_generation->arrival);
  TraceEOL();

  assert(is_square_empty(curr_generation->arrival));
  curr_generation->capture = no_capture;
  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void push_move_regular_capture(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  TraceSquare(curr_generation->departure);
  TraceSquare(curr_generation->arrival);
  TraceEOL();

  curr_generation->capture = curr_generation->arrival;
  assert(!is_square_empty(curr_generation->capture));
  assert(TSTFLAG(being_solved.spec[curr_generation->capture],advers(trait[nbply])));
  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

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
  TraceEOL();

  curr_generation->capture = sq_capture;
  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

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
  TraceEOL();

  curr_generation->capture = sq_special;
  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean is_null_move(numecoup curr)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  result = (move_generation_stack[curr].departure==nullsquare
            && move_generation_stack[curr].arrival==nullsquare
            && move_generation_stack[curr].capture==no_capture);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void push_null_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  curr_generation->departure = nullsquare;
  curr_generation->arrival = nullsquare;
  curr_generation->capture = no_capture;

  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void push_move_copy(numecoup original)
{
  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = move_generation_stack[original];
}

void push_observation_target(square sq_target)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void replace_observation_target(square sq_target)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void pop_all(void)
{
  current_move[nbply] = current_move[nbply-1];
}

typedef unsigned int mark_type;

static mark_type square_marks[square_h8+1] = { 0 };
static mark_type current_mark = 0;

static boolean is_not_duplicate(numecoup n)
{
  square const sq_arrival = move_generation_stack[n].arrival;
  if (square_marks[sq_arrival]==current_mark)
    return false;
  else
  {
    square_marks[sq_arrival] = current_mark;
    return true;
  }
}

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

  move_generator_filter_moves(last_move_of_prev_piece,&is_not_duplicate);
}

/* Priorise a move in the move generation stack
 * @param priorised index in the move generation stack of the move to be
 *                  priorised
 */
void move_generator_priorise(numecoup priorised)
{
  /* we move the priorised move one position too far and then shift back one
   * move too many */
  numecoup const one_too_far = CURRMOVE_OF_PLY(nbply)+1;
  move_generation_stack[one_too_far] = move_generation_stack[priorised];
  memmove(&move_generation_stack[priorised],
          &move_generation_stack[priorised+1],
          (one_too_far-priorised)*sizeof move_generation_stack[0]);
}
