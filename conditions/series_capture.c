#include "conditions/series_capture.h"
#include "position/position.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/proxy.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/binary.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static unsigned int level;

typedef enum
{
  without_added_movement,
  with_added_movement
} level_status_type;

typedef struct
{
  level_status_type status;
  square recurse_from;
  ply ply_secondary_movement;
} level_state_type;

static level_state_type levels[maxply+1];

static void insert_series_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const landing = alloc_pipe(STLandingAfterSeriesCapture);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const recursor = alloc_fork_slice(STSeriesCaptureRecursor,proxy1);
    slice_index const after = alloc_pipe(STLandingAfterPawnPromotion);
    slice_index const before = alloc_pipe(STBeforePawnPromotion);
    slice_index const series = alloc_pipe(STSeriesCapture);
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_slice(STSeriesCaptureFork,proxy2);

    pipe_append(si,landing);
    pipe_append(si,fork);
    pipe_append(proxy2,series);
    pipe_link(after,recursor);
    pipe_link(before,after);
    pipe_link(series,before);
    pipe_set_successor(proxy1,series);
    pipe_set_successor(recursor,landing);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STBeforeSeriesCapture),
        alloc_pipe(STSeriesCaptureJournalFixer)
    };
    move_insert_slices(si,st->context,prototypes,2);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor series_capture_inserters[] =
{
  { STGoalMateReachedTester,     &stip_structure_visitor_noop           },
  { STMove,                      &instrument_move                       },
  { STBeforeSeriesCapture,       &insert_series_capture                 },
  { STKingCaptureLegalityTester, &stip_traverse_structure_children_pipe }
};

enum
{
  nr_series_capture_inserters =
      (sizeof series_capture_inserters
       / sizeof series_capture_inserters[0])
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
                                    series_capture_inserters,
                                    nr_series_capture_inserters);
  stip_traverse_structure(si,&st);

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
void series_capture_recursor_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  ++level;

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  if (levels[level].recurse_from==initsquare)
  {
    pipe_solve_delegate(si);

    TraceValue("%u",level);
    TraceValue("%u",levels[level-1].status);
    TraceEOL();

    if (levels[level-1].status==without_added_movement)
    {
      if (solve_result==previous_move_is_illegal
          || is_in_check(SLICE_STARTER(si))
          || is_in_check(advers(SLICE_STARTER(si))))
        ;
      else
        levels[level-1].status = with_added_movement;
    }
  }
  else
  {
    assert(levels[level-1].status==with_added_movement);
    fork_solve_delegate(si);
  }


  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  --level;

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

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
void series_capture_journal_fixer_solve(slice_index si)
{
  ply ply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO find a more reasonable upper boundary
  for (ply = nbply+1; ply+1<maxply; ++ply)
    move_effect_journal_base[ply+1] = move_effect_journal_base[nbply+1];

  pipe_solve_delegate(si);

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
void series_capture_fork_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_type const type = move_effect_journal[capture].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  assert(levels[level].recurse_from==initsquare);

  if (type==move_effect_piece_removal)
  {
    levels[level].recurse_from = move_effect_journal[movement].u.piece_movement.to;
    fork_solve_delegate(si);
    levels[level].recurse_from = initsquare;
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void switch_to_secondary_movement_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nbply = levels[level].ply_secondary_movement;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void switch_to_regular_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nbply = parent_ply[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void detect_end_of_secondary_movement_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (encore())
    switch_to_regular_ply();
  else
  {
    post_move_iteration_end();
    finply();
    levels[level].status = without_added_movement;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void initialize_secondary_movement_ply(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(levels[level].status==with_added_movement);
  nextply(SLICE_STARTER(si));
  levels[level].ply_secondary_movement = nbply;
  generate_moves_for_piece(levels[level].recurse_from);
  detect_end_of_secondary_movement_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_secondary_movement_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(levels[level].status==with_added_movement);
  switch_to_secondary_movement_ply();
  pop_move();
  detect_end_of_secondary_movement_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void play_secondary_movement(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(levels[level].ply_secondary_movement);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_no_capture(sq_capture))
  {
    move_effect_journal_do_piece_movement(move_effect_reason_series_capture,
                                          sq_departure,
                                          sq_arrival);
    post_move_iteration_solve_delegate(si);
  }
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_series_capture,
                                         sq_capture);
    move_effect_journal_do_piece_movement(move_effect_reason_series_capture,
                                          sq_departure,
                                          sq_arrival);
    levels[level+1].recurse_from = sq_arrival;
    post_move_iteration_solve_delegate(si);
    levels[level+1].recurse_from = initsquare;
  }

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  if (post_move_am_i_iterating())
  {
    assert(levels[level].status==with_added_movement);
    play_secondary_movement(si);
    if (!post_move_iteration_is_locked())
      advance_secondary_movement_ply();
  }
  else
  {
    assert(levels[level].status==without_added_movement);
    post_move_iteration_solve_delegate(si);
    if (levels[level].status==without_added_movement)
      post_move_iteration_end();
    else
      initialize_secondary_movement_ply(si);
  }

  TraceValue("%u",level);
  TraceValue("%u",levels[level].status);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
