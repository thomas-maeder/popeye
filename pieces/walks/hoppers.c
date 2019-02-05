#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
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
  numecoup const save_nbcou = CURRMOVE_OF_PLY(nbply);

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

  remove_duplicate_moves_of_single_piece(save_nbcou);
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
  if (orix_check(evaluate))
    return true;

  interceptable_observation[observation_context+1].vector_index1 = 0;

  {
    vec_index_type  k;
    for (k = vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)      /* 2,4; 2,6; 4,6; */
    {
      square const sq_hurdle = sq_target+vec[k];
      square const sq_departure = sq_hurdle+vec[k];
      if (!is_square_empty(sq_hurdle)
          && !is_square_blocked(sq_hurdle)
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
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

  square const coin= coinequis(sq_target);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {
      sq_hurdle= coin+delta_horiz+delta_vert;
      vector= sq_target-sq_hurdle;
      sq_departure= sq_hurdle-vector;

      if (!is_square_empty(sq_hurdle)
          && sq_target!=sq_departure
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }

  return false;
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
        push_move_no_capture();

      {
        square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
        if (!is_square_blocked(sq_hurdle2)
            && abs(sq_hurdle2-sq_hurdle1)==abs(sq_hurdle1-sq_departure)
            && piece_belongs_to_opponent(sq_hurdle1))
        {
          square const sq_arrival = sq_hurdle1;
          curr_generation->arrival = sq_arrival;
          push_move_regular_capture();
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

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_target,-vec[interceptable_observation[observation_context].vector_index1]);
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
      if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle)
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }

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
      if (sq_departure-sq_hurdle==sq_hurdle-sq_target
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

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
