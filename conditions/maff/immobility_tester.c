#include "conditions/maff/immobility_tester.h"
#include "pydata.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "trace.h"

#include <stdlib.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_optimiser(slice_index si, stip_structure_traversal *st)
{
  slices[si].type = STMaffImmobilityTester;
  stip_traverse_structure_children(si,st);
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
                                           &substitute_optimiser);
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

static boolean maff_find_any_legal_move_king_first(slice_index si)
{
  boolean result = false;
  Side const side = slices[si].starter;
  square const *next_square_to_try = boardnum;

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = side;

  do
  {
    while (!result && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,side))
        result = true;
      repcoup();
    }
  } while (!result
           && maff_advance_departure_square(side,&next_square_to_try));

  finply();

  return result;
}

static boolean maff_is_king_immobile(slice_index si)
{
  unsigned int nr_king_flights = 0;
  Side const side = slices[si].starter;

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = side;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));
  generate_king_moves(side);

  /* don't stop if nr_king_flights>1 - we want to use up all king moves here */
  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,side))
      ++nr_king_flights;
    repcoup();
  }

  finply();

  return nr_king_flights==1;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_has_solution(slice_index si)
{
  has_solution_type result = has_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!maff_is_king_immobile(si)
      || maff_find_any_legal_move_king_first(si))
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
