#include "conditions/owu/immobility_tester.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/boolean/and.h"
#include "trace.h"

#include <stdlib.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_owu_specific_testers(slice_index si,
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
    slice_index const king_tester = alloc_pipe(STOWUImmobilityTesterKing);
    slice_index const other_tester = alloc_pipe(STOWUImmobilityTesterOther);

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

/* Substitute OWU specific immobility testers
 * @param si where to start (entry slice into stipulation)
 */
void owu_replace_immobility_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_owu_specific_testers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate (piece by piece) candidate moves to check if side is
 * immobile. Do *not* generate moves by the side's king; it has
 * already been taken care of. */
static boolean advance_departure_square(Side side,
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
has_solution_type owu_immobility_tester_king_has_solution(slice_index si)
{
  has_solution_type result;
  unsigned int nr_king_flights = 0;
  unsigned int nr_king_captures = 0;
  Side const side = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(nbply);
  current_killer_state = null_killer_state;
  trait[nbply] = side;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));
  generate_king_moves(side);

  while (nr_king_flights==0 && nr_king_captures<=1 && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      if (side==Black ? pprise[nbply]>=roib : pprise[nbply]<=roib)
        ++nr_king_captures; /* assuming OWU is OBU for checks to wK !! */
      if (!echecc(nbply,side))
        ++nr_king_flights;
    }
    repcoup();
  }

  finply();

  result = nr_king_flights==0 && nr_king_captures==1 ? has_solution : has_no_solution;

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
has_solution_type owu_immobility_tester_other_has_solution(slice_index si)
{
  has_solution_type result = has_solution;
  square const *next_square_to_try = boardnum;
  Side const side = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(nbply);
  current_killer_state = null_killer_state;
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
           && advance_departure_square(side,&next_square_to_try));

  finply();

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
