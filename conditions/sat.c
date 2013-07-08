#include "conditions/sat.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/temporary_hacks.h"
#include "solving/legal_move_counter.h"
#include "solving/check.h"
#include "debugging/trace.h"

#include <assert.h>

static boolean SATCheck;
boolean StrictSAT[nr_sides][maxply+1];
static slice_index strict_sat_flight_tester;
int SATFlights[nr_sides];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type sat_flight_moves_generator_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  PieNam const king_walk = get_walk_of_piece_on_square(king_square[starter]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();

  trait[nbply]= starter;

  dont_generate_castling= true;
  generate_moves_for_piece(starter,king_square[starter],king_walk);
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

static boolean find_flights(Side side_in_check,
                            unsigned int max_nr_flights_to_find)
{
  boolean result;

  /* avoid concurrent counts */
  assert(legal_move_counter_count[nbply]==0);

  /* stop counting once we have >=nr_flights legal king moves */
  legal_move_counter_interesting[nbply] = max_nr_flights_to_find;

  result = (solve(slices[temporary_hack_sat_flights_counter[side_in_check]].next2,
                   length_unspecified)
            == next_move_has_no_solution);

  assert(result
         ==(legal_move_counter_count[nbply]
            >legal_move_counter_interesting[nbply]));

  /* clean up after ourselves */
  legal_move_counter_count[nbply] = 0;

  return result;
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean sat_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  if (SATCheck)
  {
    boolean result;
    int nr_flights = SATFlights[side_in_check];

    SATCheck = false;

    if (nr_flights==0)
      result = true;
    else
      result = find_flights(side_in_check,nr_flights-1);

    SATCheck = true;

    return result;
  }
  else
    return is_in_check(slices[si].next1,side_in_check);
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean strictsat_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  if (SATCheck)
  {
    boolean result;
    int nr_flights = SATFlights[side_in_check];

    SATCheck = false;

    if (StrictSAT[side_in_check][parent_ply[nbply]])
    {
      if (!is_in_check(slices[si].next1,side_in_check))
        --nr_flights;
    }

    if (nr_flights==0)
      result = true;
    else
      result = find_flights(side_in_check,nr_flights-1);

    SATCheck = true;

    return result;
  }
  else
    return is_in_check(slices[si].next1,side_in_check);
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean satxy_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  if (SATCheck)
  {
    boolean result;
    int nr_flights = SATFlights[side_in_check];

    SATCheck = false;

    if (!is_in_check(slices[si].next1,side_in_check))
      --nr_flights;

    if (nr_flights==0)
      result = true;
    else
      result = find_flights(side_in_check,nr_flights-1);

    SATCheck = true;

    return result;
  }
  else
    return is_in_check(slices[si].next1,side_in_check);
}

static void update_strict_sat(void)
{
  SATCheck = false;
  StrictSAT[White][nbply]= (StrictSAT[White][parent_ply[nbply]]
                            || is_in_check(slices[strict_sat_flight_tester].next1,White));
  StrictSAT[Black][nbply]= (StrictSAT[Black][parent_ply[nbply]]
                            || is_in_check(slices[strict_sat_flight_tester].next1,Black));
  SATCheck = true;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type strict_sat_initialiser_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  SATCheck = false;
  strict_sat_flight_tester = branch_find_slice(STStrictSATCheckTester,slices[temporary_hack_check_tester].next2,stip_traversal_context_intro);
  assert(strict_sat_flight_tester!=no_slice);
  StrictSAT[White][1] = is_in_check(slices[strict_sat_flight_tester].next1,White);
  StrictSAT[Black][1] = is_in_check(slices[strict_sat_flight_tester].next1,Black);
  SATCheck = true;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
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

/* Instrument the stipulation with SAT specific king flight move generators
 * @param root_slice root slice of stipulation
 */
static void instrument_solvers(slice_index root_slice)
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

static void instrument_move_with_updater(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STStrictSATUpdater);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void strictsat_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STSATFlightsCounterFork,
                                             &stip_traverse_structure_children_pipe);
    stip_structure_traversal_override_single(&st,STMove,&instrument_move_with_updater);
    stip_traverse_structure(si,&st);
  }

  instrument_solvers(si);
  solving_instrument_check_testing(si,STStrictSATCheckTester);

  {
    slice_index const prototype = alloc_pipe(STStrictSATInitialiser);
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void sat_initialise_solving(slice_index si)
{
  instrument_solvers(si);

  if (SATFlights[White]>1 || SATFlights[Black]>1)
    solving_instrument_check_testing(si,STSATxyCheckTester);
  else
    solving_instrument_check_testing(si,STSATCheckTester);

  SATCheck = true;
}
