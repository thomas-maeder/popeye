#include "conditions/sat.h"
#include "pieces/pieces.h"
#include "pieces/walks/vectors.h"
#include "pieces/walks/generate_moves.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "stipulation/structure_traversal.h"
#include "solving/temporary_hacks.h"
#include "solving/legal_move_counter.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

boolean StrictSAT[nr_sides];
unsigned int SAT_max_nr_allowed_flights[nr_sides];

/* set to a side while we are updating its strict SAT status */
static Side strictsat_updating = nr_sides;

static boolean find_flights(slice_index si,
                            Side side_in_check,
                            unsigned int nr_flights_to_find)
{
  unsigned int nr_flights_found = 0;
  square const save_king_square = being_solved.king_square[side_in_check];
  piece_walk_type const king_walk = get_walk_of_piece_on_square(save_king_square);
  Flags const king_flags = being_solved.spec[save_king_square];
  square const save_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(side_in_check);

  curr_generation->departure = save_king_square;
  move_generation_current_walk = king_walk;
  generate_moves_for_piece_based_on_walk();

  empty_square(save_king_square);

  while (encore())
  {
    being_solved.king_square[side_in_check] = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
    if ((is_square_empty(being_solved.king_square[side_in_check])
         || TSTFLAG(being_solved.spec[being_solved.king_square[side_in_check]],advers(side_in_check)))
        && being_solved.king_square[side_in_check]!=being_solved.king_square[advers(side_in_check)]
        && !pipe_is_in_check_recursive_delegate(si,side_in_check))
      ++nr_flights_found;

    pop_move();
  }

  being_solved.king_square[side_in_check] = save_king_square;
  occupy_square(save_king_square,king_walk,king_flags);

  curr_generation->departure = save_departure;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",nr_flights_found>nr_flights_to_find);
  TraceFunctionResultEnd();
  return nr_flights_found>nr_flights_to_find;
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean sat_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;

  if (SAT_max_nr_allowed_flights[side_in_check]==0)
    result = true;
  else
    result = find_flights(si,side_in_check,SAT_max_nr_allowed_flights[side_in_check]-1);

  return result;
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean strictsat_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;

  if (strictsat_updating==side_in_check)
    result = pipe_is_in_check_recursive_delegate(si,side_in_check);
  else
  {
    unsigned int max_nr_allowed_flights = SAT_max_nr_allowed_flights[side_in_check];

    if (StrictSAT[side_in_check])
    {
      if (!pipe_is_in_check_recursive_delegate(si,side_in_check))
        --max_nr_allowed_flights;
    }

    if (max_nr_allowed_flights==0)
      result = true;
    else
      result = find_flights(si,side_in_check,max_nr_allowed_flights-1);
  }

  return result;
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean satxy_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;
  unsigned int max_nr_allowed_flights = SAT_max_nr_allowed_flights[side_in_check];

  if (!pipe_is_in_check_recursive_delegate(si,side_in_check))
    --max_nr_allowed_flights;

  if (max_nr_allowed_flights==0)
    result = true;
  else
    result = find_flights(si,side_in_check,max_nr_allowed_flights-1);

  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void strict_sat_initialiser_solve(slice_index si)
{
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(strictsat_updating==nr_sides);

  update_king_squares();

  strictsat_updating = White;
  StrictSAT[White] = is_in_check(White);

  strictsat_updating = Black;
  StrictSAT[Black] = is_in_check(Black);

  strictsat_updating = nr_sides;

  pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Adjust the Strict SAT state
 * @param diff adjustment
 */
static void do_strict_sat_adjustment(Side side)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_strict_sat_adjustment,move_effect_reason_sat_adjustment);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  entry->u.strict_sat_adjustment.side = side;

  StrictSAT[side] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo a Strict SAT state adjustment
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_undo_strict_sat_adjustment(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.strict_sat_adjustment.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  StrictSAT[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo a Strict SAT state adjustment
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_redo_strict_sat_adjustment(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.strict_sat_adjustment.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  StrictSAT[side] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void strict_sat_updater_solve(slice_index si)
{
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(strictsat_updating==nr_sides);

  update_king_squares();

  if (!StrictSAT[White])
  {
    strictsat_updating = White;
    if (is_in_check(White))
      do_strict_sat_adjustment(White);
  }
  if (!StrictSAT[Black])
  {
    strictsat_updating = Black;
    if (is_in_check(Black))
      do_strict_sat_adjustment(Black);
  }

  strictsat_updating = nr_sides;

  pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

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
    move_insert_slices(si,st->context,&prototype,1);
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

  move_effect_journal_set_effect_doers(move_effect_strict_sat_adjustment,
                                       &move_effect_journal_undo_strict_sat_adjustment,
                                       &move_effect_journal_redo_strict_sat_adjustment);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STMove,&instrument_move_with_updater);
    stip_traverse_structure(si,&st);
  }

  solving_instrument_check_testing(si,STStrictSATCheckTester);

  {
    slice_index const prototype = alloc_pipe(STStrictSATInitialiser);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void sat_initialise_solving(slice_index si)
{
  if (SAT_max_nr_allowed_flights[White]>1 || SAT_max_nr_allowed_flights[Black]>1)
    solving_instrument_check_testing(si,STSATxyCheckTester);
  else
    solving_instrument_check_testing(si,STSATCheckTester);
}
