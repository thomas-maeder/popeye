#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

void pawns_generate_ep_capture_move(square sq_departure,
                                    square sq_arrival,
                                    square sq_arrival_singlestep)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_arrival_singlestep);
  TraceFunctionParamListEnd();

  if (get_walk_of_piece_on_square(sq_departure)!=Orphan /* orphans cannot capture ep */
      && en_passant_is_capture_possible_to(trait[nbply],sq_arrival_singlestep))
  {
    square const sq_capture = en_passant_find_capturee();
    if (sq_capture!=initsquare)
    {
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture);
      move_generation_stack[current_move[nbply]].auxiliary.sq_en_passant = sq_arrival_singlestep;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* generates move of a pawn of side camp on departure capturing a piece on
 * arrival
 */
void pawns_generate_capture_move(square sq_departure, numvec dir)
{
  square const sq_arrival = sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (piece_belongs_to_opponent(sq_arrival))
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  else
    pawns_generate_ep_capture_move(sq_departure,sq_arrival,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* generates moves of a pawn in direction dir where steps single steps are
 * possible.
 */
void pawns_generate_nocapture_moves(square sq_departure, numvec dir, int steps)
{
  square sq_arrival= sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",steps);
  TraceFunctionParamListEnd();

  while (steps--)
  {
    TraceSquare(sq_arrival);
    TracePiece(e[sq_arrival]);
    TraceText("\n");
    if (is_square_empty(sq_arrival))
    {
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
      sq_arrival += dir;
    }
    else
      break;
    TraceValue("%d\n",steps);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
