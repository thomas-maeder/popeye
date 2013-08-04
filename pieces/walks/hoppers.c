#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

#include <stdlib.h>

/* Generate moves for a hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);

    if (!is_square_blocked(sq_hurdle))
    {
      curr_generation->arrival = sq_hurdle+vec[k];
      if (piece_belongs_to_opponent(curr_generation->arrival)
          || is_square_empty(curr_generation->arrival))
      {
        curr_generation->auxiliary.hopper.vec_index = k;
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        push_move();
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
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
      if (piece_belongs_to_opponent(curr_generation->arrival)
          || is_square_empty(curr_generation->arrival))
      {
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        curr_generation->auxiliary.hopper.vec_index = k;
        push_move();
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
}

/* Generate moves for a double hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void doublehopper_generate_moves(vec_index_type vec_start,
                                 vec_index_type vec_end)
{
  numecoup const save_nbcou = current_move[nbply]-1;

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
            if (is_square_empty(curr_generation->arrival)
                || piece_belongs_to_opponent(curr_generation->arrival))
              push_move();
          }
        }
      }
    }
  }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

/* Generate moves for a contra-grasshopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void contra_grasshopper_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;
  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_hurdle = curr_generation->departure+vec[k];
    if (!is_square_empty(sq_hurdle))
    {
      if (!is_square_blocked(sq_hurdle))
      {
        curr_generation->arrival = sq_hurdle+vec[k];
        while (is_square_empty(curr_generation->arrival))
        {
          curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
          push_move();
          curr_generation->arrival += vec[k];
        }

        if (piece_belongs_to_opponent(curr_generation->arrival))
        {
          curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
          push_move();
        }
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
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
      if (piece_belongs_to_opponent(curr_generation->arrival)
          || is_square_empty(curr_generation->arrival))
      {
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        curr_generation->auxiliary.hopper.vec_index = k;
        push_move();
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
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
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        curr_generation->auxiliary.hopper.vec_index = k;
        push_move();
      }
      else if (dist_hurdle_end==dist_hurdle_dep)
      {
        curr_generation->arrival = end_of_line;
        if (piece_belongs_to_opponent(curr_generation->arrival))
        {
          curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
          curr_generation->auxiliary.hopper.vec_index = k;
          push_move();
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle = sq_departure+vec[k];
    curr_generation->arrival = sq_departure + 2*vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle)>=King
        && (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival)))
    {
      curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
      curr_generation->auxiliary.hopper.vec_index = k;
      push_move();
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
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

        if (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival))
        {
          curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
          push_move();
        }
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
        push_move();

      {
        square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
        if (!is_square_blocked(sq_hurdle2)
            && abs(sq_hurdle2-sq_hurdle1)==abs(sq_hurdle1-sq_departure)
            && piece_belongs_to_opponent(sq_hurdle1))
        {
          square const sq_arrival = sq_hurdle1;
          curr_generation->arrival = sq_arrival;
          push_move();
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle1 = sq_departure+2*vec[k];
    curr_generation->arrival = sq_departure+vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle1)>=King
        && (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival)))
      push_move();
  }
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
          && !is_square_empty(sq_hurdle)
          && (is_square_empty(curr_generation->arrival)
              || piece_belongs_to_opponent(curr_generation->arrival)))
        push_move();
    }
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
      {
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        curr_generation->auxiliary.hopper.vec_index = k;
        push_move();
      }
      else if (abs(sq_end_of_line-sq_hurdle) == abs(sq_hurdle-sq_departure)
               && piece_belongs_to_opponent(sq_end_of_line))
      {
        curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
        curr_generation->auxiliary.hopper.vec_index = k;
        curr_generation->arrival = sq_end_of_line;
        push_move();
      }
    }
  }

  curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
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

        if (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival))
        {
          curr_generation->auxiliary.hopper.sq_hurdle = sq_hurdle;
          push_move();
        }
      }
    }
}
