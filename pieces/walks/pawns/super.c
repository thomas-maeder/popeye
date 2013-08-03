#include "pieces/walks/pawns/super.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

static void superpawn_generate_noncaptures(numvec dir)
{
  square sq_arrival;

  for (sq_arrival = curr_generation->departure+dir;
       is_square_empty(sq_arrival);
       sq_arrival+= dir)
    push_move_generation(sq_arrival);
}

static void superpawn_generate_captures(numvec dir)
{
  square const sq_arrival = find_end_of_line(curr_generation->departure,dir);

  /* it can move from first rank */
  if (piece_belongs_to_opponent(sq_arrival))
    push_move_generation(sq_arrival);
}

/* Generate moves for a Super Pawn
 */
void super_pawn_generate_moves(void)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(dir_forward);
  superpawn_generate_captures(dir_forward+dir_left);
  superpawn_generate_captures(dir_forward+dir_right);
}

/* Generate moves for a Super-Berolina Pawn
 */
void super_berolina_pawn_generate_moves(void)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(dir_forward+dir_left);
  superpawn_generate_noncaptures(dir_forward+dir_right);
  superpawn_generate_captures(dir_forward);
}
