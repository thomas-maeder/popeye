#include "pieces/walks/orphan.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Orphan
 */
void orphan_generate_moves(void)
{
  numecoup const save_nbcou = current_move[nbply]-1;
  PieNam const *orphan_observer;

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; ++orphan_observer)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0)
    {
      boolean found_chain;

      siblingply(trait[nbply]);
      current_move[nbply] = current_move[nbply-1]+1;
            found_chain = orphan_find_observation_chain(curr_generation->departure,
                                                        *orphan_observer,
                                                        &validate_observation);
      finply();

      if (found_chain)
        generate_moves_for_piece_based_on_walk(*orphan_observer);
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}
