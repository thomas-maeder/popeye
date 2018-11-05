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
#include "conditions/circe/takemake.h"
#include "conditions/circe/volage.h"
#include "pieces/pieces.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
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
    STMarsCirceConsideringObserverRebirth,
    STAntimarsCirceConsideringRebirth,
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
    STMarsIterateObservers,
    STMarsCirceIsSquareObservedEnforceRexInclusive,
    STGenevaInitialiseRebornFromCapturer,
    STCirceCloneDetermineRebornWalk,
    STAntiCloneCirceDetermineRebornWalk,
    STChameleonCirceAdaptRebornWalk,
    STCirceEinsteinAdjustRebornWalk,
    STCirceReverseEinsteinAdjustRebornWalk,
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
    STVerticalSymmetryCirceDetermineRebirthSquare,
    STHorizontalSymmetryCirceDetermineRebirthSquare,
    STPWCDetermineRebirthSquare,
    STDiagramCirceDetermineRebirthSquare,
    STTakeMakeCirceDetermineRebirthSquares,
    STMoveGenerationPostMoveIterator,
    STSquareObservationPostMoveIterator,
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

static void insert_visit_circe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!slice_insertion_insert_before(si,st))
  {
    stip_structure_traversal st_nested;
    branch_slice_insertion_state_type state_nested;
    slice_insertion_prepare_factored_order(si,
                                           st,
                                           &st_nested,&state_nested,
                                           circe_slice_rank_order,
                                           nr_circe_slice_rank_order_elmts,
                                           nr_circe_exit_slice_types);
    stip_traverse_structure_children_pipe(si,&st_nested);
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
  stip_structure_traversal_override_single(st,STGenevaConsideringRebirth,&insert_visit_circe);
  stip_structure_traversal_override_single(st,STMarsCirceConsideringRebirth,&insert_visit_circe);
  stip_structure_traversal_override_single(st,STMarsCirceConsideringObserverRebirth,&insert_visit_circe);
  stip_structure_traversal_override_single(st,STAntimarsCirceConsideringRebirth,&insert_visit_circe);
}

/* Insert slices into a Circe execution slices sequence.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by circe_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at slice si
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void circe_insert_slices(slice_index si,
                         stip_traversal_context_type context,
                         slice_index const prototypes[],
                         unsigned int nr_prototypes)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
      prototypes, nr_prototypes,
      circe_slice_rank_order, nr_circe_slice_rank_order_elmts, nr_circe_exit_slice_types,
      branch_slice_rank_order_nonrecursive,
      0,
      si,
      0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",context);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  slice_insertion_init_traversal(&st,&state,context);
  promotion_init_slice_insertion_traversal(&st);

  state.base_rank = get_slice_rank(SLICE_TYPE(si),&state);
  stip_traverse_structure(si,&st);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

circe_variant_type circe_variant;

circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];
circe_rebirth_context_index circe_rebirth_context_stack_pointer = 0;

static circe_variant_type const circe_variant_default = {
    .rebirth_reason = move_effect_reason_rebirth_no_choice,
    .on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_relaxed,
    .do_place_reborn = true,
    .anticirce_type = anticirce_type_count
};

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void circe_reset_variant(circe_variant_type *variant)
{
  *variant = circe_variant_default;
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

static square rennormal(piece_walk_type walk_captured,
                        square sq_capture,
                        Side capturer)
{
  square  result;
  unsigned int const column = sq_capture%onerow;
  unsigned int const rank = sq_capture/onerow;
  Side const colour_sq_capture = (rank&1)!=(column&1) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_captured);
  TraceSquare(sq_capture);
  TraceEnumerator(Side,capturer);
  TraceFunctionParamListEnd();

  if (capturer == Black)
  {
    if (is_pawn(walk_captured))
      result = column + (nr_of_slack_rows_below_board+1)*onerow;
    else if (walk_captured==standard_walks[Knight])
      result = colour_sq_capture == White ? square_b1 : square_g1;
    else if (walk_captured==standard_walks[Rook])
      result = colour_sq_capture == White ? square_h1 : square_a1;
    else if (walk_captured==standard_walks[Queen])
      result = square_d1;
    else if (walk_captured==standard_walks[Bishop])
      result = colour_sq_capture == White ? square_f1 : square_c1;
    else if (walk_captured==standard_walks[King])
      result = square_e1;
    else
      result = column + onerow*(nr_of_slack_rows_below_board+nr_rows_on_board-1);
  }
  else
  {
    if (is_pawn(walk_captured))
      result = column + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else if (walk_captured==standard_walks[King])
      result = square_e8;
    else if (walk_captured==standard_walks[Knight])
      result = colour_sq_capture == White ? square_g8 : square_b8;
    else if (walk_captured==standard_walks[Rook])
      result = colour_sq_capture == White ? square_a8 : square_h8;
    else if (walk_captured==standard_walks[Queen])
      result = square_d8;
    else if (walk_captured==standard_walks[Bishop])
      result = colour_sq_capture == White ? square_c8 : square_f8;
    else
      result = column + onerow*(nr_of_slack_rows_below_board);
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
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
                                      context->relevant_square,
                                      context->relevant_side);

  pipe_dispatch_delegate(si);

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

  move_effect_journal_do_piece_readdition(SLICE_U(si).circe_handler.variant->rebirth_reason,
                                          context->rebirth_square,
                                          context->reborn_walk,
                                          context->reborn_spec,
                                          context->rebirth_as);
  pipe_dispatch_delegate(si);

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
  pipe_dispatch_delegate(si);

  --circe_rebirth_context_stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  circe_variant_type const * variant;
  slice_type interval_start;
  slice_inserter_contextual_type const inserter;
} initialisation_state_type;

static void insert_boundaries(slice_index si, stip_structure_traversal *st)
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
    (*state->inserter)(si,st->context,prototypes,nr_prototypes);
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
    circe_insert_slices(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_basic_slices(slice_index si,
                                circe_variant_type const * variant,
                                slice_type what,
                                slice_inserter_contextual_type inserter,
                                slice_type interval_start)
{
  initialisation_state_type state = { variant, interval_start, inserter };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,what,&insert_boundaries);
  stip_structure_traversal_override_single(&st,interval_start,&instrument_circe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe
 * @param si identifies the root slice of the solving machinery
 * @param variant address of the structure holding the details of the Circe variant
 * @param what what exactly is being instrumented?
 * @param inserter slice insertion function for inserting from what slices
 * @param interval_start start of the slices interval where to instrument
 */
void circe_initialise_solving(slice_index si,
                              circe_variant_type *variant,
                              slice_type what,
                              slice_inserter_contextual_type inserter,
                              slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,what);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  insert_basic_slices(si,variant,what,inserter,interval_start);

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
      promotion_insert_slice_sequence(si,STCircePlacingReborn,&circe_insert_slices);

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
  else if (variant->determine_rebirth_square==circe_determine_rebirth_square_take_and_make)
    circe_solving_instrument_takemake();

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
    circe_insert_slices(si,st->context,&copy,1);
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

    assert(SLICE_TYPE(si)==state->interval_start);

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
  TraceEnumerator(slice_type,interval_start);
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
  SLICE_U(result).circe_handler.variant = variant;
  return result;
}
