#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

void pawns_generate_ep_capture_move(square sq_arrival_singlestep)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival_singlestep);
  TraceFunctionParamListEnd();

  if (get_walk_of_piece_on_square(curr_generation->departure)!=Orphan /* orphans cannot capture ep */
      && en_passant_is_capture_possible_to(trait[nbply],sq_arrival_singlestep))
  {
    square const sq_capture = en_passant_find_capturee();
    if (sq_capture!=initsquare)
      push_special_move(offset_en_passant_capture+sq_capture);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* generates move of a pawn of side camp on departure capturing a piece on
 * arrival
 */
void pawns_generate_capture_move(numvec dir)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  curr_generation->arrival = curr_generation->departure+dir;

  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
  else
    pawns_generate_ep_capture_move(curr_generation->arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* generates moves of a pawn in direction dir where steps single steps are
 * possible.
 */
void pawns_generate_nocapture_moves(numvec dir, int steps)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",steps);
  TraceFunctionParamListEnd();

  curr_generation->arrival = curr_generation->departure+dir;

  if (is_square_empty(curr_generation->arrival))
  {
    push_move_no_capture();
    curr_generation->arrival += dir;

    while (--steps>0)
      if (is_square_empty(curr_generation->arrival))
      {
        push_special_move(pawn_multistep);
        curr_generation->arrival += dir;
      }
      else
        break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
