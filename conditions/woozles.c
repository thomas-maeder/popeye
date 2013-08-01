#include "conditions/woozles.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <stdlib.h>

boolean woozles_rex_exclusive;

static PieNam woozlers[PieceCount];

static void init_woozlers(void)
{
  unsigned int i = 0;
  PieNam p;

  for (p = King; p<PieceCount; ++p) {
    if (may_exist[p] && p!=Dummy && p!=Hamster)
    {
      woozlers[i] = p;
      i++;
    }
  }

  woozlers[i] = Empty;
}

static boolean woozles_aux_whx(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  return sq_departure==move_generation_stack[current_move[nbply-2]-1].departure && validate_observation_geometry();
}

static boolean heffalumps_aux_whx(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;
  if (sq_departure==move_generation_stack[current_move[nbply-2]-1].departure)
  {
    int cd1= sq_departure%onerow - sq_arrival%onerow;
    int rd1= sq_departure/onerow - sq_arrival/onerow;
    int cd2= move_generation_stack[current_move[nbply-2]-1].arrival%onerow - sq_departure%onerow;
    int rd2= move_generation_stack[current_move[nbply-2]-1].arrival/onerow - sq_departure/onerow;
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
      return validate_observation_geometry();
  }

  return false;
}

static boolean woozles_aux_wh(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (validate_observation_geometry())
  {
    Side const side_woozled = trait[nbply-1];
    PieNam const p = get_walk_of_piece_on_square(move_generation_stack[current_move[nbply-1]-1].departure);
    if (number_of_pieces[side_woozled][p]>0)
    {
      siblingply(side_woozled);
      current_move[nbply] = current_move[nbply-1]+1;
      move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;
      move_generation_stack[current_move[nbply]-1].capture = move_generation_stack[current_move[nbply-1]-1].departure;
      result = (*checkfunctions[p])(p,&woozles_aux_whx);
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean heffalumps_aux_wh(void)
{
  boolean result = false;

  if (validate_observation_geometry())
  {
    Side const side_woozled = trait[nbply-1];
    PieNam const p = get_walk_of_piece_on_square(move_generation_stack[current_move[nbply-1]-1].departure);
    if (number_of_pieces[side_woozled][p]>0)
    {
      siblingply(side_woozled);
      current_move[nbply] = current_move[nbply-1]+1;
      move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;
      move_generation_stack[current_move[nbply]-1].capture = move_generation_stack[current_move[nbply-1]-1].departure;
      result = (*checkfunctions[p])(p,&heffalumps_aux_whx);
      finply();
    }
  }

  return result;
}

static boolean woozles_is_paralysed(Side side_woozle, numecoup n)
{
  square const sq_observer = move_generation_stack[n].departure;
  Side const side_woozled = trait[nbply];
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_woozle,"");
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  if (!woozles_rex_exclusive || sq_observer!=king_square[side_woozled])
  {
    PieNam const *pcheck = woozlers;
    numecoup const save_current_move = current_move[nbply]-1;

    if (woozles_rex_exclusive)
      ++pcheck;

    siblingply(side_woozle);
    current_move[nbply] = current_move[nbply-1]+1;
    current_move[nbply-1] = n+1; /* allow validation to refer to move n */
    move_generation_stack[current_move[nbply]-1].capture = sq_observer;
    move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;

    for (; *pcheck; ++pcheck)
      if (number_of_pieces[side_woozle][*pcheck]>0
          && (*checkfunctions[*pcheck])(*pcheck,&woozles_aux_wh))
      {
        result = true;
        break;
      }

    current_move[nbply-1] = save_current_move+1;
    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean heffalumps_is_paralysed(Side side_woozle, numecoup n)
{
  square const sq_observer = move_generation_stack[n].departure;
  Side const side_woozled = trait[nbply];
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_woozle,"");
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  if (!woozles_rex_exclusive || sq_observer!=king_square[side_woozled])
  {
    PieNam const *pcheck = woozlers;
    numecoup const save_current_move = current_move[nbply]-1;

    if (woozles_rex_exclusive)
      ++pcheck;

    siblingply(side_woozle);
    current_move[nbply] = current_move[nbply-1]+1;
    current_move[nbply-1] = n+1; /* allow validation to refer to move n */
    move_generation_stack[current_move[nbply]-1].auxiliary.hopper.sq_hurdle = initsquare;
    move_generation_stack[current_move[nbply]-1].capture = sq_observer;

    for (; *pcheck; ++pcheck)
      if (number_of_pieces[side_woozle][*pcheck]>0
          && (*checkfunctions[*pcheck])(*pcheck,&heffalumps_aux_wh))
      {
        result = true;
        break;
      }

    current_move[nbply-1] = save_current_move+1;
    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Woozles
 * @return true iff the observation is valid
 */
boolean woozles_validate_observation(slice_index si)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!woozles_is_paralysed(side_woozle,current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to BiWoozles
 * @return true iff the observation is valid
 */
boolean biwoozles_validate_observation(slice_index si)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!woozles_is_paralysed(side_woozle,current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Heffalumps
 * @return true iff the observation is valid
 */
boolean heffalumps_validate_observation(slice_index si)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!heffalumps_is_paralysed(side_woozle,current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to BiHeffalumps
 * @return true iff the observation is valid
 */
boolean biheffalumps_validate_observation(slice_index si)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!heffalumps_is_paralysed(side_woozle,current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean woozles_is_not_illegal_capture(numecoup n,
                                              square sq_departure,
                                              square sq_arrival,
                                              square sq_capture)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(!is_square_empty(sq_capture)
             && woozles_is_paralysed(side_woozle,n));

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
stip_length_type woozles_remove_illegal_captures_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&woozles_is_not_illegal_capture);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void woozles_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STWoozlesRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

  stip_instrument_observation_validation(si,nr_sides,STValidateObservationWoozles);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean biwoozles_is_not_illegal_capture(numecoup n,
                                                square sq_departure,
                                                square sq_arrival,
                                                square sq_capture)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(!is_square_empty(sq_capture)
             && woozles_is_paralysed(side_woozle,n));

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
stip_length_type biwoozles_remove_illegal_captures_solve(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&biwoozles_is_not_illegal_capture);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void biwoozles_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STBiWoozlesRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

  stip_instrument_observation_validation(si,nr_sides,STValidateObservationBiWoozles);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean heffalumps_is_not_illegal_capture(numecoup n,
                                                 square sq_departure,
                                                 square sq_arrival,
                                                 square sq_capture)
{
  boolean result;
  Side const side_woozle = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(!is_square_empty(sq_capture)
             && heffalumps_is_paralysed(side_woozle,n));

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
stip_length_type heffalumps_remove_illegal_captures_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&heffalumps_is_not_illegal_capture);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void heffalumps_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STHeffalumpsRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

  stip_instrument_observation_validation(si,nr_sides,STValidateObservationHeffalumps);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean biheffalumps_is_not_illegal_capture(numecoup n,
                                                   square sq_departure,
                                                   square sq_arrival,
                                                   square sq_capture)
{
  boolean result;
  Side const side_woozle = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(!is_square_empty(sq_capture)
             && heffalumps_is_paralysed(side_woozle,n));

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
stip_length_type biheffalumps_remove_illegal_captures_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&biheffalumps_is_not_illegal_capture);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void biheffalumps_insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STBiHeffalumpsRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

  stip_instrument_observation_validation(si,nr_sides,STValidateObservationBiHeffalumps);

  init_woozlers();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
