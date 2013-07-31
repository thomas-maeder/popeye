#include "pieces/walks/friend.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

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

boolean find_next_friend_in_chain(square sq_target,
                                  PieNam friend_observer,
                                  PieNam friend_type,
                                  evalfunction_t *evaluate)
{
  boolean result = false;

  move_generation_stack[current_move[nbply]].capture = sq_target;
  if ((*checkfunctions[friend_observer])(sq_target,friend_observer,evaluate))
    result = true;
  else
  {
    --number_of_pieces[trait[nbply]][Friend];

    if (number_of_pieces[trait[nbply]][friend_type]>0)
    {
      unsigned int k;
      square pos_remaining_friends[63];

      occupy_square(sq_target,Dummy,spec[sq_target]);
      locate_observees(Friend,pos_remaining_friends);

      for (k = 0; k<number_of_pieces[trait[nbply]][friend_type]; k++)
      {
        boolean is_friend_observed;

        isolate_observee(Friend,pos_remaining_friends,k);
        move_generation_stack[current_move[nbply]].capture = sq_target;
        is_friend_observed = (*checkfunctions[friend_observer])(sq_target,Friend,evaluate);
        restore_observees(Friend,pos_remaining_friends);

        if (is_friend_observed
            && find_next_friend_in_chain(pos_remaining_friends[k],friend_observer,friend_type,evaluate))
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

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 */
void friend_generate_moves(square sq_departure)
{
  numecoup const save_nbcou = current_move[nbply];
  Side const camp = trait[nbply];

  PieNam const *friend_observer;
  for (friend_observer = orphanpieces; *friend_observer!=Empty; ++friend_observer)
    if (number_of_pieces[camp][*friend_observer]>0)
    {
      boolean found_chain;

      siblingply(trait[nbply]);
      current_move[nbply] = current_move[nbply-1]+1;
      move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
      found_chain = find_next_friend_in_chain(sq_departure,*friend_observer,Friend,&validate_observation);
      finply();

      if (found_chain)
        generate_moves_for_piece_based_on_walk(sq_departure,*friend_observer);
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}
