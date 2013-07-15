#include "pieces/walks/pawns/super.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

static void superpawn_generate_noncaptures(square sq_departure, numvec dir)
{
  square sq_arrival= sq_departure+dir;

  for (; is_square_empty(sq_arrival); sq_arrival+= dir)
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
}

static void superpawn_generate_captures(square sq_departure, numvec dir)
{
  square const sq_arrival = find_end_of_line(sq_departure,dir);

  /* it can move from first rank */
  if (piece_belongs_to_opponent(sq_arrival))
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
}

/* Generate moves for a Super Pawn
 * @param sq_departure common departure square of the generated moves
 */
void super_pawn_generate_moves(square sq_departure)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(sq_departure,dir_forward);
  superpawn_generate_captures(sq_departure,dir_forward+dir_left);
  superpawn_generate_captures(sq_departure,dir_forward+dir_right);
}

/* Generate moves for a Super-Berolina Pawn
 * @param sq_departure common departure square of the generated moves
 */
void super_berolina_pawn_generate_moves(square sq_departure)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(sq_departure,dir_forward+dir_left);
  superpawn_generate_noncaptures(sq_departure,dir_forward+dir_right);
  superpawn_generate_captures(sq_departure,dir_forward);
}
