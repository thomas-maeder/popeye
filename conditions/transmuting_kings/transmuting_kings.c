#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

PieNam transmpieces[nr_sides][PieceCount];
boolean transmuting_kings_lock_recursion;

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void init_transmuters_sequence(Side side)
{
  unsigned int tp = 0;
  PieNam p;

  for (p = King; p<PieceCount; ++p) {
    if (may_exist[p] && p!=Dummy && p!=Hamster)
    {
      transmpieces[side][tp] = p;
      tp++;
    }
  }

  transmpieces[side][tp] = Empty;
}

/* Does the transmuting king of side trait[nbply] attack a particular square
 * while transmuting?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return how much attack of the transmuting king to sq_target is there?
 */
transmuting_kings_attack_type
transmuting_kings_is_square_attacked_by_transmuting_king(square sq_target,
                                                         evalfunction_t *evaluate)
{
  Side const side_attacking = trait[nbply];
  Side const side_attacked = advers(side_attacking);
  transmuting_kings_attack_type result = king_not_transmuting;

  PieNam *ptrans;

  transmuting_kings_lock_recursion = true;

  for (ptrans = transmpieces[side_attacking]; *ptrans; ptrans++)
    if (number_of_pieces[side_attacked][*ptrans]>0)
    {
      boolean is_king_transmuted;

      trait[nbply] = advers(trait[nbply]);
      is_king_transmuted = (*checkfunctions[*ptrans])(king_square[side_attacking],*ptrans,evaluate);
      trait[nbply] = advers(trait[nbply]);

      if (is_king_transmuted)
      {
        if ((*checkfunctions[*ptrans])(sq_target,King,evaluate))
        {
          result = king_transmuting_attack;
          break;
        }
        else
          result = king_transmuting_no_attack;
      }
    }

  transmuting_kings_lock_recursion = false;

  return result;
}

/* Does the transmuting king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean transmuting_kings_is_square_attacked_by_king(square sq_target,
                                                     evalfunction_t *evaluate)
{
  switch (transmuting_kings_is_square_attacked_by_transmuting_king(sq_target,evaluate))
  {
    case king_not_transmuting:
      return roicheck(sq_target,King,evaluate);

    case king_transmuting_no_attack:
      return false;

    case king_transmuting_attack:
      return true;

    default:
      assert(0);
      return false;
  }
}

static boolean generate_moves_of_transmuting_king(slice_index si,
                                                  square sq_departure)
{
  boolean result = false;
  PieNam const *ptrans;
  Side const side_moving = trait[nbply];

  for (ptrans = transmpieces[side_moving]; *ptrans!=Empty; ++ptrans)
  {
    Side const side_transmuting = advers(side_moving);

    if (number_of_pieces[side_transmuting][*ptrans]>0)
    {
      boolean is_king_transmuted;

      nextply(side_transmuting);
      is_king_transmuted = (*checkfunctions[*ptrans])(sq_departure,
                                                      *ptrans,
                                                      &validate_observation);
      finply();

      if (is_king_transmuted)
      {
        result = true;
        current_trans_gen = *ptrans;
        generate_moves_for_piece(slices[si].next1,sq_departure,*ptrans);
        current_trans_gen = Empty;
      }
    }
  }

  return result;
}

/* Does the reflective king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean reflective_kings_is_square_attacked_by_king(square sq_target,
                                                    evalfunction_t *evaluate)
{
  switch (transmuting_kings_is_square_attacked_by_transmuting_king(sq_target,evaluate))
  {
    case king_not_transmuting:
    case king_transmuting_no_attack:
      return roicheck(sq_target,King,evaluate);

    case king_transmuting_attack:
      return true;

    default:
      assert(0);
      return false;
  }
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si,
                                                square sq_departure,
                                                PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (p==King)
  {
    if (!generate_moves_of_transmuting_king(si,sq_departure))
      generate_moves_for_piece(slices[si].next1,sq_departure,King);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 */
void transmuting_kings_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[whtrans_king] || CondFlag[whsupertrans_king])
    solving_instrument_move_generation(si,White,STTransmutingKingsMovesForPieceGenerator);
  if (CondFlag[bltrans_king] || CondFlag[blsupertrans_king])
    solving_instrument_move_generation(si,Black,STTransmutingKingsMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void reflective_kings_generate_moves_for_piece(slice_index si,
                                               square sq_departure,
                                               PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (p==King)
  {
    numecoup const save_nbcou = current_move[nbply];
    generate_moves_for_piece(slices[si].next1,sq_departure,King);
    if (generate_moves_of_transmuting_king(si,sq_departure))
      remove_duplicate_moves_of_single_piece(save_nbcou);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 */
void reflective_kings_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[whrefl_king])
    solving_instrument_move_generation(si,White,STReflectiveKingsMovesForPieceGenerator);
  if (CondFlag[blrefl_king])
    solving_instrument_move_generation(si,Black,STReflectiveKingsMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
