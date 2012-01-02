#include "optimisations/intelligent/block_flights.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/mate/finish.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "options/maxsolutions/maxsolutions.h"
#include "solving/king_move_generator.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static unsigned int nr_king_flights_to_be_blocked;
static square king_flights_to_be_blocked[8];

static void block_planned_flights(void);

/* go on once all king flights have been blocked */
static void finalise_blocking(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));
  if (goal_to_be_reached==goal_stale)
  {
    assert(!echecc(nbply,Black));
    intelligent_stalemate_test_target_position();
  }
  else
  {
    assert(echecc(nbply,Black));
    intelligent_mate_test_target_position();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* block the next king flight */
static void block_next_flight(void)
{
  unsigned int i;
  unsigned int const current_flight = nr_king_flights_to_be_blocked-1;
  square const to_be_blocked = king_flights_to_be_blocked[current_flight];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nr_king_flights_to_be_blocked;

  TraceSquare(to_be_blocked);TraceText("\n");
  if (nr_reasons_for_staying_empty[to_be_blocked]==0)
  {
    for (i = 1; i<MaxPiece[Black]; i++)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_blocks;
        intelligent_place_black_piece(i,to_be_blocked,&block_planned_flights);
        black[i].usage = piece_is_unused;
      }

    e[to_be_blocked] = dummyn;
    spec[to_be_blocked] = EmptySpec;
  }

  ++nr_king_flights_to_be_blocked;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* block the king flights */
static void block_planned_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
  {
    /* nothing */
  }
  else if (nr_king_flights_to_be_blocked==0)
    finalise_blocking();
  else
    block_next_flight();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type capture_finder_can_help(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type selfcheck_guard_can_help2(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,Black))
    result = n+4;
  else
    result = capture_finder_can_help(next,n);
//    result = can_help(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int nr_available_blockers;

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type flights_to_be_blocked_finder_can_help(slice_index si,
                                                              stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = selfcheck_guard_can_help2(next,n);
//    result = can_help(next,n);
  switch (result)
  {
    case slack_length_help+4:
      /* this 'flight' is guarded */
      break;

    case slack_length_help+2:
      if (nr_king_flights_to_be_blocked<nr_available_blockers)
      {
        king_flights_to_be_blocked[nr_king_flights_to_be_blocked] = move_generation_stack[nbcou].arrival;
        ++nr_king_flights_to_be_blocked;
      }
      else
        /* more blocks are required than there are blockers available */
        result = slack_length_help;
      break;

    case slack_length_help:
      /* this flight is occupied by White and therefore can't be blocked */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int nr_available_blockers;

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type move_can_help2(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while(encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && flights_to_be_blocked_finder_can_help(next,n-1)==n-1)
    {
      result = n;
      repcoup();
      break;
    }
    else
      repcoup();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
static stip_length_type move_generator_can_help2(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
//  Side const side_at_move = slices[si].starter;
//  slice_index const next = slices[si].u.pipe.next;
  Side const side_at_move = Black;
  slice_index const next = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = side_at_move;
  generate_king_moves(side_at_move);
//  result = can_help(next,n);
  result = move_can_help2(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void plan_blocks_of_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_available_blockers = intelligent_get_nr_reservable_masses(Black);
  if (move_generator_can_help2(no_slice,slack_length_help+1)
      ==slack_length_help+1)
    /* at least 1 flight was found that can't be blocked */
    nr_king_flights_to_be_blocked = MaxPiece[Black];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find black king flights and block them */
void intelligent_find_and_block_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_king_flights_to_be_blocked==0);

  plan_blocks_of_flights();
  if (nr_king_flights_to_be_blocked==0)
    finalise_blocking();
  else
  {
    if (intelligent_reserve_black_masses_for_blocks(king_flights_to_be_blocked,
                                                    nr_king_flights_to_be_blocked))
    {
      unsigned int i;
      for (i = 0; i!=nr_king_flights_to_be_blocked; ++i)
      {
        TraceSquare(king_flights_to_be_blocked[i]);
        TraceText("\n");
        e[king_flights_to_be_blocked[i]] = dummyn;
      }

      block_planned_flights();

      for (i = 0; i!=nr_king_flights_to_be_blocked; ++i)
        e[king_flights_to_be_blocked[i]] = vide;

      intelligent_unreserve();
    }

    nr_king_flights_to_be_blocked = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
