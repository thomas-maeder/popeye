#include "pieces/walks/friend.h"
#include "pieces/walks/orphan.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

void locate_observees(PieNam walk, square pos_observees[])
{
  unsigned int current = 0;
  square const *bnp;

  for (bnp = boardnum; current<number_of_pieces[trait[nbply]][walk]; ++bnp)
    if (get_walk_of_piece_on_square(*bnp)==walk && TSTFLAG(spec[*bnp],trait[nbply]))
    {
      pos_observees[current] = *bnp;
      ++current;
    }
}

void isolate_observee(PieNam walk, square const pos_observees[], unsigned int isolated_observee)
{
  unsigned int orphan_id;

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][walk]; ++orphan_id)
    if (orphan_id!=isolated_observee)
      occupy_square(pos_observees[orphan_id],Dummy,spec[pos_observees[orphan_id]]);
}

void restore_observees(PieNam walk, square const pos_observees[])
{
  unsigned int orphan_id;

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][walk]; ++orphan_id)
    occupy_square(pos_observees[orphan_id],walk,spec[pos_observees[orphan_id]]);
}

static boolean find_next_friend_in_chain(square sq_target,
                                         PieNam friend_observer)
{
  boolean result = false;

  move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;
  observing_walk[nbply] = friend_observer;
  if ((*checkfunctions[friend_observer])(&validate_observer))
    result = true;
  else
  {
    --number_of_pieces[trait[nbply]][Friend];

    if (number_of_pieces[trait[nbply]][Friend]>0)
    {
      unsigned int k;
      square pos_remaining_friends[63];

      occupy_square(sq_target,Dummy,spec[sq_target]);
      locate_observees(Friend,pos_remaining_friends);

      for (k = 0; k<number_of_pieces[trait[nbply]][Friend]; k++)
      {
        boolean is_friend_observed;

        isolate_observee(Friend,pos_remaining_friends,k);
        move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;
        observing_walk[nbply] = Friend;
        is_friend_observed = (*checkfunctions[friend_observer])(&validate_observer);
        restore_observees(Friend,pos_remaining_friends);

        if (is_friend_observed
            && find_next_friend_in_chain(pos_remaining_friends[k],friend_observer))
        {
          result = true;
          break;
        }
      }

      occupy_square(sq_target,Friend,spec[sq_target]);
    }

    ++number_of_pieces[trait[nbply]][Friend];
  }

  return result;
}

/* Generate moves for a Friend
 */
void friend_generate_moves(void)
{
  numecoup const save_nbcou = CURRMOVE_OF_PLY(nbply);
  Side const camp = trait[nbply];

  PieNam const *friend_observer;
  for (friend_observer = orphanpieces; *friend_observer!=Empty; ++friend_observer)
    if (number_of_pieces[camp][*friend_observer]>0)
    {
      boolean found_chain;

      siblingply(trait[nbply]);
      ++current_move[nbply];
      found_chain = find_next_friend_in_chain(curr_generation->departure,
                                              *friend_observer);
      finply();

      if (found_chain)
        generate_moves_for_piece_based_on_walk(*friend_observer);
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

boolean friend_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  PieNam const *pfr;
  boolean result = false;
  square pos_friends[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  locate_observees(Friend,pos_friends);

  siblingply(trait[nbply]);
  ++current_move[nbply];

  for (pfr = orphanpieces; *pfr!=Empty; pfr++)
    if (number_of_pieces[trait[nbply]][*pfr]>0)
    {
      unsigned int k;
      for (k = 0; k<number_of_pieces[trait[nbply]][Friend]; ++k)
      {
        boolean does_friend_observe;

        isolate_observee(Friend,pos_friends,k);
        move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;
        observing_walk[nbply] = Friend;
        does_friend_observe = (*checkfunctions[*pfr])(evaluate);
        restore_observees(Friend,pos_friends);

        if (does_friend_observe
            && find_next_friend_in_chain(pos_friends[k],*pfr))
        {
          result = true;
          break;
        }
      }

      if (result)
        break;
    }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
