#include "conditions/series_capture.h"
#include "position/position.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "pieces/walks/pawns/promotion.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static unsigned int level;

typedef struct
{
  ply ply_secondary_movement;
} level_state_type;

static level_state_type levels[maxply+1];

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STSeriesCapture);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor ultraschachzwang_enforcer_inserters[] =
{
  { STGoalMateReachedTester,     &stip_structure_visitor_noop           },
  { STMove,                      &instrument_move                       },
  { STKingCaptureLegalityTester, &stip_traverse_structure_children_pipe }
};

enum
{
  nr_ultraschachzwang_enforcer_inserters =
      (sizeof ultraschachzwang_enforcer_inserters
       / sizeof ultraschachzwang_enforcer_inserters[0])
};

/* Instrument the solving machinery with Series Capture
 * @param si identifies entry slice into solving machinery
 */
void solving_instrument_series_capture(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    ultraschachzwang_enforcer_inserters,
                                    nr_ultraschachzwang_enforcer_inserters);
  stip_traverse_structure(si,&st);

  promotion_insert_slice_sequence(si,STSeriesCapture,&move_insert_slices);

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
void series_capture_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    ++level;

    if (post_move_am_i_iterating())
    {
      numecoup const curr = CURRMOVE_OF_PLY(levels[level].ply_secondary_movement);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_capture = move_gen_top->capture;
      square const sq_departure = move_gen_top->departure;
      square const sq_arrival = move_gen_top->arrival;

      if (!is_no_capture(sq_capture))
        move_effect_journal_do_piece_removal(move_effect_reason_series_capture,
                                             sq_capture);
      move_effect_journal_do_piece_movement(move_effect_reason_series_capture,
                                            sq_departure,
                                            sq_arrival);
      move_effect_journal_base[levels[level].ply_secondary_movement+1] = move_effect_journal_base[nbply+1];
      post_move_iteration_solve_delegate(si);

      if (!post_move_iteration_is_locked())
      {
        nbply = levels[level].ply_secondary_movement;
        pop_move();
        if (encore())
          nbply = parent_ply[nbply];
        else
        {
          post_move_iteration_end();
          finply();
          levels[level].ply_secondary_movement = 0;
        }
      }
    }
    else
    {
      post_move_iteration_solve_delegate(si);
      if (solve_result==this_move_is_illegal)
        post_move_iteration_cancel();
      else
      {
        move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
        auxiliaryply(SLICE_STARTER(si));
        assert(levels[level].ply_secondary_movement==0);
        levels[level].ply_secondary_movement = nbply;
        generate_moves_for_piece(move_effect_journal[movement].u.piece_movement.to);
        nbply = parent_ply[nbply];
      }
    }

    --level;
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
