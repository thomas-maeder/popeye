#include "conditions/multicaptures.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/king_capture_avoider.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>
#include <string.h>

boolean multicaptures_who[nr_sides];

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean multicaptures_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  result = pipe_is_in_check_recursive_delegate(si,side_in_check);
  if (result)
  {
    // TODO this won't work for Friends and Orphans
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    assert(is_on_board(sq_departure));
    piece_walk_type save_walk = get_walk_of_piece_on_square(sq_departure);
    Flags const spec = being_solved.spec[sq_departure];

    TraceWalk(save_walk);
    TraceSquare(sq_departure);
    TraceEOL();
    occupy_square(sq_departure,Dummy,spec);
    if (TSTFLAG(spec,White))
    {
      assert(being_solved.number_of_pieces[White][save_walk]>0);
      --being_solved.number_of_pieces[White][save_walk];
    }
    if (TSTFLAG(spec,Black))
    {
      assert(being_solved.number_of_pieces[Black][save_walk]>0);
      --being_solved.number_of_pieces[Black][save_walk];
    }
    result = pipe_is_in_check_recursive_delegate(si,side_in_check);
    occupy_square(sq_departure,save_walk,spec);
    if (TSTFLAG(spec,White))
      ++being_solved.number_of_pieces[White][save_walk];
    if (TSTFLAG(spec,Black))
      ++being_solved.number_of_pieces[Black][save_walk];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static int compare_square_capture(void const *a, void const *b)
{
  move_generation_elmt const * const elmt_a = a;
  move_generation_elmt const * const elmt_b = b;

  if (is_on_board(elmt_a->capture) && !is_on_board(elmt_b->capture))
    return -1;
  else if (!is_on_board(elmt_a->capture) && is_on_board(elmt_b->capture))
    return +1;
  else if (elmt_a->capture<elmt_b->capture)
    return -1;
  else if (elmt_a->capture>elmt_b->capture)
    return +1;
  else if (elmt_a->departure<elmt_b->departure)
    return -1;
  else if (elmt_a->departure>elmt_b->departure)
    return +1;
  else if (elmt_a->arrival<elmt_b->arrival)
    return -1;
  else if (elmt_a->arrival>elmt_b->arrival)
    return +1;
  else
    return 0;
}

/* Filter out single captures
 * @param si identifies generator slice
 */
void multicaptures_filter_singlecaptures(slice_index si)
{
  numecoup const base = MOVEBASE_OF_PLY(nbply);
  numecoup top = CURRMOVE_OF_PLY(nbply);
  numecoup i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",base);
  TraceValue("%u",top);
  TraceEOL();

  for (i = base+1; i<=top; ++i)
  {
    TraceValue("%u",i);
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);
    TraceSquare(move_generation_stack[i].capture);
    TraceEOL();
  }
  TraceEOL();

  qsort(&move_generation_stack[base+1],
        top-base,
        sizeof move_generation_stack[0],
        &compare_square_capture);

  for (i = base+1; i<=top; ++i)
  {
    TraceValue("%u",i);
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);
    TraceSquare(move_generation_stack[i].capture);
    TraceEOL();
  }

  TraceEOL();

  i = base+1;
  while (i<top)
  {
    if (!is_on_board(move_generation_stack[i].capture))
      break;
    else if (i==top)
    {
      --top;
      break;
    }
    else if (move_generation_stack[i].capture==move_generation_stack[i+1].capture)
    {
      do
      {
        ++i;
      } while (i<=top
               && move_generation_stack[i-1].capture==move_generation_stack[i].capture);
    }
    else
    {
      memmove(&move_generation_stack[i],
              &move_generation_stack[i+1],
              sizeof move_generation_stack[0]*(top-i));
      --top;
    }
  }

  CURRMOVE_OF_PLY(nbply) = top;

  TraceValue("%u",base);
  TraceValue("%u",top);
  TraceEOL();

  for (i = base+1; i<=top; ++i)
  {
    TraceValue("%u",i);
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);
    TraceSquare(move_generation_stack[i].capture);
    TraceEOL();
  }

  pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STMultiCapturesMoveGenerationFilter);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with MultiCaptures
 * @param si identifies root slice of solving machinery
 * @param side for who - pass nr_sides for both sides
 */
void multicaptures_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STGeneratingMoves,&insert_filter);
  stip_traverse_structure(si,&st);

  solving_instrument_check_testing(si,STMultiCapturesObserationTester);
  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
