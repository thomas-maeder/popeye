#include "solving/en_passant.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "position/pieceid.h"
#include "conditions/actuated_revolving_centre.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

square en_passant_multistep_over[2][maxply+1];

/* Remember a square avoided by a multistep move of a pawn
 * @param s avoided square
 */
void en_passant_remember_multistep_over(square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(s!=initsquare);
  en_passant_multistep_over[0][nbply] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void en_passant_remember_multistep_over2(square s1, square s2)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s1);
  TraceSquare(s2);
  TraceFunctionParamListEnd();

  assert(s1!=initsquare);
  assert(s2!=initsquare);
  en_passant_multistep_over[0][nbply] = s1;
  en_passant_multistep_over[1][nbply] = s2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  en_passant_multistep_over[0][nbply] = initsquare;
  en_passant_multistep_over[1][nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply)
{
  boolean const result = en_passant_multistep_over[0][ply]!=initsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the en passant capturee of the current ply. Only meaningful if an en
 * passant capture is actually possible, which isn't tested here.
 * @return position of the capturee
 *         initsquare if the capturee vanished from the board
 */
square en_passant_find_capturee(void)
{
  ply const ply_parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[ply_parent-1];
  move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
  PieceIdType const capturee_id = GetPieceId(move_effect_journal[parent_movement].u.piece_movement.movingspec);
  move_effect_journal_index_type other;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[ply_parent];
  square result = move_effect_journal[parent_movement].u.piece_movement.to;

  for (other = parent_base+move_effect_journal_index_offset_other_effects;
       other<parent_top;
       ++other)
    switch (move_effect_journal[other].type)
    {
      case move_effect_piece_removal:
        if (move_effect_journal[other].u.piece_removal.from==result)
        {
          assert(GetPieceId(move_effect_journal[other].u.piece_removal.removedspec)==capturee_id);
          result = initsquare;
        }
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        if (GetPieceId(move_effect_journal[other].u.piece_addition.addedspec)==capturee_id)
        {
          assert(result==initsquare);
          result = move_effect_journal[other].u.piece_addition.on;
        }
        break;

      case move_effect_piece_movement:
        if (move_effect_journal[other].u.piece_movement.from==result)
        {
          assert(GetPieceId(move_effect_journal[other].u.piece_movement.movingspec)==capturee_id);
          result = move_effect_journal[other].u.piece_movement.to;
        }
        break;

      case move_effect_piece_exchange:
        if (move_effect_journal[other].u.piece_exchange.from==result)
          result = move_effect_journal[other].u.piece_exchange.to;
        else if (move_effect_journal[other].u.piece_exchange.to==result)
          result = move_effect_journal[other].u.piece_exchange.from;
        break;

      case move_effect_board_transformation:
        result = transformSquare(result,move_effect_journal[other].u.board_transformation.transformation);
        break;

      case move_effect_centre_revolution:
        result = actuated_revolving_centre_revolve_square(result);
        break;

      case move_effect_none:
      case move_effect_no_piece_removal:
      case move_effect_piece_change:
      case move_effect_side_change:
      case move_effect_king_square_movement:
      case move_effect_flags_change:
      case move_effect_imitator_addition:
      case move_effect_imitator_movement:
      case move_effect_remember_ghost:
      case move_effect_forget_ghost:
      case move_effect_neutral_recoloring_do:
      case move_effect_neutral_recoloring_undo:
        /* nothing */
        break;

      default:
        assert(0);
        break;
    }

  return result;
}

/* Determine whether side trait[nbply] gives check by en_passant_multistep_over[0]. capture
 * @param tester pawn-specific tester function
 * @param evaluate address of evaluater function
 * @return true if side trait[nbply] gives check by en_passant_multistep_over[0]. capture
 */
boolean en_passant_test_check(en_passant_check_tester_type tester,
                              evalfunction_t *evaluate)
{
  Side const side_in_check = advers(trait[nbply]);

  if (king_square[side_in_check]==en_passant_find_capturee())
  {
    square sq_arrival = en_passant_multistep_over[0][parent_ply[nbply]];
    if (sq_arrival!=initsquare
        && (*tester)(sq_arrival,king_square[side_in_check],evaluate))
      return true;

    sq_arrival = en_passant_multistep_over[1][parent_ply[nbply]]; /* Einstein triple step */
    if (sq_arrival!=initsquare
        && (*tester)(sq_arrival,king_square[side_in_check],evaluate))
      return true;
  }

  return false;
}

/* Is an en passant capture possible to a specific square?
 * @param side for which side
 * @param s the square
 * @return true iff an en passant capture to s is currently possible
 */
boolean en_passant_is_capture_possible_to(Side side, square s)
{
  boolean result;
  ply const ply_parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(s);
  TraceFunctionParamListEnd();

  result = (trait[ply_parent]!=side
            && (en_passant_multistep_over[0][ply_parent]==s || en_passant_multistep_over[1][ply_parent]==s));

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
