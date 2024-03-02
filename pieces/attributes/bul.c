#include "pieces/attributes/bul.h"
#include "pieces/walks/hoppers.h"
#include "position/effects/piece_movement.h"
#include "solving/pipe.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "solving/has_solution_type.h"
#include "solving/post_move_iteration.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static ply bul_ply[maxply+1];

static boolean is_false(numecoup n)
{
  return false;
}

static void do_hurdle_movement(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(bul_ply[nbply]);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceEOL();

  move_effect_journal_do_piece_movement(move_effect_reason_bul,sq_departure,sq_arrival);
}

static boolean are_hurdle_movements_exhausted(void)
{
  return CURRMOVE_OF_PLY(bul_ply[nbply])==CURRMOVE_OF_PLY(bul_ply[nbply]-1);
}

static boolean advance_hurdle_movement(void)
{
  ply const save_nbply = nbply;

  nbply = bul_ply[nbply];
  pop_move();
  nbply = save_nbply;

  return !are_hurdle_movements_exhausted();
}

static boolean generate_hurdle_movements(slice_index si,
                                         move_effect_journal_index_type const movement)
{
  boolean result;
  ply const save_nbply = nbply;
  piece_walk_type walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",movement);
  TraceFunctionParamListEnd();

  TraceSquare(sq_hurdle);
  TraceWalk(walk_moving);
  TraceEOL();

  siblingply(advers(SLICE_STARTER(si)));
  bul_ply[save_nbply] = nbply;

  curr_generation->departure = sq_hurdle;
  move_generation_current_walk = walk_moving;
  generate_moves_delegate(SLICE_NEXT2(temporary_hack_move_generator[trait[nbply]]));
  move_generator_filter_captures(CURRMOVE_OF_PLY(nbply-1),&is_false);

  nbply = save_nbply;

  result = !are_hurdle_movements_exhausted();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void cleanup_hurdle_movements(void)
{
  nbply = bul_ply[nbply];
  finply();
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
void bul_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(movingspec,Bul))
  {
    if (!post_move_am_i_iterating())
    {
      if (generate_hurdle_movements(si,movement))
      {
        do_hurdle_movement(si);
        post_move_iteration_solve_delegate(si);
      }
      else
      {
        solve_result = this_move_is_illegal;
        cleanup_hurdle_movements();
      }
    }
    else if (post_move_have_i_lock() && !advance_hurdle_movement())
    {
      solve_result = this_move_is_illegal;
      cleanup_hurdle_movements();
      post_move_iteration_end();
    }
    else
    {
      do_hurdle_movement(si);
      post_move_iteration_solve_delegate(si);
    }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_bul(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STBul);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
