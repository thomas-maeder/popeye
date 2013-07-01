#include "pieces/spiral_springers.h"
#include "pydata.h"
#include "pyproc.h"
#include "debugging/trace.h"

static void generate_leg(Side side, square sq_departure,
                         numvec zig, numvec zag)
{
  square sq_arrival = sq_departure+zig;

  while (is_square_empty(sq_arrival))
  {
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    sq_arrival += zag;
    if (is_square_empty(sq_arrival))
    {
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
      sq_arrival += zig;
    }
    else
      break;
  }

  if (piece_belongs_to_opponent(sq_arrival,side))
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
}

static void generate_in_direction(Side side, square sq_departure,
                                  numvec zig, numvec zag)
{
  generate_leg(side,sq_departure,zig,zag);
  generate_leg(side,sq_departure,zig,-zag);
}

void spiralspringer_generate_moves(Side side, square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type const top = vec_knight_start+vec_knight_end;
  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    generate_in_direction(side,sq_departure,vec[k],vec[top-k]);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void diagonalspiralspringer_generate_moves(Side side, square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_in_direction(side,sq_departure,vec[k],vec[k+1]);
    generate_in_direction(side,sq_departure,vec[k+1],vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void boyscout_generate_moves(Side side, square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type const top = vec_bishop_start+vec_bishop_end;
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
    generate_in_direction(side,sq_departure,vec[k],vec[top-k]);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void girlscout_generate_moves(Side side, square sq_departure)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type const top = vec_rook_start+vec_rook_end;
  vec_index_type k;
  for (k = vec_rook_end; k>=vec_rook_start; --k)
    generate_in_direction(side,sq_departure,vec[k],vec[top-k]);

  remove_duplicate_moves_of_single_piece(save_current_move);
}
