#include "pieces/angle/hoppers.h"
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
                                  angle_t angle,
                                  Side side)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  for (k = kend; k>=kanf; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      vec_index_type const k1 = 2*k;

      {
        square const sq_arrival= sq_hurdle+angle_vectors[angle][k1];
        if ((is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,side))
            && angle_hoppers_imok(sq_departure,sq_arrival,sq_hurdle,vec[k],angle_vectors[angle][k1]))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }

      {
        square const sq_arrival= sq_hurdle+angle_vectors[angle][k1-1];
        if ((is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,side))
            && angle_hoppers_imok(sq_departure,sq_arrival,sq_hurdle,vec[k],angle_vectors[angle][k1-1]))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  vec_index_type k;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TracePiece(p);
  TraceFunctionParamListEnd();

  for (k = kend; k>=kanf; --k)
  {
    numvec const v = vec[k];
    square const sq_hurdle = sq_target+v;
    TraceValue("%d",v);
    TraceSquare(sq_hurdle);
    TracePiece(e[sq_hurdle]);
    TraceText("\n");
    if (get_walk_of_piece_on_square(sq_hurdle)>=King)
    {
      vec_index_type k1 = 2*k;

      {
        numvec const v1 = angle_vectors[angle][k1];
        square const sq_departure = find_end_of_line(sq_hurdle,v1);
        PieNam const hopper = get_walk_of_piece_on_square(sq_departure);
        TraceSquare(sq_departure);
        TracePiece(hopper);
        TraceValue("%d\n",v1);
        if (hopper==p && TSTFLAG(spec[sq_departure],trait[nbply]))
        {
          if (evaluate(sq_departure,sq_target,sq_target)
              && angle_hoppers_imok(sq_departure,sq_target,sq_hurdle,-v1,-v))
          {
            result = true;
            break;
          }
        }
      }

      {
        numvec const v1 = angle_vectors[angle][k1-1];
        square const sq_departure = find_end_of_line(sq_hurdle,v1);
        PieNam const hopper = get_walk_of_piece_on_square(sq_departure);
        TraceSquare(sq_departure);
        TracePiece(hopper);
        TraceValue("%d\n",v1);
        if (hopper==p && TSTFLAG(spec[sq_departure],trait[nbply])) {
          if (evaluate(sq_departure,sq_target,sq_target)
              && angle_hoppers_imok(sq_departure,sq_target,sq_hurdle,-v1,-v))
          {
            result = true;
            break;
          }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
