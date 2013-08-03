#include "pieces/walks/cardinal.h"
#include "pieces/walks/angle/angles.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void cardinal_generate_moves_recursive(square in, numvec k, int x)
{
  int k1;
  square sq_arrival = in+k;

  while (is_square_empty(sq_arrival))
  {
    push_move_generation(sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(sq_arrival))
    push_move_generation(sq_arrival);
  else if (x && is_square_blocked(sq_arrival))
  {
    for (k1= 1; k1<=4; k1++)
      if (!is_square_blocked(sq_arrival+vec[k1]))
        break;

    if (k1<=4)
    {
      sq_arrival+= vec[k1];
      if (piece_belongs_to_opponent(sq_arrival))
        push_move_generation(sq_arrival);
      else if (is_square_empty(sq_arrival))
      {
        push_move_generation(sq_arrival);
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (is_square_blocked(sq_arrival+angle_vectors[angle_90][k1]))
          k1--;

        cardinal_generate_moves_recursive(sq_arrival,angle_vectors[angle_90][k1],x-1);
      }
    }
  }
}

/* Generate moves for a Cardinal
 */
void cardinal_generate_moves(void)
{
  vec_index_type k;
  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    cardinal_generate_moves_recursive(curr_generation->departure,vec[k],1);
}
