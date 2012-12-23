#include "conditions/sat.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_player.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/temporary_hacks.h"
#include "solving/legal_move_counter.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean SATCheck;
boolean StrictSAT[nr_sides][maxply+1];
int SATFlights[nr_sides];
boolean satXY;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sat_flight_moves_generator_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();

  trait[nbply]= starter;

  dont_generate_castling= true;
  if (starter==White)
    gen_wh_piece(king_square[starter],abs(e[king_square[starter]]));
  else
    gen_bl_piece(king_square[starter],-abs(e[king_square[starter]]));
  dont_generate_castling = false;

  result = solve(slices[si].next1,n);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const generator = branch_find_slice(STMoveGenerator,slices[si].next2,st->context);
    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_pipe(STSATFlightMoveGenerator));
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation with SAT specific king flight move generators
 * @param root_slice root slice of stipulation
 */
void stip_substitute_sat_king_flight_generators(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STSATFlightsCounterFork,
                                           &substitute_generator);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side is in SAT check
 * @param side side for which to test check
 * @return true iff side is in check
 */
boolean echecc_SAT(Side side)
{
  boolean result;
  int nr_flights = SATFlights[side];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  SATCheck = false;

  if (satXY || (CondFlag[strictSAT] && StrictSAT[side][parent_ply[nbply]]))
  {
    if (!echecc(side))
      --nr_flights;
  }

  if (nr_flights==0)
    result = true;
  else
  {
    /* avoid concurrent counts */
    assert(legal_move_counter_count[nbply]==0);

    /* stop counting once we have >=nr_flights legal king moves */
    legal_move_counter_interesting[nbply] = nr_flights-1;

    result = (solve(slices[temporary_hack_sat_flights_counter[side]].next2,
                     length_unspecified)
              == has_no_solution);

    assert(result
           ==(legal_move_counter_count[nbply]
              >legal_move_counter_interesting[nbply]));

    /* clean up after ourselves */
    legal_move_counter_count[nbply] = 0;
  }

  SATCheck = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void update_strict_sat(void)
{
  if (SATCheck)
  {
    SATCheck = false;
    StrictSAT[White][nbply]= (StrictSAT[White][parent_ply[nbply]]
                              || echecc(White));
    StrictSAT[Black][nbply]= (StrictSAT[Black][parent_ply[nbply]]
                              || echecc(Black));
    SATCheck = true;
  }
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type strict_sat_updater_solve(slice_index si,
                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  update_strict_sat();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_strict_sat(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STStrictSATUpdater);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
