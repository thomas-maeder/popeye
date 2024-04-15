#include "conditions/series_capture.h"
#include "position/position.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "position/effects/utils.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/binary.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const recursion_landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*recursion_landing==no_slice);

  *recursion_landing = si;
  stip_traverse_structure_children(si,st);
  *recursion_landing = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_series_capture(slice_index si, stip_structure_traversal *st)
{
  slice_index * const recursion_landing = st->param;
  slice_index const save_recursion_landing = *recursion_landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*recursion_landing!=no_slice);
  assert(SLICE_TYPE(*recursion_landing)==STSeriesCaptureRecursionLanding);

  {
    slice_index const landing = alloc_pipe(STLandingAfterSeriesCapture);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const fork = alloc_fork_slice(STSeriesCaptureFork,proxy);

    pipe_append(si,landing);
    pipe_append(si,fork);
    pipe_set_successor(proxy,*recursion_landing);
  }

  *recursion_landing = no_slice;
  stip_traverse_structure_children(si,st);
  *recursion_landing = save_recursion_landing;

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
        alloc_pipe(STSeriesCaptureRecursionLanding),
        alloc_pipe(STBeforeSeriesCapture),
        alloc_pipe(STSeriesCaptureJournalFixer)
    };

    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    move_insert_slices(si,st->context,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_ply_rewinder(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STSeriesCapturePlyRewinder)
    };

    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor series_capture_inserters[] =
{
  { STGoalMateReachedTester,         &stip_structure_visitor_noop           },
  { STDoneGeneratingMoves,           &insert_ply_rewinder                   },
  { STMove,                          &instrument_move                       },
  { STSeriesCaptureRecursionLanding, &remember_landing                      },
  { STBeforeSeriesCapture,           &insert_series_capture                 },
  { STKingCaptureLegalityTester,     &stip_traverse_structure_children_pipe }
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
  slice_index recursion_landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&recursion_landing);
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
void series_capture_ply_rewinder_solve(slice_index si)
{
  ply const save_nbply = nbply;
  ply current_ply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  nextply(SLICE_STARTER(si));
  current_ply = nbply;
  finply();

  TraceValue("%u",save_nbply);
  TraceValue("%u",nbply);
  TraceValue("%u",current_ply);
  TraceEOL();

  nbply = current_ply-1;

  assert(nbply>=save_nbply);
  while (nbply>save_nbply)
  {
    en_passant_top[nbply-1] = en_passant_top[nbply];
    finply();
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
void series_capture_journal_fixer_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_reason_type const reason = move_effect_journal[movement].reason;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (reason==move_effect_reason_series_capture)
  {
    move_effect_journal_base[nbply] = move_effect_journal_base[nbply+1];
    --nbply;
    series_capture_journal_fixer_solve(si);
    ++nbply;
  }
  else
    pipe_solve_delegate(si);

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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void initialize_secondary_movement_ply(slice_index si, square from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));
  generate_moves_for_piece(from);
  detect_end_of_secondary_movement_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_secondary_movement_ply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pop_move();
  detect_end_of_secondary_movement_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void play_secondary_movement(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];

  if (is_no_capture(sq_capture))
    move_effect_journal_do_no_piece_removal();
  else
    move_effect_journal_do_piece_removal(move_effect_reason_series_capture,
                                         sq_capture);

  move_effect_journal_do_piece_movement(move_effect_reason_series_capture,
                                        sq_departure,
                                        sq_arrival);
  post_move_iteration_solve_fork(si);

  /* A nested iteration (e.g. the search for the next Breton victim) may have
   * found that this move is not playable, and we may therefore not have reached
   * our journal fixer; let's do its job here, just in case: */
  move_effect_journal_base[nbply] = move_effect_journal_base[nbply+1];

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
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_type const capture_type = move_effect_journal[capture].type;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const original_to = move_effect_journal[movement].u.piece_movement.to;
  Flags const moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
  PieceIdType const moving_id = GetPieceId(moving_spec);
  square const eventual_to = move_effect_journal_follow_piece_through_other_effects(nbply,moving_id,original_to);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (capture_type==move_effect_piece_removal
      && TSTFLAG(being_solved.spec[eventual_to],trait[nbply]))
  {
    if (post_move_am_i_iterating())
    {
      ++nbply;
      play_secondary_movement(si);

      if (post_move_iteration_is_locked())
        --nbply;
      else
        advance_secondary_movement_ply();
    }
    else
    {
      post_move_iteration_solve_delegate(si);

      if (solve_result==previous_move_is_illegal
          || is_in_check(SLICE_STARTER(si))
          || is_in_check(advers(SLICE_STARTER(si))))
        post_move_iteration_end();
      else
        initialize_secondary_movement_ply(si,eventual_to);
    }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
