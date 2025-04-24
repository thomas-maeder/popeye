#include "conditions/marscirce/phantom.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "pieces/pieces.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean phantom_king_square_observation_tester_ply_initialiser_is_in_check(slice_index si,
                                                                           Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  marscirce_rebirth_square[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id] = initsquare;
  result = pipe_is_in_check_recursive_delegate(si,side_in_check);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

circe_variant_type phantom_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void phantom_reset_variant(circe_variant_type *variant)
{
  anticirce_reset_variant(variant);

  variant->relevant_capture = circe_relevant_capture_nocapture;
  variant->default_relevant_piece = circe_relevant_piece_generated;
  variant->actual_relevant_piece = circe_relevant_piece_generated;
  variant->do_place_reborn = false;
  variant->is_rex_inclusive = false;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
}

static boolean is_regular_arrival(square sq_arrival,
                                  numecoup start_moves_from_rebirth_square,
                                  square sq_departure)
{
  boolean result = false;
  numecoup curr_regular_move;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParam("%u",start_moves_from_rebirth_square);
  TraceFunctionParamListEnd();

  for (curr_regular_move = start_moves_from_rebirth_square;
       move_generation_stack[curr_regular_move].departure==sq_departure;
       --curr_regular_move)
    if (move_generation_stack[curr_regular_move].arrival==sq_arrival)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_avoid_duplicate_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  numecoup const start_moves_from_rebirth_square = CURRMOVE_OF_PLY(nbply);
  numecoup top_filtered = start_moves_from_rebirth_square;
  numecoup curr_from_sq_rebirth;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (curr_from_sq_rebirth = start_moves_from_rebirth_square+1;
       curr_from_sq_rebirth<=CURRMOVE_OF_PLY(nbply);
       ++curr_from_sq_rebirth)
  {
    square const sq_arrival = move_generation_stack[curr_from_sq_rebirth].arrival;
    if (sq_arrival!=sq_departure
        && !is_regular_arrival(sq_arrival,
                               start_moves_from_rebirth_square,
                               sq_departure))
    {
      ++top_filtered;
      move_generation_stack[top_filtered] = move_generation_stack[curr_from_sq_rebirth];
    }
  }

  SET_CURRMOVE(nbply,top_filtered);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_generation_no_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMarsCirceRememberNoRebirth)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_generation_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STPhantomAvoidDuplicateMoves),
        alloc_pipe(STMarsCirceGenerateFromRebirthSquare),
        alloc_pipe(STMarsCirceRememberRebirth)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  if (!phantom_variant.is_rex_inclusive)
  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsCirceMoveGeneratorEnforceRexInclusive)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_observation_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsIsSquareObserved)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si)
{
  circe_variant_type observation_variant = phantom_variant;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();
  move_generator_instrument_for_alternative_paths(si,nr_sides);

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);
  move_effect_journal_register_pre_capture_effect();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_generation_no_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_generation_rebirth);
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
                           alloc_pipe(STMarscirceRemoveReborn));

  is_square_observed_instrument_for_alternative_paths(si,nr_sides);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STIsSquareObservedAlternativePath,
                                             &instrument_observation_rebirth);
    stip_traverse_structure(si,&st);
  }

  observation_variant.default_relevant_piece = circe_relevant_piece_observing_walk;
  /* cf. get_relevant_piece_determinator */
  observation_variant.actual_relevant_piece = circe_relevant_piece_observing_walk;

  circe_initialise_solving(si,
                           &observation_variant,
                           STIsSquareObservedAlternativePath,
                           &slice_insertion_insert_contextually,
                           STMarsCirceConsideringObserverRebirth);
  circe_instrument_solving(si,
                           STMarsCirceConsideringObserverRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveReborn));

  if (phantom_variant.rebirth_reason==move_effect_reason_rebirth_choice)
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

  solving_instrument_check_testing(si,STPhantomKingSquareObservationTesterPlyInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
