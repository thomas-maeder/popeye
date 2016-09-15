#include "conditions/marscirce/marscirce.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/pieces.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

square marscirce_rebirth_square[toppile+1];

circe_variant_type marscirce_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void marscirce_reset_variant(circe_variant_type *variant)
{
  anticirce_reset_variant(variant);

  variant->relevant_capture = circe_relevant_capture_nocapture;
  variant->default_relevant_piece = circe_relevant_piece_generated;
  variant->actual_relevant_piece = circe_relevant_piece_generated;
  variant->do_place_reborn = false;
  variant->is_rex_inclusive = true;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
}

/* Control a Mars Circe variant for meaningfulness
 * @param variant address of the variant
 */
boolean marscirce_is_variant_consistent(circe_variant_type const *variant)
{
  if (variant->on_occupied_rebirth_square_default!=circe_on_occupied_rebirth_square_strict)
    return false;
  if (variant->reborn_walk_adapter!=circe_reborn_walk_adapter_none)
    /* TODO allow circe_reborn_walk_adapter_chameleon? */
    return false;
  if (variant->is_turncoat)
    return false;
  if (variant->default_relevant_piece!=circe_relevant_piece_generated)
    return false;
  if (variant->actual_relevant_piece!=circe_relevant_piece_generated)
    return false;
  if (variant->relevant_capture!=circe_relevant_capture_nocapture)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_pwc)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_equipollents)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_take_and_make)
    return false;

  return true;
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_fix_departure(slice_index si)
{
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (curr = base+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    move_generation_stack[curr].departure = curr_generation->departure;

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
void marscirce_initialise_reborn_from_generated_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_from = curr_generation->departure;
  context->reborn_walk = move_generation_current_walk;
  context->reborn_spec = being_solved.spec[context->rebirth_from];

  context->relevant_ply = nbply;
  context->relevant_square = context->rebirth_from;
  context->relevant_side = advers(trait[context->relevant_ply]);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remember_no_rebirth(slice_index si)
{
  numecoup curr_id = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (; curr_id<current_move_id[nbply]; ++curr_id)
    marscirce_rebirth_square[curr_id] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remember_rebirth(slice_index si)
{
  numecoup curr_id = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (; curr_id<current_move_id[nbply]; ++curr_id)
    marscirce_rebirth_square[curr_id] = curr_generation->departure;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remove_capturer_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  square const sq_departure = context->rebirth_from;
  piece_walk_type const walk = get_walk_of_piece_on_square(sq_departure);
  Flags const flags = being_solved.spec[sq_departure];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(walk!=Empty);

  empty_square(sq_departure);
  pipe_dispatch_delegate(si);
  occupy_square(sq_departure,walk,flags);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_from_rebirth_square(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer-1];
  square const sq_departure = curr_generation->departure;
  square const sq_rebirth = context->rebirth_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  assert(circe_rebirth_context_stack_pointer>0);

  curr_generation->departure = sq_rebirth;

  occupy_square(sq_rebirth,context->reborn_walk,context->reborn_spec);
  pipe_move_generation_delegate(si);
  empty_square(sq_rebirth);

  curr_generation->departure = sq_departure;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_moves_enforce_rex_exclusive(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(being_solved.spec[curr_generation->departure],Royal))
    pipe_move_generation_delegate(si);

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
void marscirce_move_to_rebirth_square_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt * const move_gen_top = move_generation_stack+curr;
  numecoup const id = move_gen_top->id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (marscirce_rebirth_square[id]==initsquare
      /* marscirce_rebirth_square isn't set when castlings are generated */
      || (min_castling<=sq_capture && sq_capture<=max_castling))
  {
    move_effect_journal_do_null_effect();
    pipe_solve_delegate(si);
  }
  else
  {
    square const sq_departure = move_gen_top->departure;

    move_effect_journal_do_piece_movement(move_effect_reason_phantom_movement,
                                          sq_departure,marscirce_rebirth_square[id]);

    move_gen_top->departure = marscirce_rebirth_square[id];
    pipe_solve_delegate(si);
    move_gen_top->departure = sq_departure;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean mars_enforce_observer(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer-1];
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = context->rebirth_square;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(circe_rebirth_context_stack_pointer>0);

  if (sq_observer==sq_departure)
  {
    /* restore as if the capture had occcured directly, to allow other
     * conditions (e.g. Madrasi) to correctly work. */
    Flags const spec_observing = being_solved.spec[sq_observer];

    empty_square(sq_observer);
    occupy_square(context->rebirth_from,observing_walk[nbply],spec_observing);
    move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure = context->rebirth_from;

    result = pipe_validate_observation_recursive_delegate(si);

    move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure = sq_observer;
    empty_square(context->rebirth_from);
    occupy_square(sq_observer,observing_walk[nbply],spec_observing);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a specific piece delivers check to a specific side
 * @param observer_origin potentially delivering check ...
 * @note the piece on pos_checking must belong to advers(side)
 * @note sets observation_result
 */
void marscirce_is_square_observed(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer-1];
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  assert(circe_rebirth_context_stack_pointer>0);

  observation_result = false;

  if (observing_walk[nbply]<Queen || observing_walk[nbply]>Bishop
      || CheckDir[observing_walk[nbply]][sq_target-context->rebirth_square]!=0)
  {
    if (is_square_empty(context->rebirth_square))
    {
      TraceSquare(context->rebirth_square);
      TraceWalk(context->reborn_walk);
      TraceValue("%u",TSTFLAG(being_solved.spec[context->rebirth_square],White));
      TraceValue("%u",TSTFLAG(being_solved.spec[context->rebirth_square],Black));
      TraceEOL();
      occupy_square(context->rebirth_square,context->reborn_walk,context->reborn_spec);
      pipe_is_square_observed_delegate(si);
      empty_square(context->rebirth_square);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side observes a specific square
 * @param si identifies the tester slice
 * @note sets observation_result
 */
void marscirce_iterate_observers(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  Side const side_observing = trait[nbply];
  square const *observer_origin;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  observation_result = false;

  context->relevant_ply = nbply;

  for (observer_origin = boardnum; *observer_origin; ++observer_origin)
    if (*observer_origin!=sq_target /* no auto-observation */
        && TSTFLAG(being_solved.spec[*observer_origin],side_observing)
        && get_walk_of_piece_on_square(*observer_origin)==observing_walk[nbply])
    {
      context->rebirth_from = *observer_origin;
      context->reborn_walk = observing_walk[nbply];
      context->reborn_spec = being_solved.spec[context->rebirth_from];
      context->relevant_side = advers(side_observing);
      context->relevant_square = context->rebirth_from;

      pipe_is_square_observed_delegate(si);

      if (observation_result)
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @note sets observation_validation_result
 */
void marscirce_is_square_observed_enforce_rex_exclusive(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(context->reborn_spec,Royal))
    pipe_is_square_observed_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_no_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMarsCirceRememberNoRebirth),
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STMarsCirceGenerateFromRebirthSquare),
        alloc_pipe(STMarsCirceRememberRebirth),
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_marscirce(slice_index si)
{
  circe_variant_type observation_variant = marscirce_variant;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_no_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_rebirth);
    stip_traverse_structure(si,&st);
  }

  circe_initialise_solving(si,
                           &marscirce_variant,
                           STMoveForPieceGeneratorAlternativePath,
                           &slice_insertion_insert_contextually,
                           STMarsCirceConsideringRebirth);
  circe_instrument_solving(si,
                           STMarsCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveCapturer));

  observation_variant.default_relevant_piece = circe_relevant_piece_observing_walk;
  /* cf. get_relevant_piece_determinator */
  observation_variant.actual_relevant_piece = circe_relevant_piece_observing_walk;

  circe_initialise_solving(si,
                           &observation_variant,
                           STDetermineObserverWalk,
                           &slice_insertion_insert_contextually,
                           STMarsCirceConsideringObserverRebirth);
  circe_instrument_solving(si,
                           STMarsCirceConsideringObserverRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveCapturer));

  if (marscirce_variant.rebirth_reason==move_effect_reason_rebirth_choice)
  {
    circe_instrument_solving(si,
                             STMarsCirceConsideringRebirth,
                             STMarsCirceConsideringRebirth,
                             alloc_pipe(STMoveGenerationPostMoveIterator));
    circe_instrument_solving(si,
                             STMarsCirceConsideringObserverRebirth,
                             STMarsCirceConsideringObserverRebirth,
                             alloc_pipe(STSquareObservationPostMoveIterator));
  }

  if (!observation_variant.is_rex_inclusive)
    circe_instrument_solving(si,
                             STMarsCirceConsideringObserverRebirth,
                             STMarsIterateObservers,
                             alloc_pipe(STMarsCirceIsSquareObservedEnforceRexInclusive));

  stip_instrument_is_square_observed_testing(si,nr_sides,STMarsIsSquareObserved);

  stip_instrument_check_validation(si,nr_sides,STMarsCirceGenerateFromRebirthSquare);
  stip_instrument_observation_validation(si,nr_sides,STMarsCirceGenerateFromRebirthSquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
