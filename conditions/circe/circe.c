#include "conditions/circe/circe.h"
#include "conditions/circe/capture_fork.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include <assert.h>

square current_circe_rebirth_square[maxply+1];

PieNam current_circe_reborn_piece[maxply+1];
Flags current_circe_reborn_spec[maxply+1];

PieNam current_circe_relevant_piece[maxply+1];
Flags current_circe_relevant_spec[maxply+1];
Side current_circe_relevant_side[maxply+1];

move_effect_reason_type current_circe_rebirth_reason[maxply+1];

square (*circe_determine_rebirth_square)(PieNam, Flags, square, square, square, Side);

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
stip_length_type circe_determine_reborn_piece_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  Flags const removedspec = move_effect_journal[capture].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* circe capture fork makes sure of that */
  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  current_circe_reborn_piece[nbply] = move_effect_journal[capture].u.piece_removal.removed;
  current_circe_reborn_spec[nbply] = removedspec;

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
stip_length_type circe_determine_relevant_piece_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_circe_relevant_piece[nbply] = current_circe_reborn_piece[nbply];
  current_circe_relevant_spec[nbply] = current_circe_reborn_spec[nbply];
  current_circe_relevant_side[nbply] = slices[si].starter;

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
stip_length_type circe_determine_rebirth_square_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  current_circe_rebirth_reason[nbply] = move_effect_reason_circe_rebirth;
  current_circe_rebirth_square[nbply] = (*circe_determine_rebirth_square)(current_circe_relevant_piece[nbply],
                                                      current_circe_relevant_spec[nbply],
                                                      sq_capture,
                                                      sq_departure,
                                                      sq_arrival,
                                                      current_circe_relevant_side[nbply]);

  result = solve(slices[si].next1,n);
  current_circe_rebirth_reason[nbply] = move_effect_no_reason;

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
stip_length_type circe_place_reborn_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_circe_reborn_piece[nbply]!=Empty
      && is_square_empty(current_circe_rebirth_square[nbply]))
  {
    assert(current_circe_rebirth_reason[nbply]!=move_effect_no_reason);
    move_effect_journal_do_piece_readdition(current_circe_rebirth_reason[nbply],
                                            current_circe_rebirth_square[nbply],
                                            current_circe_reborn_piece[nbply],
                                            current_circe_reborn_spec[nbply]);
  }
  else
    current_circe_rebirth_square[nbply] = initsquare;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void replace(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const substitute = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  slices[si].type = *substitute;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Use an alternative type of slices for determining the reborn piece
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_reborn_piece(slice_index si,
                                               slice_type substitute)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,substitute,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&substitute);
  stip_structure_traversal_override_single(&st,STCirceDetermineRebornPiece,&replace);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Use an alternative type of slices for determining the piece relevant for
 * determining the rebirth square
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_relevant_piece(slice_index si,
                                                 slice_type substitute)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,substitute,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&substitute);
  stip_structure_traversal_override_single(&st,STCirceDetermineRelevantPiece,&replace);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STCirceDetermineRelevantPiece);
  stip_instrument_moves(si,STCirceDetermineRebirthSquare);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

square renrank(PieNam p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  square sq= ((sq_capture/onerow)%2==1
              ? rennormal(p_captured,p_captured_spec,
                          sq_capture,sq_departure,sq_arrival,capturer)
              : renspiegel(p_captured,p_captured_spec,
                           sq_capture,sq_departure,sq_arrival,capturer));
  return onerow*(sq_capture/onerow) + sq%onerow;
}

square renfile(PieNam p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer)
{
  int col= sq_capture % onerow;
  square result;

  TraceFunctionEntry(__func__);
  TracePiece(p_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (capturer==Black)
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      result = col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

square renspiegelfile(PieNam p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(p_captured,p_captured_spec,
                 sq_capture,sq_departure,sq_arrival,advers(capturer));
} /* renspiegelfile */

square renpwc(PieNam p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents(PieNam p_captured, Flags p_captured_spec,
                       square sq_capture,
                       square sq_departure, square sq_arrival,
                       Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_capture + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents */

square renequipollents_anti(PieNam p_captured, Flags p_captured_spec,
                            square sq_capture,
                            square sq_departure, square sq_arrival,
                            Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_arrival + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents_anti */

square rensymmetrie(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (square_h8+square_a1) - sq_capture;
} /* rensymmetrie */

square renantipoden(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  int const row= sq_capture/onerow - nr_of_slack_rows_below_board;
  int const file= sq_capture%onerow - nr_of_slack_files_left_of_board;

  sq_departure= sq_capture;

  if (row<nr_rows_on_board/2)
    sq_departure+= nr_rows_on_board/2*dir_up;
  else
    sq_departure+= nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    sq_departure+= nr_files_on_board/2*dir_right;
  else
    sq_departure+= nr_files_on_board/2*dir_left;

  return sq_departure;
} /* renantipoden */

square rendiagramm(PieNam p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal(PieNam pnam_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  unsigned int col = sq_capture % onerow;
  unsigned int const ran = sq_capture / onerow;

  TraceFunctionEntry(__func__);
  TracePiece(pnam_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (CondFlag[circemalefiquevertical])
  {
    col = onerow-1 - col;
    if (pnam_captured==Queen)
      pnam_captured = King;
    else if (pnam_captured==King)
      pnam_captured = Queen;
  }

  {
    Side const cou = (ran&1) != (col&1) ? White : Black;

    if (capturer == Black)
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+1)*onerow;
      else if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_b1 : square_g1;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_h1 : square_a1;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d1;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_f1 : square_c1;
      else if (pnam_captured==standard_walks[King])
        Result = square_e1;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
    }
    else
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
      else if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
      else if (pnam_captured==standard_walks[King])
        Result = square_e8;
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_g8 : square_b8;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_a8 : square_h8;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d8;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_c8 : square_f8;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(Result);
  TraceFunctionResultEnd();
  return(Result);
}

square rendiametral(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (square_h8+square_a1
          - rennormal(p_captured,p_captured_spec,
                      sq_capture,sq_departure,sq_arrival,capturer));
}

square renspiegel(PieNam p_captured, Flags p_captured_spec,
                  square sq_capture,
                  square sq_departure, square sq_arrival,
                  Side capturer)
{
  return rennormal(p_captured,p_captured_spec,
                   sq_capture,sq_departure,sq_arrival,advers(capturer));
}
