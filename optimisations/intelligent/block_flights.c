#include "optimisations/intelligent/block_flights.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "options/maxsolutions/maxsolutions.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void block_planned_flights(stip_length_type n,
                                  unsigned int nr_flights_to_block);

static void finalise_blocking(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const is_white_in_check = echecc(nbply,White);
    if (goal_to_be_reached==goal_stale)
    {
      unsigned int const is_black_in_check = echecc(nbply,Black);
      if (is_black_in_check)
        intelligent_stalemate_intercept_checks(n,is_white_in_check,Black);
      else if (is_white_in_check)
        intelligent_stalemate_intercept_checks(n,is_black_in_check,White);
      else
        intelligent_stalemate_test_target_position(n);
    }
    else
    {
      assert(echecc(nbply,Black));
      intelligent_mate_finish(n,is_white_in_check);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    square to_be_blocked,
                    piece blocker_type,
                    Flags blocker_flags,
                    square blocks_from,
                    unsigned int nr_remaining_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             blocks_from,
                                                                             blocker_type,
                                                                             to_be_blocked);
    if (time<=Nr_remaining_black_moves)
    {
      Nr_remaining_black_moves -= time;
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(blocker_type,to_be_blocked,blocker_flags);
      block_planned_flights(n,nr_remaining_flights_to_block);
      Nr_remaining_black_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n,
                            square to_be_blocked,
                            Flags blocker_flags,
                            square blocks_from,
                            unsigned int nr_remaining_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(to_be_blocked))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                      blocks_from,
                                                                                      to_be_blocked);
    if (time<=Nr_remaining_black_moves)
    {
      unsigned int const diffcol = abs(blocks_from%onerow - to_be_blocked%onerow);
      SetPiece(pn,to_be_blocked,blocker_flags);
      if (diffcol<=Nr_unused_white_masses)
      {
        Nr_unused_white_masses -= diffcol;
        Nr_remaining_black_moves -= time;
        TraceValue("%u",Nr_unused_white_masses);
        TraceValue("%u\n",Nr_remaining_black_moves);
        block_planned_flights(n,nr_remaining_flights_to_block);
        Nr_remaining_black_moves += time;
        Nr_unused_white_masses += diffcol;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          square to_be_blocked,
                          square blocks_from,
                          Flags blocker_flags,
                          unsigned int nr_remaining_flights_to_block)
{
  unsigned int nr_moves_guesstimate = blocks_from/onerow - nr_of_slack_rows_below_board;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  /* A rough check whether it is worth thinking about promotions */
  if (nr_moves_guesstimate==6)
    --nr_moves_guesstimate; /* double step! */

  if (to_be_blocked>=square_a2)
    /* square is not on 8th rank -- 1 move necessary to get there */
    ++nr_moves_guesstimate;

  TraceValue("%u\n",nr_moves_guesstimate);
  if (Nr_remaining_black_moves>=nr_moves_guesstimate)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(blocks_from,
                                                                                       pp,
                                                                                       to_be_blocked);
        unsigned int diffcol = 0;
        if (pp==fn)
        {
          unsigned int const placed_from_file = blocks_from%nr_files_on_board;
          square const promotion_square_on_same_file = square_a1+placed_from_file;
          if (SquareCol(to_be_blocked)!=SquareCol(promotion_square_on_same_file))
            diffcol = 1;
        }
        if (diffcol<=Nr_unused_white_masses
            && time<=Nr_remaining_black_moves)
        {
          Nr_unused_white_masses -= diffcol;
          Nr_remaining_black_moves -= time;
          TraceValue("%u",Nr_unused_white_masses);
          TraceValue("%u\n",Nr_remaining_black_moves);
          SetPiece(pp,to_be_blocked,blocker_flags);
          block_planned_flights(n,nr_remaining_flights_to_block);
          Nr_remaining_black_moves += time;
          Nr_unused_white_masses += diffcol;
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int nr_king_flights_to_be_blocked;

static square king_flights_to_be_blocked[8];

static void block_next_flight(stip_length_type n,
                              unsigned int nr_flights_to_block)
{
  unsigned int index_of_current_blocker;
  unsigned int const current_flight = nr_flights_to_block-1;
  square const to_be_blocked = king_flights_to_be_blocked[current_flight];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_flights_to_block);
  TraceFunctionParamListEnd();

  TraceSquare(to_be_blocked);TraceText("\n");
  if (nr_reasons_for_staying_empty[to_be_blocked]==0)
  {
    for (index_of_current_blocker = 1;
         index_of_current_blocker<MaxPiece[Black];
         index_of_current_blocker++)
      if (black[index_of_current_blocker].usage==piece_is_unused)
      {
        piece const blocker_type = black[index_of_current_blocker].type;
        square const blocks_from = black[index_of_current_blocker].diagram_square;
        Flags const blocker_flags = black[index_of_current_blocker].flags;

        black[index_of_current_blocker].usage = piece_blocks;

        if (blocker_type==pn)
        {
          if (to_be_blocked>=square_a2)
            unpromoted_pawn(n,
                            to_be_blocked,blocker_flags,blocks_from,
                            nr_flights_to_block-1);

          promoted_pawn(n,
                        to_be_blocked,blocks_from,blocker_flags,
                        nr_flights_to_block-1);
        }
        else
          officer(n,
                  to_be_blocked,
                  blocker_type,blocker_flags,blocks_from,
                  nr_flights_to_block-1);

        black[index_of_current_blocker].usage = piece_is_unused;
      }

    e[to_be_blocked] = vide;
    spec[to_be_blocked] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_planned_flights(stip_length_type n,
                                  unsigned int nr_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_flights_to_block);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
  {
    /* nothing */
  }
  else if (nr_flights_to_block==0)
    finalise_blocking(n);
  else
    block_next_flight(n,nr_flights_to_block);

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

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type stalemate_flight_optimiser_can_help(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (goal_to_be_reached==goal_stale)
  {
    /* we also need to block the flights that are currently guarded by a
     * line piece through the king's square - that line is going to be
     * intercepted in the play, so intercept it here as well */
    e[move_generation_stack[nbcou].departure] = obs;
    result = selfcheck_guard_can_help2(next,n);
//    result = can_help(next,n);
    e[move_generation_stack[nbcou].departure] = vide;
  }
  else
    result = selfcheck_guard_can_help2(next,n);
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

  result = stalemate_flight_optimiser_can_help(next,n);
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
  genmove(side_at_move);
//  result = can_help(next,n);
  result = move_can_help2(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int plan_blocks_of_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_king_flights_to_be_blocked = 0;

  nr_available_blockers = Nr_unused_black_masses;
  if (move_generator_can_help2(no_slice,slack_length_help+1)
      ==slack_length_help+1)
    /* at least 1 flight was found that can't be blocked */
    nr_king_flights_to_be_blocked = MaxPiece[Black];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",nr_king_flights_to_be_blocked);
  TraceFunctionResultEnd();
  return nr_king_flights_to_be_blocked;
}

static int count_nr_unused_white_masses(void)
{
  int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; ++i)
    if (white[i].usage==piece_is_unused)
      ++result;


  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_min_nr_black_moves_for_blocks(unsigned int nr_flights_to_block)
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_flights_to_block && result<=Nr_remaining_black_moves; ++i)
    result += intelligent_estimate_min_nr_black_moves_to_square(king_flights_to_be_blocked[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

void intelligent_find_and_block_flights(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const nr_flights_to_block = plan_blocks_of_flights();
    if (nr_flights_to_block<=Nr_unused_black_masses)
    {
      unsigned int const mtba = count_min_nr_black_moves_for_blocks(nr_flights_to_block);
      if (mtba<=Nr_remaining_black_moves)
      {
        Nr_unused_white_masses = count_nr_unused_white_masses();
        block_planned_flights(n,nr_flights_to_block);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
