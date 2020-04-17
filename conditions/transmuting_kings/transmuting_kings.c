#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"

piece_walk_type transmuting_kings_potential_transmutations[nr_piece_walks];

boolean transmuting_kings_testing_transmutation[nr_sides];

static boolean testing_with_non_transmuting_king[maxply+1];

static enum
{
  dont_know,
  does_transmute,
  does_not_transmute
} is_king_transmuting_as_observing_walk[maxply+1];

static boolean is_king_transmuting_as_any_walk[maxply+1];

/* Initialise the sequence of king transmuters
 */
void transmuting_kings_init_transmuters_sequence(void)
{
  unsigned int tp = 0;
  piece_walk_type p;

  for (p = King; p<nr_piece_walks; ++p) {
    if (piece_walk_may_exist[p] && p!=Dummy && p!=Hamster)
    {
      transmuting_kings_potential_transmutations[tp] = p;
      tp++;
    }
  }

  transmuting_kings_potential_transmutations[tp] = Empty;
}

/* Determine whether the moving side's king is transmuting as a specific walk
 * @param p the piece
 */
boolean transmuting_kings_is_king_transmuting_as(piece_walk_type walk)
{
  boolean result;
  Side const side_attacking = trait[nbply];
  square const sq_king = being_solved.king_square[side_attacking];

  TraceFunctionEntry(__func__);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  assert(sq_king!=initsquare);

  if (transmuting_kings_testing_transmutation[side_attacking])
    result = false;
  else
  {
    transmuting_kings_testing_transmutation[side_attacking] = true;

    siblingply(advers(side_attacking));
    push_observation_target(sq_king);
    observing_walk[nbply] = walk;
    result = fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed_specific[trait[nbply]],
                                                     EVALUATE(observation));
    finply();

    transmuting_kings_testing_transmutation[side_attacking] = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves of a potentially transmuting king
 * @param si identifies move generator slice
 * @return true iff the king is transmuting (which doesn't necessarily mean that
 *              any moves were generated!)
 */
boolean generate_moves_of_transmuting_king(slice_index si)
{
  boolean result = false;
  piece_walk_type const *ptrans;
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  Side const side_moving = trait[nbply];
  square const sq_king = being_solved.king_square[side_moving];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_king!=initsquare)
  {
    for (ptrans = transmuting_kings_potential_transmutations; *ptrans!=Empty; ++ptrans)
      if (transmuting_kings_is_king_transmuting_as(*ptrans))
      {
        pipe_move_generation_different_walk_delegate(si,*ptrans);
        result = true;
      }

    remove_duplicate_moves_of_single_piece(save_current_move);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void transmuting_kings_generate_moves_for_piece(slice_index si)
{
  Flags const mask = BIT(trait[nbply])|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFULLFLAGMASK(being_solved.spec[curr_generation->departure],mask))
  {
    if (!generate_moves_of_transmuting_king(si))
      pipe_move_generation_delegate(si);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    Side side;
    slice_index past_detector;
} instrumentation_state_type;

static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==state->side)
  {
    assert(state->past_detector==no_slice);
    is_square_observed_insert_slice(si,alloc_pipe(STTransmutingKingDetectNonTransmutation));
    stip_traverse_structure_children(si,st);
    assert(state->past_detector!=no_slice);
    is_square_observed_insert_slice(si,
                                    alloc_fork_slice(STTransmutingKingIsSquareObserved,
                                                     state->past_detector));
    state->past_detector = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_detector(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (SLICE_STARTER(si)==no_side /* just inserted */
      && state->past_detector==no_slice)
  {
    SLICE_STARTER(si) = state->side;
    state->past_detector = alloc_proxy_slice();
    pipe_append(si,state->past_detector);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the observation machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 * @note invoked by transmuting_kings_initialise_observing()
 */
void transmuting_kings_initialise_observing(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  {
    instrumentation_state_type state = { side, no_slice  };
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STTestingIfSquareIsObserved,
                                             &instrument_testing);
    stip_structure_traversal_override_single(&st,
                                             STTransmutingKingDetectNonTransmutation,
                                             &remember_detector);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_observation_validation(si,side,STTransmutingKingsEnforceObserverWalk);
  stip_instrument_check_validation(si,side,STTransmutingKingsEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void transmuting_kings_initialise_solving(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,side,STTransmutingKingsMovesForPieceGenerator);

  transmuting_kings_initialise_observing(si,side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a square is observed be the side at the move according to
 * Transmuting Kings
 * @param si identifies next slice
 * @note sets observation_result
 */
void transmuting_king_is_square_observed(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_is_square_observed_delegate(si);

  if (!observation_result)
  {
    square const sq_king = being_solved.king_square[trait[nbply]];
    if (sq_king!=initsquare && !is_king_transmuting_as_any_walk[nbply])
    {
      testing_with_non_transmuting_king[nbply] = true;
      observing_walk[nbply] = get_walk_of_piece_on_square(sq_king);
      fork_is_square_observed_delegate(si);
      testing_with_non_transmuting_king[nbply] = false;
    }
  }

  is_king_transmuting_as_any_walk[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find out if the royal piece is not transmuted (i.e. moves according to its
 * original walk)
 * @param si identifies next slice
 * @note sets observation_result
 */
void transmuting_king_detect_non_transmutation(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(!testing_with_non_transmuting_king[nbply]);

  is_king_transmuting_as_observing_walk[nbply] = dont_know;

  pipe_is_square_observed_delegate(si);

  if (!observation_result && !is_king_transmuting_as_any_walk[nbply])
    switch (is_king_transmuting_as_observing_walk[nbply])
    {
      case dont_know:
      {
        Side const side_transmuting = trait[nbply];
        square const sq_king = being_solved.king_square[side_transmuting];

        is_king_transmuting_as_any_walk[nbply] = sq_king!=initsquare && transmuting_kings_is_king_transmuting_as(observing_walk[nbply]);
        break;
      }

      case does_not_transmute:
        break;

      case does_transmute:
        is_king_transmuting_as_any_walk[nbply] = true;
        break;

      default:
        assert(0);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean transmuting_kings_enforce_observer_walk(slice_index si)
{
  boolean result;
  square const sq_king = being_solved.king_square[trait[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (transmuting_kings_testing_transmutation[advers(trait[nbply])])
    result = pipe_validate_observation_recursive_delegate(si);
  else if (testing_with_non_transmuting_king[nbply])
  {
    assert(observing_walk[nbply]==get_walk_of_piece_on_square(sq_king));
    if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==sq_king)
      result = pipe_validate_observation_recursive_delegate(si);
    else
      result = false;
  }
  else if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==sq_king)
  {
    if (transmuting_kings_is_king_transmuting_as(observing_walk[nbply]))
    {
      piece_walk_type const save_walk = observing_walk[nbply];
      observing_walk[nbply] = get_walk_of_piece_on_square(sq_king);
      result = pipe_validate_observation_recursive_delegate(si);
      observing_walk[nbply] = save_walk;
      is_king_transmuting_as_observing_walk[nbply] = does_transmute;
    }
    else
    {
      result = false;
      is_king_transmuting_as_observing_walk[nbply] = does_not_transmute;
    }
  }
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
