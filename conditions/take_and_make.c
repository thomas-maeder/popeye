#include "conditions/take_and_make.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <string.h>

static boolean is_not_pawn_make_to_base_line(numecoup n,
                                             square sq_departure,
                                             square sq_arrival,
                                             square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(is_pawn(get_walk_of_piece_on_square(sq_departure))
             && ((trait[nbply]==White && sq_arrival<=square_h1)
                 || (trait[nbply]==Black && sq_arrival>=square_a8)));

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
stip_length_type take_and_make_avoid_pawn_make_to_base_line_solve(slice_index si,
                                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_not_pawn_make_to_base_line);

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
stip_length_type take_and_make_generate_make_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  numecoup const take_top = current_move[nbply];
  numecoup take_current = current_move[nbply-1]+1;
  Side const moving = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (; take_current<=take_top; ++take_current)
  {
    square const take_capture = move_generation_stack[take_current].capture;
    PieNam const taken = get_walk_of_piece_on_square(take_capture);

    if (taken==Empty)
    {
      ++current_move[nbply];
      move_generation_stack[current_move[nbply]] = move_generation_stack[take_current];
    }
    else
    {
      Flags const taken_spec = spec[take_capture];
      square const take_departure = move_generation_stack[take_current].departure;
      square const take_arrival = move_generation_stack[take_current].arrival;
      numecoup make_current = current_move[nbply];
      numecoup make_filtered_top = make_current;

      empty_square(take_capture);
      occupy_square(take_arrival,get_walk_of_piece_on_square(take_departure),spec[take_departure]);
      empty_square(take_departure);

      trait[nbply] = advers(moving);

      generate_moves_for_piece_based_on_walk(take_arrival,taken);

      for (++make_current; make_current<=current_move[nbply]; ++make_current)
        if (is_square_empty(move_generation_stack[make_current].capture))
        {
          /* save the arrival square before possibly overwriting it */
          square const make_arrival = move_generation_stack[make_current].arrival;
          ++make_filtered_top;
          move_generation_stack[make_filtered_top] = move_generation_stack[take_current];
          move_generation_stack[make_filtered_top].arrival = make_arrival;
        }

      current_move[nbply] = make_filtered_top;

      trait[nbply] = moving;

      occupy_square(take_departure,get_walk_of_piece_on_square(take_arrival),spec[take_arrival]);
      empty_square(take_arrival);
      occupy_square(take_capture,taken,taken_spec);
    }
  }

  memmove(&move_generation_stack[current_move[nbply-1]+1],
          &move_generation_stack[take_top+1],
          (current_move[nbply]-take_top) * sizeof move_generation_stack[0]);
  current_move[nbply] -= take_top-current_move[nbply-1];

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    branch_insert_slices_contextual(si,st->context,prototypes,2);
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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_take_and_make(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);

  stip_structure_traversal_override_single(&st,
                                           STBrunnerDefenderFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STKingCaptureLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STMoveLegalityTester,
                                           &stip_traverse_structure_children_pipe);

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
