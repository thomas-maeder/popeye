#include "pieces/walks/argentinian/saltador.h"
#include "pieces/walks/chinese/mao.h"
#include "solving/move_generator.h"
#include "solving/observation.h"

static boolean is_false(numecoup n)
{
  return false;
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
