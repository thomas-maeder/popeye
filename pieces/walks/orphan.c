#include "pieces/walks/orphan.h"
#include "pieces/walks/friend.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

PieNam orphanpieces[PieceCount];

static boolean orphan_find_observation_chain(square sq_target,
                                             PieNam orphan_observer);

static boolean find_next_orphan_in_chain(square sq_target,
                                         square const pos_orphans[],
                                         PieNam orphan_observer)
{
  boolean result = false;
  unsigned int orphan_id;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_observer);
  TraceFunctionParamListEnd();

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][Orphan]; ++orphan_id)
  {
    boolean does_orphan_observe;

    isolate_observee(Orphan,pos_orphans,orphan_id);
    move_generation_stack[current_move[nbply]-1].capture = sq_target;
    observing_walk[nbply] = Orphan;
    does_orphan_observe = (*checkfunctions[orphan_observer])(&validate_observer);
    restore_observees(Orphan,pos_orphans);

    if (does_orphan_observe
        && orphan_find_observation_chain(pos_orphans[orphan_id],orphan_observer))
    {
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean orphan_find_observation_chain(square sq_target,
                                             PieNam orphan_observer)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_observer);
  TraceFunctionParamListEnd();

  trait[nbply] = advers(trait[nbply]);

  move_generation_stack[current_move[nbply]-1].capture = sq_target;
  observing_walk[nbply] = orphan_observer;
  if ((*checkfunctions[orphan_observer])(&validate_observer))
    result = true;
  else if (number_of_pieces[trait[nbply]][Orphan]==0)
    result = false;
  else
  {
    --number_of_pieces[advers(trait[nbply])][Orphan];
    occupy_square(sq_target,Dummy,spec[sq_target]);

    {
      square pos_orphans[63];
      locate_observees(Orphan,pos_orphans);
      result = find_next_orphan_in_chain(sq_target,pos_orphans,orphan_observer);
    }

    occupy_square(sq_target,Orphan,spec[sq_target]);
    ++number_of_pieces[advers(trait[nbply])][Orphan];
  }

  trait[nbply] = advers(trait[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
                                                  *orphan_observer);
      finply();

      if (found_chain)
        generate_moves_for_piece_based_on_walk(*orphan_observer);
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

boolean orphan_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;
  PieNam const *orphan_observer;
  square pos_orphans[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  locate_observees(Orphan,pos_orphans);

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]-1].capture = sq_target;

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; orphan_observer++)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0)
    {
      TracePiece(*orphan_observer);TraceText("\n");
      if (find_next_orphan_in_chain(sq_target,pos_orphans,*orphan_observer))
      {
        result = true;
        break;
      }
    }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
