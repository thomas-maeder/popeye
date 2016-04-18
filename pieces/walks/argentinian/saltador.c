#include "pieces/walks/argentinian/saltador.h"
#include "pieces/walks/chinese/mao.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

static boolean is_false(numecoup n)
{
  return false;
}

static void maooahopper_generate_moves(vec_index_type k,
                                       numvec to_intermediate)
{
  numvec const to_arrival = vec[k];
  square const sq_departure = curr_generation->departure;
  square const sq_hurdle = sq_departure+to_intermediate;

  if (!is_square_empty(sq_hurdle))
  {
    curr_generation->arrival = sq_departure+to_arrival;
    if (is_square_empty(curr_generation->arrival)
        || piece_belongs_to_opponent(curr_generation->arrival))
      hoppers_push_move(k,sq_hurdle);
  }
}

static void moahopper_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    maooahopper_generate_moves(k,moa_intermediate_vector(vec[k]));
}

static void maohopper_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    maooahopper_generate_moves(k,mao_intermediate_vector(vec[k]));
}

/* Generate moves for a Saltador
 */
void saltador_generate_moves(void)
{
  {
    numecoup const save_start_of_noncaptures = CURRMOVE_OF_PLY(nbply);
    maohopper_generate_moves();
    moahopper_generate_moves();
    move_generator_filter_captures(save_start_of_noncaptures,&is_false);
    remove_duplicate_moves_of_single_piece(save_start_of_noncaptures);
  }

  {
    numecoup const save_start_of_captures = CURRMOVE_OF_PLY(nbply);
    mao_generate_moves();
    moa_generate_moves();
    move_generator_filter_noncaptures(save_start_of_captures,&is_false);
    remove_duplicate_moves_of_single_piece(save_start_of_captures);
  }
}

boolean saltador_check(validator_id evaluate)
{
  return mao_check(evaluate) || moa_check(evaluate);
}
