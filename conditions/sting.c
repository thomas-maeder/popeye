#include "conditions/sting.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void sting_generate_moves_for_piece(slice_index si,
                                    square sq_departure,
                                    PieNam p)
{
  if (p==King)
    hoppers_generate_moves(sq_departure,vec_queen_start,vec_queen_end);

  generate_moves_for_piece(slices[si].next1,sq_departure,p);
}

/* Initialise the solving machinery with Sting Chess
 * @param si root slice of the solving machinery
 */
void sting_initalise_solving(slice_index si)
{
  solving_instrument_move_generation(si,nr_sides,STStingMovesForPieceGenerator);
}
