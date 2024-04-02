#include "pieces/walks/hoppers.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/promotion.h"
#include "position/effects/flags_change.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "solving/pipe.h"
#include "solving/post_move_iteration.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <stdlib.h>

hoppper_moves_auxiliary_type hoppper_moves_auxiliary[toppile+1];

/* push current move, all while remembering the current hurdle and move direction
 * @param idx_dir index into vec
 * @param sq_hurdle hurdle for the move being pushed
 */
void hoppers_push_move(vec_index_type idx_dir, square sq_hurdle)
{
  push_move_no_capture();
  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].idx_dir = idx_dir;
  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
}

void hoppers_push_capture(vec_index_type idx_dir, square sq_hurdle)
{
  push_move_regular_capture();
  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].idx_dir = idx_dir;
  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
}

/* Clear hurdle information in the moves from the current move of ply nbply
 * down to (but not including) some base move.
 * To be used while generating moves for walks that can do both moves using a
 * hurdle and moves that don't
 * @param base index of the base move
 */
void hoppers_clear_hurdles(numecoup base)
{
  numecoup i;
  for (i = current_move[nbply]; i!=base; --i)
    hoppper_moves_auxiliary[move_generation_stack[i].id].sq_hurdle = initsquare;
}

/* Generate moves for a hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void rider_hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);

    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle+vec[k];
      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
      else if (is_square_empty(curr_generation->arrival))
        hoppers_push_move(k,sq_hurdle);
    }
  }
}

boolean rider_hoppers_check(vec_index_type kanf, vec_index_type kend,
                            validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1<=kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = sq_target+vec[interceptable_observation[observation_context].vector_index1];

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation[observation_context].vector_index1]);

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

boolean grasshopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_queen_start, vec_queen_end, evaluate);
}

boolean nightrider_hopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_knight_start,vec_knight_end, evaluate);
}

boolean camelrider_hopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean zebrarider_hopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_zebre_start, vec_zebre_end, evaluate);
}

boolean gnurider_hopper_check(validator_id evaluate)
{
  return  (rider_hoppers_check(vec_knight_start,vec_knight_end, evaluate)
           || rider_hoppers_check(vec_chameau_start, vec_chameau_end, evaluate));
}

boolean rookhopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean bishopper_check(validator_id evaluate)
{
  return rider_hoppers_check(vec_bishop_start,vec_bishop_end, evaluate);
}

/* Generate moves for a leaper hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = curr_generation->departure+vec[k];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle+vec[k];
      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
      else if (is_square_empty(curr_generation->arrival))
        hoppers_push_move(k,sq_hurdle);
    }
  }
}

static boolean leaper_hoppers_check(vec_index_type kanf, vec_index_type kend,
                                    validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1 <= kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = sq_target+vec[interceptable_observation[observation_context].vector_index1];

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = sq_hurdle+vec[interceptable_observation[observation_context].vector_index1];

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

boolean kinghopper_check(validator_id evaluate)
{
  return leaper_hoppers_check(vec_queen_start, vec_queen_end, evaluate);
}

boolean knighthopper_check(validator_id evaluate)
{
  return leaper_hoppers_check(vec_knight_start, vec_knight_end, evaluate);
}

/* Generate moves for a double hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void doublehopper_generate_moves(vec_index_type vec_start,
                                 vec_index_type vec_end)
{
  vec_index_type k;
  for (k = vec_end; k>=vec_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      square const past_sq_hurdle1 = sq_hurdle1+vec[k];
      if (is_square_empty(past_sq_hurdle1))
      {
        vec_index_type k1;
        for (k1 = vec_end; k1>=vec_start; k1--)
        {
          square const sq_hurdle2 = find_end_of_line(past_sq_hurdle1,vec[k1]);
          if (!is_square_blocked(sq_hurdle2))
          {
            curr_generation->arrival = sq_hurdle2+vec[k1];
            if (curr_generation->arrival!=curr_generation->departure)
            {
              if (is_square_empty(curr_generation->arrival))
                push_move_no_capture();
              else if (piece_belongs_to_opponent(curr_generation->arrival))
                push_move_regular_capture();
            }
          }
        }
      }
    }
  }
}

static boolean doublehopper_check(vec_index_type vec_start, vec_index_type vec_end,
                                  validator_id evaluate)
{
  boolean result = false;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index2 = vec_end;
       interceptable_observation[observation_context].vector_index2>=vec_start && !result;
       interceptable_observation[observation_context].vector_index2--)
  {
    numvec const dir2 = vec[interceptable_observation[observation_context].vector_index2];
    square const sq_hurdle2 = sq_target+dir2;
    if (!is_square_empty(sq_hurdle2) && !is_square_blocked(sq_hurdle2))
      for (interceptable_observation[observation_context].auxiliary = sq_hurdle2+dir2;
           is_square_empty(interceptable_observation[observation_context].auxiliary);
           interceptable_observation[observation_context].auxiliary += dir2)
        for (interceptable_observation[observation_context].vector_index1 = vec_end;
             interceptable_observation[observation_context].vector_index1>=vec_start;
             interceptable_observation[observation_context].vector_index1--)
        {
          numvec const dir1 = vec[interceptable_observation[observation_context].vector_index1];
          square const sq_hurdle1 = interceptable_observation[observation_context].auxiliary+dir1;
          if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
          {
            square const sq_departure = find_end_of_line(sq_hurdle1,dir1);
            if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
            {
              result = true;
              break;
            }
          }
        }
  }

  --observation_context;

  return result;
}

boolean doublegrasshopper_check(validator_id evaluate)
{
  /* W.B.Trumper feenschach 1968 - but null moves will not be allowed by Popeye
   */
  return doublehopper_check(vec_queen_start,vec_queen_end,evaluate);
}

boolean doublerookhopper_check(validator_id evaluate)
{
  return doublehopper_check(vec_rook_start,vec_rook_end,evaluate);
}

boolean doublebishopper_check(validator_id evaluate)
{
  return doublehopper_check(vec_bishop_start,vec_bishop_end,evaluate);
}

/* Generate moves for a contra-grasshopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void contra_grasshopper_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kbeg);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = curr_generation->departure+vec[k];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle+vec[k];
      while (is_square_empty(curr_generation->arrival))
      {
        hoppers_push_move(k,sq_hurdle);
        curr_generation->arrival += vec[k];
      }

      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean contragrasshopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_start;
       interceptable_observation[observation_context].vector_index1<=vec_queen_end;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = sq_hurdle+vec[interceptable_observation[observation_context].vector_index1];

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

static square grasshoppers_n_find_target(square sq_hurdle,
                                         numvec dir,
                                         unsigned int dist_hurdle_target)
{
  square result = sq_hurdle;

  unsigned int dist_remaining = dist_hurdle_target;
  while (--dist_remaining)
  {
    result += dir;
    if (!is_square_empty(result))
      return initsquare;
  }

  result += dir;

  return result;
}

/* Generate moves for a grasshopper-n piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void grasshoppers_n_generate_moves(vec_index_type kbeg, vec_index_type kend,
                                   unsigned int dist_hurdle_target)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);

    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = grasshoppers_n_find_target(sq_hurdle,vec[k],dist_hurdle_target);
      if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
      else if (is_square_empty(curr_generation->arrival))
        hoppers_push_move(k,sq_hurdle);
    }
  }
}

static square grasshoppers_n_find_hurdle(square sq_target,
                                         numvec dir,
                                         unsigned int dist_hurdle_target)
{
  square result = sq_target;
  unsigned int dist_remaining = dist_hurdle_target;
  while (--dist_remaining>0)
  {
    result += dir;
    if (!is_square_empty(result))
      return initsquare;
  }

  result += dir;

  return result;
}

static boolean grasshoppers_n_check(vec_index_type kanf, vec_index_type kend,
                                    unsigned int dist_hurdle_target,
                                    validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1<=kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = grasshoppers_n_find_hurdle(sq_target,vec[interceptable_observation[observation_context].vector_index1],dist_hurdle_target);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation[observation_context].vector_index1]);

      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean grasshopper_2_check(validator_id evaluate)
{
  return grasshoppers_n_check(vec_queen_start, vec_queen_end, 2, evaluate);
}

boolean grasshopper_3_check(validator_id evaluate)
{
  return grasshoppers_n_check(vec_queen_start, vec_queen_end, 3, evaluate);
}

/* Generate moves for an equihopper
 */
void equihopper_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const end_of_line = find_end_of_line(sq_hurdle,vec[k]);
      int const dist_hurdle_end = abs(end_of_line-sq_hurdle);
      int const dist_hurdle_dep = abs(sq_hurdle-sq_departure);
      if (dist_hurdle_end>dist_hurdle_dep)
      {
        curr_generation->arrival = sq_hurdle+sq_hurdle-sq_departure;
        hoppers_push_move(k,sq_hurdle);
      }
      else if (dist_hurdle_end==dist_hurdle_dep)
      {
        curr_generation->arrival = end_of_line;
        if (piece_belongs_to_opponent(curr_generation->arrival))
          hoppers_push_capture(k,sq_hurdle);
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle = sq_departure+vec[k];
    curr_generation->arrival = sq_departure + 2*vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle)>=King)
    {
      if (is_square_empty(curr_generation->arrival))
        hoppers_push_move(k,sq_hurdle);
      else if (piece_belongs_to_opponent(curr_generation->arrival))
        hoppers_push_capture(k,sq_hurdle);
    }
  }
}

boolean equihopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  if (orix_check(evaluate))
    result = true;
  else
  {
    ++observation_context;

    interceptable_observation[observation_context+1].vector_index1 = 0;

    {
      vec_index_type  k;
      for (k = vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)      /* 2,4; 2,6; 4,6; */
      {
        square const sq_hurdle = sq_target+vec[k];
        square const sq_departure = sq_hurdle+vec[k];

        hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
        if (!is_square_empty(sq_hurdle)
            && !is_square_blocked(sq_hurdle)
            && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        {
          result = true;
          break;
        }
      }
    }

    hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

    --observation_context;
  }

  return result;
}

static square coinequis(square i)
{
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));
}

/* Generate moves for an nonstop-equihopper
 */
void nonstop_equihopper_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  square const coin = coinequis(sq_departure);
  numvec delta_horiz;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)
  {
    numvec delta_vert;
    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down)
    {
      square const sq_hurdle = coin+delta_horiz+delta_vert;

      if (sq_hurdle!=sq_departure /* prevent nNE from capturing itself */
          && !is_square_empty(sq_hurdle))
      {
        numvec const vector = sq_hurdle-sq_departure;
        curr_generation->arrival = sq_hurdle+vector;

        if (is_square_empty(curr_generation->arrival))
          hoppers_push_move(0,sq_hurdle);
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          hoppers_push_capture(0,sq_hurdle);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean nonstop_equihopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  numvec delta_horiz, delta_vert;
  numvec vector;
  square sq_hurdle;
  square sq_departure;
  boolean result = false;

  square const coin= coinequis(sq_target);

  ++observation_context;

  for (delta_horiz= 3*dir_right;
       !result && delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {
      sq_hurdle= coin+delta_horiz+delta_vert;
      vector= sq_target-sq_hurdle;
      sq_departure= sq_hurdle-vector;

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (!is_square_empty(sq_hurdle)
          && sq_target!=sq_departure
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

void equistopper_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle1))
    {
      curr_generation->arrival = (sq_hurdle1+sq_departure)/2;
      if (!((sq_hurdle1/onerow+sq_departure/onerow)%2
            || (sq_hurdle1%onerow+sq_departure%onerow)%2)) /* is sq_arrival a square? */
        hoppers_push_move(k,sq_hurdle1);

      {
        square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
        if (!is_square_blocked(sq_hurdle2)
            && abs(sq_hurdle2-sq_hurdle1)==abs(sq_hurdle1-sq_departure)
            && piece_belongs_to_opponent(sq_hurdle1))
        {
          square const sq_arrival = sq_hurdle1;
          curr_generation->arrival = sq_arrival;
          hoppers_push_capture(k,sq_hurdle2);
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle1 = sq_departure+2*vec[k];
    curr_generation->arrival = sq_departure+vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle1)>=King)
    {
      if  (is_square_empty(curr_generation->arrival))
        push_move_no_capture();
      else if (piece_belongs_to_opponent(curr_generation->arrival))
        push_move_regular_capture();
    }
  }
}

boolean equistopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_target,-vec[interceptable_observation[observation_context].vector_index1]);
      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (sq_departure-sq_target==sq_target-sq_hurdle
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  if (!result)
    for (interceptable_observation[observation_context].vector_index1 = vec_equi_nonintercept_start;
        interceptable_observation[observation_context].vector_index1<=vec_equi_nonintercept_end;
        interceptable_observation[observation_context].vector_index1++)      /* 2,4; 2,6; 4,6; */
    {
      square const sq_departure = sq_target-vec[interceptable_observation[observation_context].vector_index1];
      square const sq_hurdle = sq_target+vec[interceptable_observation[observation_context].vector_index1];
      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle)
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

/* Generate moves for an nonstop-equistopper
 * @param sq_departure common departure square of the generated moves
 */
void nonstop_equistopper_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  square sq_hurdle;
  numvec delta_horiz, delta_vert;
  numvec vector;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      curr_generation->arrival = coin+delta_horiz+delta_vert;
      vector= curr_generation->arrival-sq_departure;
      sq_hurdle= curr_generation->arrival+vector;

      if (curr_generation->arrival!=sq_departure
          && !is_square_empty(sq_hurdle))
      {
        if (is_square_empty(curr_generation->arrival))
          push_move_no_capture();
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          push_move_regular_capture();
      }
    }
}

boolean nonstop_equistopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  /* check by non-stop equistopper? */
  square sq_hurdle;
  square const *bnp;
  numvec vector;
  square sq_departure;

  for (bnp= boardnum; *bnp; bnp++)
  {
    sq_departure= *bnp;
    vector= sq_target-sq_departure;
    sq_hurdle= sq_target+vector;
    if (!is_square_empty(sq_hurdle)
        && !is_square_blocked(sq_hurdle)
        && sq_target!=sq_departure
        && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

/* Generate moves for an Orix
 */
void orix_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_end_of_line = find_end_of_line(sq_hurdle,vec[k]);
      curr_generation->arrival = sq_hurdle+sq_hurdle-sq_departure;
      if (abs(sq_end_of_line-sq_hurdle) > abs(sq_hurdle-sq_departure))
        hoppers_push_move(k,sq_hurdle);
      else if (abs(sq_end_of_line-sq_hurdle) == abs(sq_hurdle-sq_departure)
               && piece_belongs_to_opponent(sq_end_of_line))
      {
        curr_generation->arrival = sq_end_of_line;
        hoppers_push_capture(k,sq_hurdle);
      }
    }
  }
}

boolean orix_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation[observation_context].vector_index1]);
      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
      if (sq_departure-sq_hurdle==sq_hurdle-sq_target
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  return result;
}

/* Generate moves for an nonstop-Orix
 */
void nonstop_orix_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;
  square sq_hurdle;
  numvec delta_horiz, delta_vert, delta;
  numvec vector;
  boolean queenlike;

  square const coin = coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_hurdle= coin+delta_horiz+delta_vert;
      delta= abs(sq_hurdle - sq_departure);
      queenlike= (delta <= 3*dir_right)
                  || (delta % onerow == 0)
                  || (delta % (onerow + dir_right) == 0)
                  || (delta % (onerow + dir_left) == 0);

      if (queenlike && !is_square_empty(sq_hurdle))
      {
        vector= sq_hurdle-sq_departure;
        curr_generation->arrival = sq_hurdle+vector;

        if (is_square_empty(curr_generation->arrival))
          hoppers_push_move(0,sq_hurdle);
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          hoppers_push_capture(0,sq_hurdle);
      }
    }
}

boolean nonstop_orix_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  numvec delta_horiz, delta_vert;

  square const coin = coinequis(sq_target);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down)
    {
      square const sq_hurdle= coin+delta_horiz+delta_vert;
      numvec const vector = sq_target-sq_hurdle;
      numvec const delta = abs(vector);
      boolean const queenlike = (delta <= 3*dir_right)
                                 || (delta % onerow == 0)
                                 || (delta % (onerow + dir_right) == 0)
                                 || (delta % (onerow + dir_left) == 0);
      square const sq_departure = sq_hurdle-vector;

      if (queenlike
          && !is_square_empty(sq_hurdle)
          && sq_target!=sq_departure
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }

  return false;
}


enum
{
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static unsigned int stack_pointer;

static move_effect_journal_index_type horizon;

static piece_flag_type const options_masks[] = {
    0,
    BIT(ColourChange),
    BIT(Bul),
    BIT(Dob),
    BIT(ColourChange)|BIT(Bul),
    BIT(ColourChange)|BIT(Dob)
};

enum {
  nr_masks = sizeof options_masks / sizeof options_masks[0]
};

static boolean promote_walk_into[nr_piece_walks][nr_masks];
static unsigned int promote_walk_into_nr[nr_piece_walks];

static unsigned int next_prom_to_changing_happening[stack_size];

/* Find a promotion in the effects of the move being played since we last looked
 * @param base start of set of effects where to look for a promotion
 * @return index of promotion effect; base if there is none
 */
static move_effect_journal_index_type find_promotion(move_effect_journal_index_type base)
{
  move_effect_journal_index_type curr = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result = base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  while (curr>base)
  {
    --curr;

    if (move_effect_journal[curr].type==move_effect_walk_change
        && move_effect_journal[curr].reason==move_effect_reason_pawn_promotion)
    {
      result = curr;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate solving to the next slice, while remembering the set of effects of
 * this move where prommotions have been considered.
 * @param si identifies the current slice
 */
static void solve_nested(slice_index si)
{
  move_effect_journal_index_type const save_horizon = horizon;

  horizon = move_effect_journal_base[nbply+1];
  ++stack_pointer;
  pipe_solve_delegate(si);
  --stack_pointer;
  horizon = save_horizon;
}

/* Delegate solving to the next slice, while remembering the set of effects of
 * this move where prommotions have been considered.
 * @param si identifies the current slice
 */
static void solve_nested_iterating(slice_index si)
{
  move_effect_journal_index_type const save_horizon = horizon;

  horizon = move_effect_journal_base[nbply+1];
  ++stack_pointer;
  post_move_iteration_solve_delegate(si);
  --stack_pointer;
  horizon = save_horizon;
}

/* start or continue an iteration over leaving non-changing and changing to
 * changing
 * @param si identifies the current slice
 * @param idx_promotion index of the promotion effect
 */
static void iterate_over_possible_options(slice_index si,
                                          square sq_prom,
                                          piece_walk_type walk_promotee)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_prom);
  TraceWalk(walk_promotee);
  TraceFunctionParamListEnd();

  if (!post_move_am_i_iterating())
  {
    next_prom_to_changing_happening[stack_pointer] = 0;

    while (next_prom_to_changing_happening[stack_pointer]!=nr_masks
           && !promote_walk_into[walk_promotee][next_prom_to_changing_happening[stack_pointer]])
      ++next_prom_to_changing_happening[stack_pointer];
  }

  if (next_prom_to_changing_happening[stack_pointer]==nr_masks)
    post_move_iteration_end();
  else if (options_masks[next_prom_to_changing_happening[stack_pointer]]==0)
    solve_nested_iterating(si);
  else
  {
    Flags changed = being_solved.spec[sq_prom];
    SETFLAGMASK(changed,options_masks[next_prom_to_changing_happening[stack_pointer]]);
    move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                        sq_prom,
                                        changed);
    solve_nested_iterating(si);
  }

  if (post_move_have_i_lock())
    do
    {
      ++next_prom_to_changing_happening[stack_pointer];
      if (next_prom_to_changing_happening[stack_pointer]==nr_masks)
      {
        post_move_iteration_end();
        break;
      }
    } while (!promote_walk_into[walk_promotee][next_prom_to_changing_happening[stack_pointer]]);

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
void hopper_attribute_specific_promotion_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const idx_promotion = find_promotion(horizon);
    if (idx_promotion==horizon)
      /* no promotion */
      solve_nested(si);
    else
    {
      piece_walk_type const walk_promotee = move_effect_journal[idx_promotion].u.piece_walk_change.to;
      square const sq_prom = move_effect_journal[idx_promotion].u.piece_walk_change.on;
      iterate_over_possible_options(si,sq_prom,walk_promotee);
    }
  }

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
void hopper_attribute_specific_promotion_initialiser_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    piece_walk_type p;
    for (p = King; p!=nr_piece_walks; ++p)
    {
      unsigned int i;
      for (i = 0; i!=nr_masks; ++i)
        promote_walk_into[p][i] = false;

      promote_walk_into_nr[p] = 0;
    }

    for (p = King; p<=Bishop; ++p)
    {
      promote_walk_into[standard_walks[p]][0] = true;
      promote_walk_into_nr[standard_walks[p]] = 1;
    }
  }

  {
    piece_flag_type const mask = BIT(ColourChange)|BIT(Bul)|BIT(Dob);
    square const *s;
    for (s = boardnum; *s; ++s)
    {
      piece_walk_type const p = get_walk_of_piece_on_square(*s);
      if (p!=Empty)
      {
        piece_flag_type const flags = TSTFLAGMASK(being_solved.spec[*s],mask);
        unsigned int i;

        TraceWalk(p);
        for (i = 0; i!=nr_masks; ++i)
          if (flags==options_masks[i])
          {
            TraceValue("%u",i);
            TraceValue("%x",options_masks[i]);
            promote_walk_into[p][i] = true;
            ++promote_walk_into_nr[p];
          }
        TraceEOL();
      }
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_promoter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STHopperAttributeSpecificPromotion);
    promotion_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void solving_insert_hopper_specific_promotions(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STHopperAttributeSpecificPromotionInitialiser);
    slice_insertion_insert(si,&prototype,1);
  }

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STBeforePawnPromotion,&instrument_promoter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
