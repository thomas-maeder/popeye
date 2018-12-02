#include "conditions/actuated_revolving_centre.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static void occupy(square dest, piece_walk_type pi_src, Flags spec_src)
{
  assert(pi_src!=Invalid);

  if (pi_src==Empty)
    empty_square(dest);
  else
    occupy_square(dest,pi_src,spec_src);
}

static void revolve(void)
{
  piece_walk_type const piece_temp = get_walk_of_piece_on_square(square_d4);
  Flags const spec_temp = being_solved.spec[square_d4];

  occupy(square_d4,get_walk_of_piece_on_square(square_e4),being_solved.spec[square_e4]);
  occupy(square_e4,get_walk_of_piece_on_square(square_e5),being_solved.spec[square_e5]);
  occupy(square_e5,get_walk_of_piece_on_square(square_d5),being_solved.spec[square_d5]);
  occupy(square_d5,piece_temp,spec_temp);
}

static void unrevolve(void)
{
  piece_walk_type const piece_temp = get_walk_of_piece_on_square(square_d5);
  Flags const spec_temp = being_solved.spec[square_d5];

  occupy(square_d5,get_walk_of_piece_on_square(square_e5),being_solved.spec[square_e5]);
  occupy(square_e5,get_walk_of_piece_on_square(square_e4),being_solved.spec[square_e4]);
  occupy(square_e4,get_walk_of_piece_on_square(square_d4),being_solved.spec[square_d4]);
  occupy(square_d4,piece_temp,spec_temp);
}

/* Apply revolution to one square
 * @param s the square
 * @return revolved square
 */
square actuated_revolving_centre_revolve_square(square s)
{
  square result;

  if (s==square_d4)
    result = square_d5;
  else if (s==square_d5)
    result = square_e5;
  else if (s==square_e5)
    result = square_e4;
  else if (s==square_e4)
    result = square_d4;
  else
    result = initsquare;

  return result;
}

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
static void do_centre_revolution(move_effect_reason_type reason)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_centre_revolution,reason);
  revolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_centre_revolution(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  unrevolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_centre_revolution(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  revolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean does_move_trigger_revolution(void)
{
  boolean result;
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = (sq_departure==square_d4    || sq_departure==square_e4
            || sq_departure==square_d5 || sq_departure==square_e5
            || sq_arrival==square_d4   || sq_arrival==square_e4
            || sq_arrival==square_d5   || sq_arrival==square_e5);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
void actuated_revolving_centre_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
    do_centre_revolution(move_effect_reason_actuate_revolving_centre);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_actuated_revolving_centre(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_centre_revolution,
                                       &undo_centre_revolution,
                                       &redo_centre_revolution);

  stip_instrument_moves(si,STActuatedRevolvingCentre);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
