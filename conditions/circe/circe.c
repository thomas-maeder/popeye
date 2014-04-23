#include "conditions/circe/circe.h"
#include "conditions/circe/april.h"
#include "conditions/circe/cage.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/double_agents.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/glasgow.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/super.h"
#include "conditions/circe/volage.h"
#include "pieces/pieces.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"

#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Order in which the slice types for Circe execution appear
 */
static slice_type const circe_slice_rank_order[] =
{
    STCirceConsideringRebirth,
    STAnticirceConsideringRebirth,
    STGenevaConsideringRebirth,
    STMarsCirceConsideringRebirth,
    STCirceInitialiseFromCurrentMove,
    STCirceInitialiseFromLastMove,
    STCirceKamikazeCaptureFork,
    STCirceCaptureFork,
    STCirceParrainThreatFork,
    STCirceCageNoCageFork,
    STSuperCirceNoRebirthFork,
    STCirceDeterminingRebirth,
    STCirceInitialiseRebornFromCapturee,
    STAnticirceInitialiseRebornFromCapturer,
    STMarscirceInitialiseRebornFromGenerated,
    STGenevaInitialiseRebornFromCapturer,
    STCirceCloneDetermineRebornWalk,
    STAntiCloneCirceDetermineRebornWalk,
    STChameleonCirceAdaptRebornWalk,
    STCirceEinsteinAdjustRebornWalk,
    STCirceReversalEinsteinAdjustRebornWalk,
    STCirceInitialiseRelevantFromReborn,
    STCirceCouscousMakeCapturerRelevant,
    STAnticirceCouscousMakeCaptureeRelevant,
    STCircePreventKingRebirth,
    STAprilCaptureFork,
    STMirrorCirceOverrideRelevantSide,
    STRankCirceOverrideRelevantSide,
    STMagicSquareType2AnticirceRelevantSideAdapter,
    STCirceDetermineRebirthSquare,
    STFileCirceDetermineRebirthSquare,
    STAntipodesCirceDetermineRebirthSquare,
    STCirceParrainDetermineRebirth,
    STCirceContraparrainDetermineRebirth,
    STSymmetryCirceDetermineRebirthSquare,
    STPWCDetermineRebirthSquare,
    STDiagramCirceDetermineRebirthSquare,
    STTakeMakeCirceDetermineRebirthSquares,
    STSuperCirceDetermineRebirthSquare,
    STCirceDiametralAdjustRebirthSquare,
    STCirceVerticalMirrorAdjustRebirthSquare,
    STRankCirceProjectRebirthSquare,
    STCirceFrischaufAdjustRebirthSquare,
    STCirceGlasgowAdjustRebirthSquare,
    STCirceDeterminedRebirth,
    STContactGridAvoidCirceRebirth,
    STGenevaStopCaptureFromRebirthSquare,
    STAnticirceCheylanFilter,
    STAnticirceRemoveCapturer,
    STMarscirceRemoveCapturer,
    STCirceTestRebirthSquareEmpty,
    STCirceRebirthOnNonEmptySquare,
    STSupercircePreventRebirthOnNonEmptySquare,
    STCirceAssassinAssassinate,
    STCirceParachuteRemember,
    STCirceVolcanicRemember,
    STCircePlacingReborn,
    STCircePlaceReborn,
    STBeforePawnPromotion, /* reborn pawn */
    STLandingAfterPawnPromotion,
    STCirceDoubleAgentsAdaptRebornSide,
    STCirceVolageRecolorer,
    STCirceCageCageTester,
    STCirceVolcanicSwapper,
    STCirceRebirthAvoided,
    STCirceDoneWithRebirth
};

enum
{
  nr_circe_slice_rank_order_elmts = sizeof circe_slice_rank_order / sizeof circe_slice_rank_order[0],
  nr_circe_exit_slice_types = 1
};

/* Determine whether a slice type contributes to the execution of moves
 * @param type slice type
 * @return true iff type is a slice type that contributes to the execution of moves
 */
boolean is_circe_slice_type(slice_type type)
{
  unsigned int i;
  for (i = 0; i!=nr_circe_slice_rank_order_elmts-nr_circe_exit_slice_types; ++i)
    if (type==circe_slice_rank_order[i])
      return true;

  return false;
}

/* Start inserting according to the slice type order for Circe execution
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
static void start_insertion_according_to_circe_order(slice_index si,
                                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch_insert_slices_factored_order(si,
                                      st,
                                      circe_slice_rank_order,
                                      nr_circe_slice_rank_order_elmts,
                                      STCirceDoneWithRebirth);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to start slice insertion within the sequence of slices that deal with
 * Circe.
 * @param base_type type relevant for determining the position of the slices to
 *                  be inserted
 * @param si identifies the slice where to actually start the insertion traversal
 * @param st address of the structure representing the insertion traversal
 * @return true iff base_type effectively is a type from the Circe slices sequence
 */
boolean circe_start_insertion(slice_type base_type,
                              slice_index si,
                              stip_structure_traversal *st)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,base_type,"");
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (promotion_start_insertion(base_type,si,st))
    result = true;
  else if (is_circe_slice_type(base_type))
  {
    start_insertion_according_to_circe_order(si,st);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_visit_circe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type const * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (!branch_insert_before(si,rank,st))
      start_insertion_according_to_circe_order(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal for inserting slices
 * into the Circe execution sequence
 * @param st address of structure representing the traversal
 */
void circe_init_slice_insertion_traversal(stip_structure_traversal *st)
{
  stip_structure_traversal_override_single(st,STCirceConsideringRebirth,&insert_visit_circe);
  stip_structure_traversal_override_single(st,STAnticirceConsideringRebirth,&insert_visit_circe);
}

circe_variant_type circe_variant;

circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];
circe_rebirth_context_index circe_rebirth_context_stack_pointer = 0;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void circe_reset_variant(circe_variant_type *variant)
{
  variant->is_promotion_possible = false;
  variant->rebirth_reason = move_effect_reason_rebirth_no_choice;
  variant->relevant_side_overrider = circe_relevant_side_overrider_none;
  variant->rebirth_square_adapter = circe_rebirth_square_adapter_none;
  variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_default;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_relaxed;
  variant->reborn_walk_adapter = circe_reborn_walk_adapter_none;
  variant->is_turncoat = false;
  variant->do_place_reborn = true;
  variant->default_relevant_piece = circe_relevant_piece_capturee;
  variant->actual_relevant_piece = circe_relevant_piece_capturee;
  variant->relevant_capture = circe_relevant_capture_thismove;
  variant->determine_rebirth_square = circe_determine_rebirth_square_from_pas;
  variant->anticirce_type = anticirce_type_count;
  variant->is_rex_inclusive = false;
  variant->is_restricted_to_walks = false;
}

/* Find the Circe rebirth effect in the current move
 * @return the index of the rebirth effect
 *         move_effect_journal_base[nbply+1] if there is none
 */
move_effect_journal_index_type circe_find_current_rebirth(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = top-1; result>=base+move_effect_journal_index_offset_other_effects; --result)
    if (move_effect_journal[result].type==move_effect_piece_readdition
        && (move_effect_journal[result].reason==move_effect_reason_rebirth_no_choice
            || move_effect_journal[result].reason==move_effect_reason_rebirth_choice))
      break;

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
void circe_determine_rebirth_square_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_square = rennormal(context->relevant_walk,
                                      context->relevant_spec,
                                      context->relevant_square,
                                      context->relevant_side);

  pipe_solve_delegate(si);

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
void circe_place_reborn_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(context->reborn_walk!=Empty);
  assert(is_square_empty(context->rebirth_square));

  move_effect_journal_do_piece_readdition(slices[si].u.circe_handler.variant->rebirth_reason,
                                          context->rebirth_square,
                                          context->reborn_walk,
                                          context->reborn_spec);
  pipe_solve_delegate(si);

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
void circe_done_with_rebirth(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++circe_rebirth_context_stack_pointer;
  pipe_solve_delegate(si);

  --circe_rebirth_context_stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

square renfile(piece_walk_type p_captured, square sq_capture, Side capturer)
{
  int col = sq_capture % onerow;
  square result;

  TraceFunctionEntry(__func__);
  TraceWalk(p_captured);
  TraceSquare(sq_capture);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (capturer==Black)
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      result = col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

square renantipoden_polymorphic(piece_walk_type p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  unsigned int const row = sq_capture/onerow - nr_of_slack_rows_below_board;
  unsigned int const file = sq_capture%onerow - nr_of_slack_files_left_of_board;

  square result = sq_capture;

  if (row<nr_rows_on_board/2)
    result += nr_rows_on_board/2*dir_up;
  else
    result += nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    result += nr_files_on_board/2*dir_right;
  else
    result += nr_files_on_board/2*dir_left;

  return result;
}

square rennormal_polymorphic(piece_walk_type pnam_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  return rennormal(pnam_captured,p_captured_spec,sq_capture,capturer);
}

square rennormal(piece_walk_type pnam_captured, Flags p_captured_spec,
                  square sq_capture,
                  Side capturer)
{
  square  Result;
  unsigned int col = sq_capture % onerow;
  unsigned int const ran = sq_capture / onerow;
  Side const cou = (ran&1) != (col&1) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceWalk(pnam_captured);
  TraceSquare(sq_capture);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (capturer == Black)
  {
    if (is_pawn(pnam_captured))
      Result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else if (pnam_captured==standard_walks[Knight])
      Result = cou == White ? square_b1 : square_g1;
    else if (pnam_captured==standard_walks[Rook])
      Result = cou == White ? square_h1 : square_a1;
    else if (pnam_captured==standard_walks[Queen])
      Result = square_d1;
    else if (pnam_captured==standard_walks[Bishop])
      Result = cou == White ? square_f1 : square_c1;
    else if (pnam_captured==standard_walks[King])
      Result = square_e1;
    else
      Result = col + onerow*(nr_of_slack_rows_below_board+nr_rows_on_board-1);
  }
  else
  {
    if (is_pawn(pnam_captured))
      Result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else if (pnam_captured==standard_walks[King])
      Result = square_e8;
    else if (pnam_captured==standard_walks[Knight])
      Result = cou == White ? square_g8 : square_b8;
    else if (pnam_captured==standard_walks[Rook])
      Result = cou == White ? square_a8 : square_h8;
    else if (pnam_captured==standard_walks[Queen])
      Result = square_d8;
    else if (pnam_captured==standard_walks[Bishop])
      Result = cou == White ? square_c8 : square_f8;
    else
      Result = col + onerow*(nr_of_slack_rows_below_board);
  }

  TraceFunctionExit(__func__);
  TraceSquare(Result);
  TraceFunctionResultEnd();
  return(Result);
}

square renspiegel_polymorphic(piece_walk_type p_captured, Flags p_captured_spec,
                              square sq_capture,
                              square sq_departure, square sq_arrival,
                              Side capturer)
{
  return rennormal(p_captured,p_captured_spec,sq_capture,advers(capturer));
}

typedef struct
{
  circe_variant_type const * variant;
  slice_type interval_start;
} initialisation_state_type;

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    initialisation_state_type const * const state = st->param;
    slice_index const prototypes[] = {
        alloc_pipe(state->interval_start),
        alloc_pipe(STCirceDoneWithRebirth)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_circe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STCirceDeterminingRebirth),
        alloc_pipe(STCirceDeterminedRebirth)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_basic_slices(slice_index si,
                                circe_variant_type const * variant,
                                slice_type what,
                                slice_type interval_start)
{
  initialisation_state_type state = { variant, interval_start };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,what,&instrument_move);
  stip_structure_traversal_override_single(&st,interval_start,&instrument_circe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe
 * @param si identifies the root slice of the solving machinery
 * @param variant address of the structure holding the details of the Circe variant
 * @param what what exactly is being instrumented?
 * @param interval_start start of the slices interval where to instrument
 */
void circe_initialise_solving(slice_index si,
                              circe_variant_type *variant,
                              slice_type what,
                              slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,what,"");
  TraceEnumerator(slice_type,interval_start,"");
  TraceFunctionParamListEnd();

  insert_basic_slices(si,variant,what,interval_start);

  circe_solving_instrument_relevant_capture(si,variant,interval_start);
  circe_solving_instrument_reborn_piece(si,variant,interval_start);
  circe_solving_instrument_relevant_piece(si,variant,interval_start);
  circe_solving_instrument_relevant_side_overrider(si,variant,interval_start);
  circe_solving_instrument_rebirth_square(si,variant,interval_start);

  if (CondFlag[glasgow])
    circe_glasgow_initialise_solving(si,variant,interval_start);

  if (variant->do_place_reborn)
  {
    circe_instrument_solving(si,
                             interval_start,
                             STCirceDeterminedRebirth,
                             alloc_pipe(STCircePlacingReborn));
    circe_instrument_solving(si,
                             interval_start,
                             STCirceDeterminedRebirth,
                             alloc_circe_handler_slice(STCircePlaceReborn,
                                                       variant));

    if (variant->is_promotion_possible)
      pieces_pawns_promotion_insert_solvers(si,STCircePlacingReborn);

    if (variant->is_turncoat)
      circe_solving_instrument_turncoats(si,variant,interval_start);
    if (TSTFLAG(some_pieces_flags,Volage))
      circe_solving_instrument_volage(si,variant,interval_start);

    if (variant->rebirth_square_adapter==circe_rebirth_square_adapter_frischauf)
      frischauf_initialise_solving(si);
  }

  /* "rebirth avoiders" in reverse order of their position in the sequence: */

  circe_solving_instrument_rebirth_on_occupied_square(si,variant,interval_start);

  if (variant->is_restricted_to_walks)
    circe_solving_instrument_april(si,variant,interval_start);

  circe_rex_inclusive_initialise_solving(si,variant,interval_start);

  if (variant->determine_rebirth_square==circe_determine_rebirth_square_super)
    circe_solving_instrument_super(si,variant,interval_start);
  else if (variant->determine_rebirth_square==circe_determine_rebirth_square_cage)
    circe_solving_instrument_cage(si,variant,interval_start);

  if (variant->relevant_capture==circe_relevant_capture_lastmove)
    circe_solving_instrument_parrain(si,variant,interval_start);

  if (variant->relevant_capture!=circe_relevant_capture_nocapture)
    circe_solving_instrument_nocapture_bypass(si,interval_start);

  if (variant->reborn_walk_adapter==circe_reborn_walk_adapter_chameleon)
    chameleon_init_sequence_implicit(&variant->chameleon_is_walk_squence_explicit,
                                     &variant->chameleon_walk_sequence);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  slice_type interval_start;
  slice_type hook_type;
  slice_index prototype;
} instrumentation_state_type;

static void instrument(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const copy = copy_slice(state->prototype);
    branch_insert_slices_contextual(si,st->context,&copy,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_instrumentation(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    instrumentation_state_type * const state = st->param;
    stip_structure_traversal st_nested;

    assert(slices[si].type==state->interval_start);

    stip_structure_traversal_init_nested(&st_nested,st,state);
    stip_structure_traversal_override_single(&st_nested,STCirceDoneWithRebirth,&stip_structure_visitor_noop);
    stip_structure_traversal_override_single(&st_nested,state->hook_type,&instrument);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the Circe solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval where to instrument
 * @param hook_type insertion is tried at each slice of this type
 * @param prototype prototype of type of which to add instances
 * @note circe_instrument_solving() assumes ownership of prototype
 */
void circe_instrument_solving(slice_index si,
                              slice_type interval_start,
                              slice_type hook_type,
                              slice_index prototype)
{
  instrumentation_state_type state = { interval_start, hook_type, prototype };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start,"");
  TraceFunctionParam("%u",prototype);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,interval_start,&start_instrumentation);
  stip_traverse_structure(si,&st);

  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a Circe handler slice
 * @param type type of Circe handler slice
 * @param variant address of Circe variant structure to be used for parametrising
 * @return the allocated pipe
 */
slice_index alloc_circe_handler_slice(slice_type type,
                                      circe_variant_type const *variant)
{
  slice_index const result = alloc_pipe(type);
  slices[result].u.circe_handler.variant = variant;
  return result;
}
