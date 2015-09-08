#include "optimisations/intelligent/block_flights.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/mate/finish.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "optimisations/orthodox_square_observation.h"
#include "options/maxsolutions/maxsolutions.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static unsigned int nr_king_flights_to_be_blocked;
static square king_flights_to_be_blocked[8];

static void block_planned_flights(void);

/* go on once all king flights have been blocked */
static void finalise_blocking(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /*assert(!echecc(White));*/
  if (goal_to_be_reached==goal_stale)
  {
    /*assert(!echecc(Black));*/
    intelligent_stalemate_test_target_position();
  }
  else
  {
    /*assert(echecc(Black));*/
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

  TraceSquare(to_be_blocked);TraceEOL();
  if (nr_reasons_for_staying_empty[to_be_blocked]==0)
  {
    for (i = 1; i<MaxPiece[Black]; i++)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_blocks;
        intelligent_place_black_piece(i,to_be_blocked,&block_planned_flights);
        black[i].usage = piece_is_unused;
      }

    occupy_square(to_be_blocked,Dummy,BIT(Black));
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

static void plan_blocks_of_flights(void)
{
  vec_index_type i;
  unsigned int nr_available_blockers;
  Flags const king_square_flags = being_solved.spec[being_solved.king_square[Black]];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_available_blockers = intelligent_get_nr_reservable_masses(Black);

  assert(get_walk_of_piece_on_square(being_solved.king_square[Black])==King);
  empty_square(being_solved.king_square[Black]);

  for (i = vec_queen_start; i<=vec_queen_end; ++i)
  {
    square const flight = being_solved.king_square[Black]+vec[i];

    if (is_square_blocked(flight) || TSTFLAG(being_solved.spec[flight],Black))
    {
      /* 'flight' is off board or blocked - don't bother */
    }
    else if (!is_square_observed_ortho(White,flight))
    {
      if (TSTFLAG(being_solved.spec[flight],White)
          || nr_king_flights_to_be_blocked==nr_available_blockers)
      {
        /* flight can't be blocked! */
        nr_king_flights_to_be_blocked = nr_available_blockers+1;
        break;
      }
      else
      {
        king_flights_to_be_blocked[nr_king_flights_to_be_blocked] = flight;
        ++nr_king_flights_to_be_blocked;
      }
    }
  }

  occupy_square(being_solved.king_square[Black],King,king_square_flags);

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
        TraceEOL();
        block_square(king_flights_to_be_blocked[i]);
      }

      block_planned_flights();

      for (i = 0; i!=nr_king_flights_to_be_blocked; ++i)
        empty_square(king_flights_to_be_blocked[i]);

      intelligent_unreserve();
    }

    nr_king_flights_to_be_blocked = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
