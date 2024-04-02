#include "conditions/multicaptures.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/king_capture_avoider.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "solving/observation.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>
#include <string.h>

Side multicaptures_who;

static unsigned int counter;

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean multicaptures_initialise_check_detection(slice_index si,
                                                 Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  assert(counter==0);

  result = pipe_is_in_check_recursive_delegate(si,side_in_check);

  counter = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue validating an observation (or observer or observation geometry)
 * @param si identifies the slice with which to continue
 * @return true iff the observation is valid
 */
boolean multi_captures_count_checks(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(counter<=1);

  if (pipe_validate_observation_recursive_delegate(si))
    ++counter;

  result = counter>1;

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
  Side const side_moving = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (multicaptures_who==nr_sides || multicaptures_who==side_moving)
  {
    numecoup const base = MOVEBASE_OF_PLY(nbply);
    numecoup top = CURRMOVE_OF_PLY(nbply);
    numecoup i;

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
  }

  pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with MultiCaptures
 * @param si identifies root slice of solving machinery
 * @param side for who - pass nr_sides for both sides
 */
void multicaptures_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,STMultiCapturesMoveGenerationFilter);

  solving_instrument_check_testing(si,STMultiCapturesInitializeCheckDetection);
  stip_instrument_check_validation(si,multicaptures_who,STMultiCapturesCheckCounter);
  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
