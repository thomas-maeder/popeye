#include "conditions/woozles.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"
#include <stdlib.h>

boolean woozles_rex_inclusive;

static piece_walk_type woozlers[nr_piece_walks];

typedef enum
{
  is_observed,
  is_mutually_observed,
  is_observation_mutual
} validation_phase;

static validation_phase phase[maxply+1];

static void init_woozlers(void)
{
  unsigned int i = 0;
  piece_walk_type p;

  for (p = King; p<nr_piece_walks; ++p) {
    if (piece_walk_may_exist[p] && p!=Dummy && p!=Hamster)
    {
      woozlers[i] = p;
      i++;
    }
  }

  woozlers[i] = Empty;
}

static boolean find_mutual_observer(void)
{
  Side const side_woozled = trait[nbply-1];
  piece_walk_type const p = get_walk_of_piece_on_square(move_generation_stack[MOVEBASE_OF_PLY(nbply)].departure);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  siblingply(side_woozled);

  push_observation_target(move_generation_stack[MOVEBASE_OF_PLY(nbply)].departure);
  observing_walk[nbply] = p;

  ++phase[parent_ply[nbply]];

  result = fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed_specific[trait[nbply]],
                                                   EVALUATE(observer));

  --phase[parent_ply[nbply]];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_observer_of_observer(Side side_woozle, numecoup n)
{
  square const sq_observer = move_generation_stack[n].departure;
  Side const side_woozled = trait[nbply];
  Flags const mask = BIT(side_woozled)|BIT(Royal);
  boolean result = true;

  if (woozles_rex_inclusive || !TSTFULLFLAGMASK(being_solved.spec[sq_observer],mask))
  {
    piece_walk_type const *pcheck = woozlers;
    numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

    if (!woozles_rex_inclusive)
      ++pcheck;

    siblingply(side_woozle);
    push_observation_target(sq_observer);
    SET_MOVEBASE_OF_PLY(nbply,n); /* allow validation to refer to move n */

    ++phase[parent_ply[nbply]];

    for (; *pcheck; ++pcheck)
    {
      observing_walk[nbply] = *pcheck;
      if (being_solved.number_of_pieces[side_woozle][*pcheck]>0
          && fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed_specific[trait[nbply]],
                                                     EVALUATE(observer)))
      {
        result = false;
        break;
      }
    }

    --phase[parent_ply[nbply]];

    SET_MOVEBASE_OF_PLY(nbply,save_current_move);
    finply();
  }

  return result;
}

static boolean woozles_is_observation_mutual(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;

  return sq_departure==move_generation_stack[CURRMOVE_OF_PLY(nbply-2)].departure;
}

static boolean woozles_can_observe(Side side_woozle, numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_woozle);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  switch (phase[parent_ply[nbply]])
  {
    case is_observed:
      result = find_observer_of_observer(side_woozle,n);
      break;

    case is_mutually_observed:
      result = find_mutual_observer();
      break;

    case is_observation_mutual:
      result = woozles_is_observation_mutual();
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to Woozles
 * @return true iff the observer can observe
 */
boolean woozles_validate_observer(slice_index si)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (woozles_can_observe(side_woozle,CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to BiWoozles
 * @return true iff the observer can observe
 */
boolean biwoozles_validate_observer(slice_index si)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (woozles_can_observe(side_woozle,CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean woozles_is_not_illegal_capture(numecoup n)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = woozles_can_observe(side_woozle,n);

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
void woozles_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&woozles_is_not_illegal_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void woozles_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STWoozlesRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument solving in Woozles
 * @param si identifies the root slice of the stipulation
 */
void woozles_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &woozles_insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observer_validation(si,nr_sides,STWoozlesRemoveIllegalCaptures);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean biwoozles_is_not_illegal_capture(numecoup n)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = woozles_can_observe(side_woozle,n);

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
void biwoozles_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&biwoozles_is_not_illegal_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void biwoozles_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STBiWoozlesRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument solving in BiWoozles
 * @param si identifies the root slice of the stipulation
 */
void biwoozles_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &biwoozles_insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observer_validation(si,nr_sides,STBiWoozlesRemoveIllegalCaptures);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean heffalumps_is_observation_from_heffalumped_on_line(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (sq_departure==move_generation_stack[CURRMOVE_OF_PLY(nbply-2)].departure)
  {
    int cd1= sq_departure%onerow - sq_arrival%onerow;
    int rd1= sq_departure/onerow - sq_arrival/onerow;
    int cd2= move_generation_stack[CURRMOVE_OF_PLY(nbply-2)].arrival%onerow - sq_departure%onerow;
    int rd2= move_generation_stack[CURRMOVE_OF_PLY(nbply-2)].arrival/onerow - sq_departure/onerow;
    int t= 7;

    if (cd1 != 0)
      t= abs(cd1);
    if (rd1 != 0 && t > abs(rd1))
      t= abs(rd1);

    while (!(cd1%t == 0 && rd1%t == 0))
      t--;
    cd1= cd1/t;
    rd1= rd1/t;

    t= 7;
    if (cd2 != 0)
      t= abs(cd2);
    if (rd2 != 0 && t > abs(rd2))
      t= abs(rd2);

    while (!(cd2%t == 0 && rd2%t == 0))
      t--;

    cd2= cd2/t;
    rd2= rd2/t;

    if ((cd1==cd2 && rd1==rd2) || (cd1==-cd2 && rd1==-rd2))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean heffalumps_can_observe_on_line(Side side_woozle, numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_woozle);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  switch (phase[parent_ply[nbply]])
  {
    case is_observed:
      result = find_observer_of_observer(side_woozle,n);
      break;

    case is_mutually_observed:
      result = find_mutual_observer();
      break;

    case is_observation_mutual:
      result = heffalumps_is_observation_from_heffalumped_on_line();
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to Heffalumps
 * @return true iff the observer can observe
 */
boolean heffalumps_validate_observer(slice_index si)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (heffalumps_can_observe_on_line(side_woozle,CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to BiHeffalumps
 * @return true iff the observer can observe
 */
boolean biheffalumps_validate_observer(slice_index si)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (heffalumps_can_observe_on_line(side_woozle,CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean heffalumps_is_not_illegal_capture(numecoup n)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = heffalumps_can_observe_on_line(side_woozle,n);

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
void heffalumps_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&heffalumps_is_not_illegal_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void heffalumps_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STHeffalumpsRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument solving in Heffalumps
 * @param si identifies the root slice of the stipulation
 */
void heffalumps_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &heffalumps_insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observer_validation(si,nr_sides,STHeffalumpsRemoveIllegalCaptures);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean biheffalumps_is_not_illegal_capture(numecoup n)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = heffalumps_can_observe_on_line(side_woozle,n);

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
void biheffalumps_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&biheffalumps_is_not_illegal_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void biheffalumps_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STBiHeffalumpsRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument solving in BiHeffalumps
 * @param si identifies the root slice of the stipulation
 */
void biheffalumps_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &biheffalumps_insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observer_validation(si,nr_sides,STBiHeffalumpsRemoveIllegalCaptures);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
