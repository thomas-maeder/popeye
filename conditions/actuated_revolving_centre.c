#include "conditions/actuated_revolving_centre.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_actuated_revolving_centre(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STActuatedRevolvingCentre);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void occupy(square dest, PieNam pi_src, Flags spec_src)
{
  assert(pi_src!=Invalid);

  if (pi_src==Empty)
    empty_square(dest);
  else
    occupy_square(dest,pi_src,spec_src);
}

static void revolve(void)
{
  PieNam const piece_temp = get_walk_of_piece_on_square(square_d4);
  Flags const spec_temp = spec[square_d4];

  occupy(square_d4,get_walk_of_piece_on_square(square_e4),spec[square_e4]);
  occupy(square_e4,get_walk_of_piece_on_square(square_e5),spec[square_e5]);
  occupy(square_e5,get_walk_of_piece_on_square(square_d5),spec[square_d5]);
  occupy(square_d5,piece_temp,spec_temp);
}

static void unrevolve(void)
{
  PieNam const piece_temp = get_walk_of_piece_on_square(square_d5);
  Flags const spec_temp = spec[square_d5];

  occupy(square_d5,get_walk_of_piece_on_square(square_e5),spec[square_e5]);
  occupy(square_e5,get_walk_of_piece_on_square(square_e4),spec[square_e4]);
  occupy(square_e4,get_walk_of_piece_on_square(square_d4),spec[square_d4]);
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
static void move_effect_journal_do_centre_revolution(move_effect_reason_type reason)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_centre_revolution;
  top_elmt->reason = reason;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  revolve();

  {
    square revolved = actuated_revolving_centre_revolve_square(king_square[White]);
    if (revolved!=initsquare)
      move_effect_journal_do_king_square_movement(reason,White,revolved);
  }

  {
    square revolved = actuated_revolving_centre_revolve_square(king_square[Black]);
    if (revolved!=initsquare)
      move_effect_journal_do_king_square_movement(reason,Black,revolved);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_centre_revolution(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  unrevolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_centre_revolution(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

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


/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type actuated_revolving_centre_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
  {
    move_effect_journal_do_centre_revolution(move_effect_reason_actuate_revolving_centre);
    result = solve(next,n);
  }
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
