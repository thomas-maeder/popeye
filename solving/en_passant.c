#include "solving/en_passant.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

square ep[maxply+1];

/* Remember a square avoided by a multistep move of a pawn
 * @param s avoided square
 */
void en_passant_remember_multistep_over(square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(s!=initsquare);
  ep[nbply] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ep[nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply)
{
  boolean const result = ep[ply]!=initsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is an en passant capture possible to a specific square?
 * @param s the square
 * @return true iff an en passant capture to s is currently possible
 */
boolean en_passant_is_capture_possible_to(square s)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  result = ep[parent_ply[nbply]]==s;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find out whether the pawn that has just moved (if any) has the potential
 * of being captured en passant
 * @param sq_multistep_departure departure square of pawn move
 * @return the square avoided by the multistep; initsquare if no multistep
 */
square en_passant_find_potential(square sq_multistep_departure)
{
  square result = initsquare;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  PieNam const pi_moving = abs(move_effect_journal[movement].u.piece_movement.moving);
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_multistep_departure);
  TraceFunctionParamListEnd();

  switch (pi_moving)
  {
    case Pawn:
    case MarinePawn:
    {
      numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==White
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      if (sq_arrival-sq_multistep_departure==2*dir_forward
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    case BerolinaPawn:
    {
      numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==White
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      numvec const v = sq_arrival-sq_multistep_departure;
      if ((v==2*dir_forward+2*dir_left || v==2*dir_forward+2*dir_right)
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    case ReversePawn:
    {
      numvec const dir_backward = trait[nbply]==Black ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==Black
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      if (sq_arrival-sq_multistep_departure==2*dir_backward
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
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
stip_length_type en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  piece const pi_moving = move_effect_journal[movement].u.piece_movement.moving;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_pawn(abs(pi_moving))
      && move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    square const multistep_over = en_passant_find_potential(move_effect_journal[movement].u.piece_movement.from);
    if (multistep_over!=initsquare)
    {
      en_passant_remember_multistep_over(multistep_over);
      result = solve(slices[si].next1,n);
      en_passant_forget_multistep();
    }
    else
      result = solve(slices[si].next1,n);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
