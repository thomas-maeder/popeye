#include "pieces/walks/orphan.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 */
void orphan_generate_moves(square sq_departure)
{
  numecoup const save_nbcou = current_move[nbply]-1;
  PieNam const *orphan_observer;

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; ++orphan_observer)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0)
    {
      boolean found_chain;

      siblingply(trait[nbply]);
      current_move[nbply] = current_move[nbply-1]+1;
      move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;
      found_chain = orphan_find_observation_chain(sq_departure,*orphan_observer,&validate_observation);
      finply();

      if (found_chain)
        generate_moves_for_piece_based_on_walk(sq_departure,*orphan_observer);
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}
