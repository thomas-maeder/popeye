#include "conditions/maff/immobility_tester.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "trace.h"

#include <stdlib.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_maff_specific_testers(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const next1 = slices[si].u.pipe.next;
    slice_index const next2 = stip_deep_copy(next1);
    slice_index const king_tester = alloc_pipe(STMaffImmobilityTesterKing);
    slice_index const other_tester = alloc_pipe(STMaffImmobilityTesterOther);

    pipe_link(si,alloc_and_slice(proxy1,proxy2));
    pipe_link(proxy1,king_tester);
    pipe_link(king_tester,next1);
    pipe_link(proxy2,other_tester);
    pipe_link(other_tester,next2);

    pipe_remove(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Replace immobility tester slices to cope with condition MAFF
 * @param si where to start (entry slice into stipulation)
 */
void maff_replace_immobility_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_maff_specific_testers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean maff_advance_departure_square(Side side,
                                             square const **next_square_to_try)
{
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));

  while (true)
  {
    square const sq_departure = **next_square_to_try;
    if (sq_departure==0)
      break;
    else
    {
      piece p = e[sq_departure];
      ++*next_square_to_try;
      if (p!=vide)
      {
        if (TSTFLAG(spec[sq_departure],Neutral))
          p = -p;

        if (side==White)
        {
          if (p>obs && sq_departure!=rb)
            gen_wh_piece(sq_departure,p);
        }
        else
        {
          if (p<vide && sq_departure!=rn)
            gen_bl_piece(sq_departure,p);
        }

        return true;
      }
    }
  }

  return false;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_king_has_solution(slice_index si)
{
  has_solution_type result;
  unsigned int nr_king_flights = 0;
  Side const side = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = side;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));
  generate_king_moves(side);

  while (nr_king_flights<=1 && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,side))
      ++nr_king_flights;
    repcoup();
  }

  finply();

  result = nr_king_flights==1 ? has_solution : has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_other_has_solution(slice_index si)
{
  has_solution_type result = has_solution;
  Side const side = slices[si].starter;
  square const *next_square_to_try = boardnum;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = side;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));

  do
  {
    while (result==has_solution && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,side))
        result = has_no_solution;
      repcoup();
    }
  } while (result==has_solution
           && maff_advance_departure_square(side,&next_square_to_try));

  finply();

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
