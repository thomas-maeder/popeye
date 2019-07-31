#include "conditions/take_and_make.h"
#include "pieces/walks/generate_moves.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include <string.h>

static boolean is_not_pawn_make_to_base_line(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = !(is_pawn(get_walk_of_piece_on_square(sq_departure))
             && ((trait[nbply]==White && sq_arrival<=square_h1)
                 || (trait[nbply]==Black && sq_arrival>=square_a8)));

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
void take_and_make_avoid_pawn_make_to_base_line_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_not_pawn_make_to_base_line);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean always_reject(numecoup n)
{
  return false;
}

static void generate_make_for_one_take(numecoup take_current,
                                       square take_capture)
{
  piece_walk_type const taken = get_walk_of_piece_on_square(take_capture);
  Flags const taken_spec = being_solved.spec[take_capture];
  square const take_departure = move_generation_stack[take_current].departure;
  square const take_arrival = move_generation_stack[take_current].arrival;
  numecoup const make_filtered_base = CURRMOVE_OF_PLY(nbply);
  numecoup make_current;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",take_current);
  TraceSquare(take_capture);
  TraceFunctionParamListEnd();

  empty_square(take_capture);
  occupy_square(take_arrival,
                get_walk_of_piece_on_square(take_departure),
                being_solved.spec[take_departure]);
  empty_square(take_departure);

  curr_generation->departure = take_arrival;
  move_generation_current_walk = taken;
  generate_moves_for_piece_based_on_walk();
  curr_generation->departure = take_departure;

  move_generator_filter_captures(make_filtered_base,&always_reject);

  for (make_current = make_filtered_base+1; make_current<=CURRMOVE_OF_PLY(nbply); ++make_current)
  {
    square const make_arrival = move_generation_stack[make_current].arrival;
    move_generation_stack[make_current] = move_generation_stack[take_current];
    move_generation_stack[make_current].arrival = make_arrival;
  }

  occupy_square(take_departure,
                get_walk_of_piece_on_square(take_arrival),
                being_solved.spec[take_arrival]);
  empty_square(take_arrival);
  occupy_square(take_capture,taken,taken_spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
#include "solving/post_move_iteration.h"

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
void take_and_make_generate_make_solve(slice_index si)
{
  numecoup const take_top = CURRMOVE_OF_PLY(nbply);
  numecoup take_current = MOVEBASE_OF_PLY(nbply)+1;
  Side const moving = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(advers(moving)); /* generate according to the taken piece's side */

  for (; take_current<=take_top; ++take_current)
  {
    square const take_capture = move_generation_stack[take_current].capture;
    if (en_passant_is_ep_capture(take_capture))
      generate_make_for_one_take(take_current,
                                 take_capture-offset_en_passant_capture);
    else if (is_no_capture(take_capture))
      push_move_copy(take_current);
    else
      generate_make_for_one_take(take_current,take_capture);
  }

  trait[nbply] = moving; /* move on behalf of to the taking piece's side */

  pipe_solve_delegate(si);

  finply();

  /* prevent the current take generation from disturbing if we generate more
   * moves in the current ply, e.g. while testing for immobility of a side
   */
  pop_all();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_make_generator_avoid_pawn_to_baseline(slice_index si,
                                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STTakeAndMakeGenerateMake),
        alloc_pipe(STTakeAndMakeAvoidPawnMakeToBaseLine),
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_make_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STTakeAndMakeGenerateMake);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_take_and_make(slice_index si)
{
  stip_structure_traversal st;
  boolean collecting_rebirth_squares = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&collecting_rebirth_squares);

  stip_structure_traversal_override_single(&st,
                                           STExecutingKingCapture,
                                           &stip_structure_visitor_noop);

  if (CondFlag[normalp])
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_make_generator);
  else
    /* Extra rule: pawns must not 'make' to their base line */
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_make_generator_avoid_pawn_to_baseline);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
