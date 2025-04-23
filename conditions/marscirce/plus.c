#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "pieces/pieces.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static square const center_squares[] = { square_d4, square_d5, square_e4, square_e5 };
enum { nr_center_squares = sizeof center_squares / sizeof center_squares[0] };

/* Try occupying rebirth square and generate moves
 * @param si identifies move generator slice
 * @param sq_rebirth rebirth square
 */
static void try_rebirth_and_generate(slice_index si, square sq_rebirth)
{
  square const sq_departure = curr_generation->departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  assert(sq_rebirth!=sq_departure);

  if (is_square_empty(sq_rebirth))
  {
    curr_generation->departure = sq_rebirth;

    occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),being_solved.spec[sq_departure]);
    empty_square(sq_departure);

    pipe_move_generation_delegate(si);

    occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),being_solved.spec[sq_rebirth]);
    empty_square(sq_rebirth);

    curr_generation->departure = sq_departure;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void plus_generate_additional_captures_for_piece(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_center_squares; ++i)
    if (sq_departure==center_squares[i])
    {
      unsigned int j;
      for (j = 0; j!=nr_center_squares; ++j)
        if (j!=i)
          try_rebirth_and_generate(si,center_squares[j]);
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void is_square_observed_from_rebirth_square(slice_index si,
                                                   validator_id evaluate,
                                                   square observer_origin,
                                                   square sq_rebirth)
{
  Flags const spec_observing = being_solved.spec[observer_origin];
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(observer_origin);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  observation_result = false;

  if (observing_walk[nbply]<Queen || observing_walk[nbply]>Bishop
      || CheckDir(observing_walk[nbply])[sq_target-sq_rebirth]!=0)
  {
    empty_square(observer_origin);

    /* test only now - we may have just emptied the rebirth square! */
    if (is_square_empty(sq_rebirth))
    {
      occupy_square(sq_rebirth,observing_walk[nbply],spec_observing);
      pipe_is_square_observed_delegate(si);
      empty_square(sq_rebirth);
    }

    occupy_square(observer_origin,observing_walk[nbply],spec_observing);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void is_square_observed_from_center(slice_index si,
                                           validator_id evaluate,
                                           square observer_origin)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(observer_origin);
  TraceFunctionParamListEnd();

  observation_result = false;

  for (i = 0; i!=nr_center_squares; ++i)
    if (observer_origin!=center_squares[i]) /* already tested without rebirth */
    {
      is_square_observed_from_rebirth_square(si,evaluate,observer_origin,center_squares[i]);
      if (observation_result)
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side observes a specific square
 * @param identifies tester slice
 * @note sets observation_result
 */
void plus_is_square_observed(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  pipe_is_square_observed_delegate(si);

  if (!observation_result)
  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    Side const side_observing = trait[nbply];
    unsigned int i;

    for (i = 0; i!=nr_center_squares; ++i)
    {
      square const observer_origin = center_squares[i];
      if (TSTFLAG(being_solved.spec[observer_origin],side_observing)
          && get_walk_of_piece_on_square(observer_origin)==observing_walk[nbply]
          && observer_origin!=sq_target) /* no auto-observation */
      {
        is_square_observed_from_center(si,observation_validator,observer_origin);
        if (observation_result)
          break;
      }
    }
  }

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
        alloc_pipe(STMarsCirceRememberNoRebirth)
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
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STPhantomAvoidDuplicateMoves),
        alloc_pipe(STPlusAdditionalCapturesForPieceGenerator),
        alloc_pipe(STMarsCirceRememberRebirth),
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise thet solving machinery with Plus Chess
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_plus(slice_index si)
{
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
                                             &instrument_no_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_rebirth);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_is_square_observed_testing(si,nr_sides,STPlusIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
