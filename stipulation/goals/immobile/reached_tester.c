#include "stipulation/goals/immobile/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pybrafrk.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "trace.h"

#include <stdlib.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Allocate a system of slices that tests whether the side to be immobilised has
 * been
 * @return index of entry slice
 */
slice_index alloc_goal_immobile_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
  pipe_link(result,alloc_true_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalImmobileReachedTester slice.
 * @param starter_or_adversary is the starter immobilised or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const tester = alloc_pipe(STImmobilityTester);
    result = alloc_branch_fork(STGoalImmobileReachedTester,proxy);
    pipe_link(proxy,tester);
    pipe_link(tester,alloc_true_slice());
    slices[result].u.immobility_tester.applies_to_who = starter_or_adversary;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_optimiser(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const type = st->param;
  slices[si].type = *type;
  stip_traverse_structure_children(si,st);
}

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 * @param type substitute type
 */
void goal_immobile_reached_tester_replace(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_optimiser);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_king_first(slice_index si, stip_structure_traversal *st)
{
  slices[si].type = STImmobilityTesterKingFirst;
  stip_traverse_structure_children(si,st);
}

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_king_first);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_immobility_tester(slice_index si,
                                      stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_pipe(si,st);

  {
    Side const immobilised = (slices[si].u.immobility_tester.applies_to_who
                              ==goal_applies_to_starter
                              ? slices[si].starter
                              : advers(slices[si].starter));
    *starter = immobilised;
    stip_traverse_structure(slices[si].u.immobility_tester.fork,st);
  }

  *starter = slices[si].starter;

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

/* Generate moves for the king (if any) of a side
 * @param side side for which to generate king moves
 */
void generate_king_moves(Side side)
{
  if (side==White)
  {
    if (rb!=initsquare)
      gen_wh_piece(rb,abs(e[rb]));
  }
  else
  {
    if (rn!=initsquare)
      gen_bl_piece(rn,-abs(e[rn]));
  }
}

/* Find a legal move for a side. Start with the king moves that have already
 * been generated
 * @param side side for which to find a legal move
 * @return true iff a legal move has been found
 */
boolean find_any_legal_move_king_first(Side side)
{
  boolean result = false;

  square const *next_square_to_try = boardnum;
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
           && advance_departure_square(side,&next_square_to_try));

  return result;
}

static boolean test_immobility_king_first(Side side)
{
  boolean result = true;
  nextply(nbply);
  current_killer_state = null_killer_state;
  trait[nbply]= side;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(side));
  generate_king_moves(side);
  if (find_any_legal_move_king_first(side))
    result = false;
  finply();
  return result;
}

static boolean find_any_move(Side side)
{
  boolean result = false;

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play)
        && TraceCurrentMove(nbply)
        && !echecc(nbply,side))
      result = true;

    repcoup();
  }

  finply();

  return result;
}

static boolean ohneschach_find_any_move(Side side)
{
  boolean result = false;

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      /* no test for self-check necessary
       * was already done by ohneschach_pos_legal() */
      result = true;

    repcoup();
  }

  finply();

  return result;
}

static boolean ohneschach_find_nonchecking_move(Side side)
{
  boolean result = false;
  Side const ad = advers(side);

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  /* temporarily deactivate ohneschach to avoid the expensive test for checkmate
   * after moves that deliver check */
  CondFlag[ohneschach] = false;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,ad) && !echecc(nbply,side))
      result = true;
    repcoup();
  }

  CondFlag[ohneschach] = true;

  finply();

  return result;
}

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean ohneschach_immobile(Side side)
{
  boolean result = true;

  /* ohneschach_immobile() may invoke itself recursively. Protect ourselves from
   * infinite recursion. */
  if (nbply>maxply-2)
    FtlMsg(ChecklessUndecidable);

  /* first try to find a move that doesn't deliver check ... */
  if (ohneschach_find_nonchecking_move(side))
    result = false;

  /* ... then try to find a move that delivers mate. This is efficient
   * because determining whether ad is immobile is costly.
   */
  else if (ohneschach_find_any_move(side))
    result = false;

  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (find_any_move(slices[si].starter))
    result = has_no_solution;
  else
    result = slice_has_solution(slices[si].u.immobility_tester.next);

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
has_solution_type immobility_tester_king_first_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (test_immobility_king_first(slices[si].starter))
    result = has_no_solution;
  else
    result = slice_has_solution(slices[si].u.immobility_tester.next);

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
has_solution_type ohneschach_immobility_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (ohneschach_immobile(slices[si].starter))
    result = slice_has_solution(slices[si].u.immobility_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Is side immobile? */
boolean immobile(slice_index si, Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (CondFlag[ohneschach])
    result = ohneschach_immobile(side);
  else if (CondFlag[exclusive] || CondFlag[MAFF] || CondFlag[OWU])
    result = slice_has_solution(slices[si].u.immobility_tester.fork)==has_solution;
  else
    result = test_immobility_king_first(side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",true);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_immobile_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  Side const immobilised = (slices[si].u.immobility_tester.applies_to_who
                            ==goal_applies_to_starter
                            ? slices[si].starter
                            : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (immobile(si,immobilised))
    result = slice_has_solution(slices[si].u.immobility_tester.next);
  else
    result = has_no_solution;

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
has_solution_type immobility_tester_non_king_has_solution(slice_index si)
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
           && advance_departure_square(side,&next_square_to_try));

  finply();

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
