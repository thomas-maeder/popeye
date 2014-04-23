#include "conditions/marscirce/marscirce.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "optimisations/orthodox_check_directions.h"
#include "solving/pipe.h"
#include "pieces/pieces.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

square (*marscirce_determine_rebirth_square)(piece_walk_type, Flags, square, square, square, Side);

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

  solve(slices[si].next1);

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
  context->reborn_spec = spec[context->rebirth_from];

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

  solve(slices[si].next1);

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

  solve(slices[si].next1);

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
  square const sq_departure = curr_generation->departure;
  piece_walk_type const walk = get_walk_of_piece_on_square(sq_departure);
  Flags const flags = spec[sq_departure];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  empty_square(sq_departure);
  solve(slices[si].next1);
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
  solve(slices[si].next1);
  empty_square(sq_rebirth);

  curr_generation->departure = sq_departure;

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
  square const sq_departure = move_gen_top->departure;
  numecoup const id = move_gen_top->id;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (marscirce_rebirth_square[id]==initsquare)
  {
    move_effect_journal_do_null_effect();
    pipe_solve_delegate(si);
  }
  else
  {
    move_effect_journal_do_piece_movement(move_effect_reason_phantom_movement,
                                          sq_departure,marscirce_rebirth_square[id]);

    move_gen_top->departure = marscirce_rebirth_square[id];
    pipe_solve_delegate(si);
    move_gen_top->departure = sq_departure;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square current_rebirth_square[maxply+1];
static square current_observer_origin[maxply+1];

boolean mars_enforce_observer(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = current_rebirth_square[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_observer==sq_departure)
  {
    /* restore as if the capture had occcured directly, to allow other
     * conditions (e.g. Madrasi) to correctly work. */
    Flags const spec_observing = spec[sq_observer];

    empty_square(sq_observer);
    occupy_square(current_observer_origin[nbply],observing_walk[nbply],spec_observing);
    move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure = current_observer_origin[nbply];

    result = validate_observation_recursive(slices[si].next1);

    move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure = sq_observer;
    empty_square(current_observer_origin[nbply]);
    occupy_square(sq_observer,observing_walk[nbply],spec_observing);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param observer_origin potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on observer_origin must belong to advers(side)
 */
boolean mars_is_square_observed_from_rebirth_square(slice_index si,
                                                    validator_id evaluate,
                                                    square observer_origin,
                                                    square sq_rebirth)
{
  boolean result = false;
  Flags const spec_observing = spec[observer_origin];
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(observer_origin);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  current_rebirth_square[nbply] = sq_rebirth;
  current_observer_origin[nbply] = observer_origin;

  if (observing_walk[nbply]<Queen || observing_walk[nbply]>Bishop
      || CheckDir[observing_walk[nbply]][sq_target-current_rebirth_square[nbply]]!=0)
  {
    empty_square(observer_origin);

    /* test only now - we may have just emptied the rebirth square! */
    if (is_square_empty(current_rebirth_square[nbply]))
    {
      occupy_square(current_rebirth_square[nbply],observing_walk[nbply],spec_observing);
      result = is_square_observed_recursive(slices[si].next1,evaluate);
      empty_square(current_rebirth_square[nbply]);
    }

    occupy_square(observer_origin,observing_walk[nbply],spec_observing);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a specific piece delivers check to a specific side
 * @param observer_origin potentially delivering check ...
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(slice_index si,
                                   validator_id evaluate,
                                   square observer_origin)
{
  boolean result = false;
  Flags const spec_checking = spec[observer_origin];
  Side const side_observed = advers(trait[nbply]);

  square const sq_rebirth = (*marscirce_determine_rebirth_square)(observing_walk[nbply],
                                                                  spec_checking,
                                                                  observer_origin,
                                                                  initsquare,
                                                                  initsquare,
                                                                  side_observed);

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceSquare(observer_origin);
  TraceFunctionParamListEnd();

  result = mars_is_square_observed_from_rebirth_square(si,evaluate,observer_origin,sq_rebirth);

  current_rebirth_square[nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side observes a specific square
 * @param si identifies the tester slice
 * @return true iff side is in check
 */
boolean marscirce_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;
  Side const side_observing = trait[nbply];
  square const *observer_origin;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  for (observer_origin = boardnum; *observer_origin; ++observer_origin)
    if (*observer_origin!=sq_target /* no auto-observation */
        && TSTFLAG(spec[*observer_origin],side_observing)
        && get_walk_of_piece_on_square(*observer_origin)==observing_walk[nbply]
        && mars_is_square_observed_by(si,evaluate,*observer_origin))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
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
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_marscirce(slice_index si)
{
  TraceFunctionEntry(__func__);
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
                           STMarsCirceConsideringRebirth);
  circe_instrument_solving(si,
                           STMarsCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveCapturer));

  stip_instrument_is_square_observed_testing(si,nr_sides,STMarsIsSquareObserved);

  stip_instrument_check_validation(si,nr_sides,STMarsCirceGenerateFromRebirthSquare);
  stip_instrument_observation_validation(si,nr_sides,STMarsCirceGenerateFromRebirthSquare);

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);
  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
