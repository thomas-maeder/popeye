#include "conditions/make_and_take.h"
#include "solving/move_generator.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "position/position.h"
#include "debugging/trace.h"

#include <string.h>

static boolean is_false(numecoup n)
{
  return false;
}

#include "solving/fork.h"

boolean make_and_take_capture_king_as_test_for_check_solve(slice_index si,
                                                           Side side_king_attacked)
{
  boolean result = false;

  numecoup const curr = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(being_solved.king_square[side_king_attacked]);TraceEOL();

  genmove();

  {
    while (CURRMOVE_OF_PLY(nbply)>curr)
    {
      TraceValue("%u",CURRMOVE_OF_PLY(nbply));
      TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture);
      TraceEOL();
      if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture==being_solved.king_square[side_king_attacked])
      {
        extern piece_walk_type observing_walk[maxply+1];
        observing_walk[nbply] = get_walk_of_piece_on_square(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
        if (EVALUATE_OBSERVATION(EVALUATE(check),
                                 move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure,
                                 move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture))
        {
          result = true;
          break;
        }
      }

      --CURRMOVE_OF_PLY(nbply);
    }
  }

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
void make_and_take_generate_captures_by_walk_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  Side const side_capturing = SLICE_STARTER(si);
  Side const side_victim = advers(side_capturing);
  piece_walk_type walk_victim;
  piece_walk_type save_regular_walk = move_generation_current_walk;
  square const save_departure = curr_generation->departure;
  Flags const save_flags = being_solved.spec[save_departure];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceWalk(move_generation_current_walk);
  TraceSquare(curr_generation->departure);
  TraceEnumerator(Side,side_victim);
  TraceEOL();

  for (walk_victim=King; walk_victim!=nr_piece_walks; ++walk_victim)
    if (being_solved.number_of_pieces[side_victim][walk_victim]>0)
    {
      numecoup const base_walk_victim = CURRMOVE_OF_PLY(nbply);

      TraceWalk(walk_victim);
      TraceEOL();

      move_generation_current_walk = walk_victim;
      trait[nbply] = advers(trait[nbply]);
      pipe_move_generation_delegate(si);
      trait[nbply] = advers(trait[nbply]);
      move_generation_current_walk = save_regular_walk;

      move_generator_filter_captures(base_walk_victim,&is_false);

      empty_square(save_departure);

      {
        numecoup const top_walk_victim = CURRMOVE_OF_PLY(nbply);
        numecoup curr_walk_victim;
        for (curr_walk_victim = top_walk_victim;
             curr_walk_victim>base_walk_victim;
             --curr_walk_victim)
        {
          curr_generation->departure = move_generation_stack[curr_walk_victim].arrival;
          occupy_square(curr_generation->departure,save_regular_walk,save_flags);
          pipe_move_generation_delegate(si);
          empty_square(curr_generation->departure);
        }

        curr_generation->departure = save_departure;

        {
          numecoup i;
          numecoup new_top = top_walk_victim;
          for (i = top_walk_victim+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
          {
            square const sq_capture = move_generation_stack[i].capture;
            if (get_walk_of_piece_on_square(sq_capture)==walk_victim
                && TSTFLAG(being_solved.spec[sq_capture],side_victim))
            {
              ++new_top;
              move_generation_stack[new_top] = move_generation_stack[i];
            }
          }

          SET_CURRMOVE(nbply,new_top);
        }

        remove_duplicate_moves_of_single_piece(top_walk_victim);

        memmove(move_generation_stack+base_walk_victim+1,
                move_generation_stack+top_walk_victim+1,
                (CURRMOVE_OF_PLY(nbply)-top_walk_victim) * sizeof move_generation_stack[0]);
        CURRMOVE_OF_PLY(nbply) -= top_walk_victim-base_walk_victim;
      }

      occupy_square(save_departure,save_regular_walk,save_flags);
    }

  {
    numecoup x;
    for (x = CURRMOVE_OF_PLY(nbply); x>curr; --x)
      move_generation_stack[x].departure = save_departure;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_no_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMakeTakeGenerateCapturesWalkByWalk)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* test all attacked pieces, not just the king */
static void substitute_find_move_capturing_king(slice_index si, stip_structure_traversal*st)
{
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STMakeTakeCaptureKingAsTestForCheck));
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_make_and_take(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // move_effect_journal_register_pre_capture_effect();

  move_generator_instrument_for_alternative_paths(si,nr_sides);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_no_capture);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_capture);
    stip_traverse_structure(si,&st);
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STKingSquareObservationTester,
                                             &substitute_find_move_capturing_king);
    stip_traverse_structure(si,&st);
  }

  observation_play_move_to_validate(si,nr_sides);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
