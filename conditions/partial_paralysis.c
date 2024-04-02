#include "conditions/partial_paralysis.h"
#include "conditions/madrasi.h"
#include "pieces/pieces.h"
#include "pieces/walks/generate_moves.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/stipulation.h"

#include "debugging/trace.h"

#include <string.h>

static boolean recursion_breaker;

/* Determine whether moving to a specific square is forbidden according
 * to the moves just generated in an auxiliary ply
 * @param s the potentially forbidden destination square
 * @param base the base index of the auxiliary ply (which is the current one)
 * @return true iff the destination square is forbidden
 */
static boolean is_forbidden_destination_square(square s, numecoup base)
{
  boolean result = false;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (curr=base+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    if (s==move_generation_stack[curr].arrival)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the observee is reachable from the obesrver's position
 * using a specific walk
 * @param walk_paralysing the walk
 */
static boolean does_walk_reach_target(piece_walk_type walk_paralysing)
{
  boolean result = false;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  Side const side_observing = trait[nbply];
  square const save_departure = curr_generation->departure;
  numecoup const base_of_move_generation = CURRMOVE_OF_PLY(nbply);
  piece_walk_type const walk_paralysed = get_walk_of_piece_on_square(sq_observer);
  Flags const paralysed_spec = being_solved.spec[sq_observer];

  TraceFunctionEntry(__func__);
  TraceWalk(walk_paralysing);
  TraceFunctionParamListEnd();

  occupy_square(sq_observer,walk_paralysing,paralysed_spec);

  siblingply(side_observing);

  curr_generation->departure = sq_observer;
  move_generation_current_walk = walk_paralysing;
  generate_moves_for_piece_based_on_walk();
  curr_generation->departure = save_departure;

  result = is_forbidden_destination_square(sq_observee,base_of_move_generation);

  finply();

  occupy_square(sq_observer,walk_paralysed,paralysed_spec);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the currently validated observation is forbidden
 * because the observer is partially paralysed
 * @return true iff the observation is forbidden
 */
static boolean is_observation_forbidden_by_partial_paralysis(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!recursion_breaker)
  {
    Side const side_observing = trait[nbply];
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    piece_walk_type walk;

    recursion_breaker = true;

    for (walk = King; walk!=nr_piece_walks; ++walk)
      if (madrasi_is_piece_observed_by_walk(sq_departure,walk,side_observing)
          && does_walk_reach_target(walk))
      {
        result = true;
        break;
      }

    recursion_breaker = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observater according to Partial Paralysis
 * @return true iff the observation is valid
 */
boolean partial_paralysis_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_observation_forbidden_by_partial_paralysis()
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remove moves from generation stack that have been found by generating moves
 * of an observing walk
 * @param base_ply base index in the move generation stack of that ply
 * @param base_forbidden base index of the auxiliary ply containing the
 *                            potentially forbidden movements
 * @return new top of the ply where we have removed moves
 */
static numecoup remove_forbidden_moves_from_ply(numecoup base_ply,
                                                numecoup base_forbidden)
{
  numecoup curr = base_ply+1;
  numecoup result = base_forbidden;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base_ply);
  TraceFunctionParam("%u",base_forbidden);
  TraceFunctionParamListEnd();

  while (curr<=result)
    if (is_forbidden_destination_square(move_generation_stack[curr].arrival,base_forbidden))
    {
      memmove(&move_generation_stack[curr],
              &move_generation_stack[curr+1],
              (result-curr) * sizeof move_generation_stack[curr]);
      --result;
    }
    else
      ++curr;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find moves as would be executed by an observing walk and remove them from the
 * move generation stack
 * @praam base_of_move_generation base index in move generation stack of the
 *                                moves generated for a specific piece
 * @param walk_paralysing walk of a piece that paralyses the piece
 * @param side_moving the side for which we are generating moves
 * @param sq_moving_piece position of the piece for which we are generating moves
 */
static void remove_forbidden_moves_one_walk(numecoup base_of_move_generation,
                                            piece_walk_type walk_paralysing,
                                            Side side_moving,
                                            square sq_moving_piece)
{
  piece_walk_type const walk_paralysed = get_walk_of_piece_on_square(sq_moving_piece);
  Flags const paralysed_spec = being_solved.spec[sq_moving_piece];
  numecoup top_of_ply = CURRMOVE_OF_PLY(nbply);
  square const paralysis_departure = move_generation_stack[top_of_ply].departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base_of_move_generation);
  TraceWalk(walk_paralysing);
  TraceEnumerator(Side,side_moving);
  TraceSquare(sq_moving_piece);
  TraceFunctionParamListEnd();

  occupy_square(sq_moving_piece,walk_paralysing,paralysed_spec);

  siblingply(side_moving);

  curr_generation->departure = sq_moving_piece;
  move_generation_current_walk = walk_paralysing;
  generate_moves_for_piece_based_on_walk();
  curr_generation->departure = paralysis_departure;

  top_of_ply = remove_forbidden_moves_from_ply(base_of_move_generation,top_of_ply);

  finply();

  CURRMOVE_OF_PLY(nbply) = top_of_ply;

  occupy_square(sq_moving_piece,walk_paralysed,paralysed_spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param si identifies generator slice
 */
void partial_paralysis_generate_moves_for_piece(slice_index si)
{
  numecoup const move_generation_base = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  if (!recursion_breaker)
  {
    Side const side_moving = trait[nbply];
    square const sq_departure = curr_generation->departure;
    piece_walk_type walk_paralysing;

    recursion_breaker = true;

    for (walk_paralysing = King; walk_paralysing!=nr_piece_walks; ++walk_paralysing)
      if (madrasi_is_piece_observed_by_walk(sq_departure,walk_paralysing,side_moving))
        remove_forbidden_moves_one_walk(move_generation_base,
                                        walk_paralysing,
                                        side_moving,
                                        sq_departure);

    recursion_breaker = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Partial Paralysis
 * @param si identifies root slice of solving machinery
 */
void partial_paralysis_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STPartialParalysisMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STPartialParalysisMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
