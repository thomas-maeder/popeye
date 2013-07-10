#include "pieces/angle/hoppers.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"

/* Generated moves for an angle hopper
 * @param sq_departure departure square of moves to be generated
 * @param kanf first vectors index
 * @param kend last vectors index
 * @param angle angle to take from hurdle to arrival squares
 * @param side side for which to generate moves
 */
void angle_hoppers_generate_moves(square sq_departure,
                                  vec_index_type kanf, vec_index_type kend,
                                  angle_t angle)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TraceFunctionParamListEnd();

  for (k = kend; k>=kanf; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      vec_index_type const k1 = 2*k;

      {
        square const sq_arrival= sq_hurdle+angle_vectors[angle][k1];
        if (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival))
        {
          add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = sq_hurdle;
          move_generation_stack[current_move[nbply]].auxiliary.hopper.vec_index = k;
        }
      }

      {
        square const sq_arrival= sq_hurdle+angle_vectors[angle][k1-1];
        if (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival))
        {
          add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = sq_hurdle;
          move_generation_stack[current_move[nbply]].auxiliary.hopper.vec_index = k;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean angle_hoppers_is_square_observed_one_dir(square sq_target,
                                                        square sq_hurdle,
                                                        vec_index_type vec_index_departure_hurdle,
                                                        angle_t angle,
                                                        PieNam p,
                                                        evalfunction_t *evaluate)
{
  numvec const vec_departure_hurdle = angle_vectors[angle][vec_index_departure_hurdle];
  square const sq_departure = find_end_of_line(sq_hurdle,vec_departure_hurdle);
  PieNam const hopper = get_walk_of_piece_on_square(sq_departure);

  return (hopper==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_target,sq_target));
}

/* Is a particular square observed by a particular type of angle hopper?
 * @param sq_target the square
 * @param kanf first vectors index
 * @param kend last vectors index
 * @param angle angle to take from hurdle to arrival squares
 * @param p type of piece
 */
boolean angle_hoppers_is_square_observed(square sq_target,
                                         vec_index_type kanf, vec_index_type kend,
                                         angle_t angle,
                                         PieNam p,
                                         evalfunction_t *evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TracePiece(p);
  TraceFunctionParamListEnd();

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = kend;
       interceptable_observation_vector_index[observation_context]>=kanf;
       --interceptable_observation_vector_index[observation_context])
  {
    numvec const vec_hurdle_target = vec[interceptable_observation_vector_index[observation_context]];
    square const sq_hurdle = sq_target+vec_hurdle_target;

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      vec_index_type const vec_index_departure_hurdle = 2*interceptable_observation_vector_index[observation_context];

      if (angle_hoppers_is_square_observed_one_dir(sq_target,
                                                   sq_hurdle,
                                                   vec_index_departure_hurdle,
                                                   angle,
                                                   p,
                                                   evaluate)
          || angle_hoppers_is_square_observed_one_dir(sq_target,
                                                      sq_hurdle,
                                                      vec_index_departure_hurdle-1,
                                                      angle,
                                                      p,
                                                      evaluate))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
