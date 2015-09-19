#include "optimisations/intelligent/block_flights.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/orthodox_square_observation.h"
#include "solving/pipe.h"
#include "pieces/pieces.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static unsigned int nr_king_flights_to_be_blocked;
static square king_flights_to_be_blocked[8];

static void block_planned_flights(slice_index si);

/* go on once all king flights have been blocked */
static void finalise_blocking(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /*assert(!echecc(White));
  if (goal_to_be_reached==goal_stale)
  {
    assert(!echecc(Black));
  }
  else
  {
    assert(echecc(Black));
  }
  */

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* block the next king flight */
static void block_next_flight(slice_index si)
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
        intelligent_place_black_piece(si,i,to_be_blocked,&block_planned_flights);
        black[i].usage = piece_is_unused;
      }

    occupy_square(to_be_blocked,Dummy,BIT(Black));
  }

  ++nr_king_flights_to_be_blocked;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* block the king flights */
static void block_planned_flights(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nr_king_flights_to_be_blocked==0)
    finalise_blocking(si);
  else
    block_next_flight(si);

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
void intelligent_find_and_block_flights(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_king_flights_to_be_blocked==0);

  plan_blocks_of_flights();
  if (nr_king_flights_to_be_blocked==0)
    finalise_blocking(si);
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

      block_planned_flights(si);

      for (i = 0; i!=nr_king_flights_to_be_blocked; ++i)
        empty_square(king_flights_to_be_blocked[i]);

      intelligent_unreserve();
    }

    nr_king_flights_to_be_blocked = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
