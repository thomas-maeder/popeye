#include "conditions/einstein/einstein.h"
#include "pieces/pieces.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Decrease the rank of a piece
 * @param p piece whose rank to decrease
 */
PieNam einstein_decrease_piece(PieNam p)
{
  PieNam result = p;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch (p)
  {
    case Queen:
      result = Rook;
      break;

    case Rook:
      result = Bishop;
      break;

    case Bishop:
      result = Knight;
      break;

    case Knight:
      result = Pawn;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TracePiece(result);
  TraceFunctionResultEnd();
  return result;
}

/* Increase the rank of a piece
 * @param p piece whose rank to increase
 */
PieNam einstein_increase_piece(PieNam p)
{
  PieNam result = p;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch (p)
  {
    case Pawn:
      result = Knight;
      break;

    case Knight:
      result = Bishop;
      break;

    case Bishop:
      result = Rook;
      break;

    case Rook:
      result = Queen;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TracePiece(result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the capturer of the current move (if any)
 * @return departure square of the capturer; initsquare if the current move
 *                   isn't a capture
 */
square einstein_collect_capturers(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    result = move_effect_journal[movement].u.piece_movement.from;
  }
  else
    result = initsquare;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void adjust(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const capturer_origin = einstein_collect_capturers();

    move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
    move_effect_journal_index_type curr;
    assert(move_effect_journal_base[parent_ply[nbply]+1]<=top);
    for (curr = move_effect_journal_base[parent_ply[nbply]+1]; curr!=top; ++curr)
      if (move_effect_journal[curr].type==move_effect_piece_movement
          && (move_effect_journal[curr].reason==move_effect_reason_moving_piece_movement
              || move_effect_journal[curr].reason==move_effect_reason_castling_king_movement
              || move_effect_journal[curr].reason==move_effect_reason_castling_partner_movement))
      {
        square const from = move_effect_journal[curr].u.piece_movement.from;
        square const to = move_effect_journal[curr].u.piece_movement.to;
        PieNam const einsteined = get_walk_of_piece_on_square(to);
        PieNam const substitute = (capturer_origin==from
                                   ? einstein_increase_piece(einsteined)
                                   : einstein_decrease_piece(einsteined));
        if (einsteined!=substitute)
          move_effect_journal_do_piece_change(move_effect_reason_einstein_chess,
                                              to,substitute);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type einstein_moving_adjuster_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_einstein_moving_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STEinsteinArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
