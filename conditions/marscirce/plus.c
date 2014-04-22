#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

static square const center_squares[] = { square_d4, square_d5, square_e4, square_e5 };
enum { nr_center_squares = sizeof center_squares / sizeof center_squares[0] };

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
          marscirce_try_rebirth_and_generate(si,center_squares[j]);
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_square_observed_from_center(slice_index si,
                                              validator_id evaluate,
                                              square observer_origin)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(observer_origin);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_center_squares; ++i)
    if (observer_origin!=center_squares[i] /* already tested without rebirth */
        && mars_is_square_observed_from_rebirth_square(si,evaluate,observer_origin,center_squares[i]))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side observes a specific square
 * @param identifies tester slice
 * @return true iff side is in check
 */
boolean plus_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    Side const side_observing = trait[nbply];
    unsigned int i;

    for (i = 0; i!=nr_center_squares; ++i)
    {
      square const observer_origin = center_squares[i];
      if (TSTFLAG(spec[observer_origin],side_observing)
          && get_walk_of_piece_on_square(observer_origin)==observing_walk[nbply]
          && observer_origin!=sq_target /* no auto-observation */
          && is_square_observed_from_center(si,evaluate,observer_origin))
      {
        result = true;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
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

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_rebirth);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_is_square_observed_testing(si,nr_sides,STPlusIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
