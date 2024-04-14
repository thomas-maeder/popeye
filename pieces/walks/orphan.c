#include "pieces/walks/orphan.h"
#include "pieces/walks/friend.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"
#include "debugging/assert.h"
#include "pieces/pieces.h"

piece_walk_type orphanpieces[nr_piece_walks];

static boolean orphan_find_observation_chain(square sq_target,
                                             piece_walk_type orphan_observer);

static boolean find_next_orphan_in_chain(square sq_target,
                                         square const pos_orphans[],
                                         piece_walk_type orphan_observer)
{
  boolean result = false;
  unsigned int orphan_id;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceWalk(orphan_observer);
  TraceFunctionParamListEnd();

  for (orphan_id = 0; orphan_id<being_solved.number_of_pieces[trait[nbply]][Orphan]; ++orphan_id)
  {
    boolean does_orphan_observe;

    isolate_observee(Orphan,pos_orphans,orphan_id);
    move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_target;
    observing_walk[nbply] = Orphan;
    does_orphan_observe = (*checkfunctions[orphan_observer])(EVALUATE(observer));
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
                                             piece_walk_type orphan_observer)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceWalk(orphan_observer);
  TraceFunctionParamListEnd();

  trait[nbply] = advers(trait[nbply]);

  replace_observation_target(sq_target);
  observing_walk[nbply] = orphan_observer;
  if ((*checkfunctions[orphan_observer])(EVALUATE(observer)))
    result = true;
  else if (being_solved.number_of_pieces[trait[nbply]][Orphan]==0)
    result = false;
  else
  {
    piece_walk_type const save_walk = get_walk_of_piece_on_square(sq_target);

    TraceWalk(save_walk);
    TraceEnumerator(Side,trait[nbply]);
    TraceEOL();

    if (TSTFLAG(being_solved.spec[sq_target],White))
    {
      assert(being_solved.number_of_pieces[White][save_walk]>0);
      --being_solved.number_of_pieces[White][save_walk];
    }
    if (TSTFLAG(being_solved.spec[sq_target],Black))
    {
      assert(being_solved.number_of_pieces[Black][save_walk]>0);
      --being_solved.number_of_pieces[Black][save_walk];
    }
    occupy_square(sq_target,Dummy,being_solved.spec[sq_target]);

    {
      square pos_orphans[63];
      locate_observees(Orphan,pos_orphans);
      result = find_next_orphan_in_chain(sq_target,pos_orphans,orphan_observer);
    }

    occupy_square(sq_target,save_walk,being_solved.spec[sq_target]);
    if (TSTFLAG(being_solved.spec[sq_target],White))
      ++being_solved.number_of_pieces[White][save_walk];
    if (TSTFLAG(being_solved.spec[sq_target],Black))
      ++being_solved.number_of_pieces[Black][save_walk];
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
  piece_walk_type const *orphan_observer;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; ++orphan_observer)
    if (being_solved.number_of_pieces[White][*orphan_observer]+being_solved.number_of_pieces[Black][*orphan_observer]>0)
    {
      boolean found_chain;

      siblingply(trait[nbply]);
      push_observation_target(initsquare);
      found_chain = orphan_find_observation_chain(curr_generation->departure,
                                                  *orphan_observer);
      finply();

      if (found_chain)
      {
        move_generation_current_walk = *orphan_observer;
        generate_moves_for_piece_based_on_walk();
      }
    }

  move_generation_current_walk = Orphan;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean orphan_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;
  piece_walk_type const *orphan_observer;
  square pos_orphans[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  locate_observees(Orphan,pos_orphans);

  siblingply(trait[nbply]);
  push_observation_target(sq_target);

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; orphan_observer++)
    if (being_solved.number_of_pieces[White][*orphan_observer]+being_solved.number_of_pieces[Black][*orphan_observer]>0)
    {
      TraceWalk(*orphan_observer);
      TraceEOL();
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
