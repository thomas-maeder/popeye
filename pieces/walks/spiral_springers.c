#include "pieces/walks/spiral_springers.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void generate_either_pointed_or_obtuse(numvec zig, numvec zag)
{
  curr_generation->arrival = curr_generation->departure+zig;

  while (is_square_empty(curr_generation->arrival))
  {
    push_move();
    curr_generation->arrival += zag;
    if (is_square_empty(curr_generation->arrival))
    {
      push_move();
      curr_generation->arrival += zig;
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move();
}

static void generate_pointed_and_obtuse(vec_index_type index_zig,
                                        vec_index_type index_zag)
{
  generate_either_pointed_or_obtuse(vec[index_zig],vec[index_zag]);
  generate_either_pointed_or_obtuse(vec[index_zig],-vec[index_zag]);
}

void spiralspringer_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type const top = vec_knight_start+vec_knight_end;
  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    generate_pointed_and_obtuse(k,top-k);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void diagonalspiralspringer_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_pointed_and_obtuse(k,k+1);
    generate_pointed_and_obtuse(k+1,k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void boyscout_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type const top = vec_bishop_start+vec_bishop_end;
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
    generate_pointed_and_obtuse(k,top-k);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void girlscout_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type const top = vec_rook_start+vec_rook_end;
  vec_index_type k;
  for (k = vec_rook_end; k>=vec_rook_start; --k)
    generate_pointed_and_obtuse(k,top-k);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void spiralspringer40_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_either_pointed_or_obtuse(vec[k], vec[k+7]);
    generate_either_pointed_or_obtuse(vec[k+7], vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void spiralspringer20_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_either_pointed_or_obtuse(vec[k], vec[k+3]);
    generate_either_pointed_or_obtuse(vec[k+3], vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void spiralspringer33_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_either_pointed_or_obtuse(vec[k], vec[k+1]);
    generate_either_pointed_or_obtuse(vec[k+1], vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void spiralspringer11_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_either_pointed_or_obtuse(vec[k], vec[k+5]);
    generate_either_pointed_or_obtuse(vec[k+5], vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void quintessence_generate_moves(void)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    generate_either_pointed_or_obtuse(vec[k], vec[k+2]);
    generate_either_pointed_or_obtuse(vec[k+2], vec[k]);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}
