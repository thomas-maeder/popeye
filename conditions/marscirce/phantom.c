#include "conditions/marscirce/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/binary.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

boolean phantom_chess_rex_inclusive;

square marscirce_rebirth_square[toppile+1];

static boolean is_regular_arrival(square sq_arrival,
                                  numecoup start_moves_from_rebirth_square,
                                  square sq_departure)
{
  boolean result = false;
  numecoup curr_regular_move;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParam("%u",start_regular_moves);
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
void phantom_enforce_rex_inclusive(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(spec[curr_generation->departure],Royal))
    generate_moves_for_piece(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  generate_moves_for_piece(slices[si].next1);

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

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_generate_moves_for_piece(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  square const sq_rebirth = rennormal(move_generation_current_walk,
                                      spec[sq_departure],
                                      sq_departure,
                                      advers(trait[nbply]));

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (sq_rebirth!=sq_departure)
    marscirce_try_rebirth_and_generate(si,sq_rebirth);

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
    slice_index const proxy_regular = alloc_proxy_slice();
    slice_index const regular = alloc_pipe(STMoveForPieceGeneratorStandardPath);
    slice_index const remember_no_rebirth = alloc_pipe(STMarsCirceRememberNoRebirth);

    slice_index const proxy_phantom = alloc_proxy_slice();
    slice_index const capture_phantom = alloc_pipe(STMoveForPieceGeneratorAlternativePath);
    slice_index const fix_departure = alloc_pipe(STMarsCirceFixDeparture);
    slice_index const avoid_duplicates = alloc_pipe(STPhantomAvoidDuplicateMoves);
    slice_index const generate_phantom = alloc_pipe(STPhantomMovesForPieceGenerator);
    slice_index const remember_rebirth = alloc_pipe(STMarsCirceRememberRebirth);

    slice_index const separator = alloc_binary_slice(STMoveForPieceGeneratorTwoPaths,
                                                     proxy_regular,
                                                     proxy_phantom);

    pipe_link(slices[si].prev,separator);

    pipe_link(proxy_regular,regular);
    pipe_link(regular,remember_no_rebirth);
    pipe_link(remember_no_rebirth,si);

    pipe_link(proxy_phantom,capture_phantom);
    pipe_link(capture_phantom,fix_departure);
    pipe_link(fix_departure,avoid_duplicates);
    pipe_link(avoid_duplicates,generate_phantom);
    pipe_link(generate_phantom,remember_rebirth);
    pipe_link(remember_rebirth,si);

    if (!phantom_chess_rex_inclusive)
      pipe_append(capture_phantom,alloc_pipe(STPhantomEnforceRexInclusive));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);

  {
    stip_structure_traversal st;

    solving_instrument_move_generation(si,
                                       no_side,
                                       STMoveForPieceGeneratorPathsJoint);

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorPathsJoint,
                                             &insert_separator);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_is_square_observed_testing(si,nr_sides,STPhantomIsSquareObserved);

  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    Side const side_observing = trait[nbply];
    square const *observer_origin;
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

    for (observer_origin = boardnum; *observer_origin; ++observer_origin)
      if (*observer_origin!=sq_target /* no auto-observation */
          && (!TSTFLAG(spec[*observer_origin],Royal) || phantom_chess_rex_inclusive)
          && TSTFLAG(spec[*observer_origin],side_observing)
          && get_walk_of_piece_on_square(*observer_origin)==observing_walk[nbply]
          && mars_is_square_observed_by(si,evaluate,*observer_origin))
      {
        result = true;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
