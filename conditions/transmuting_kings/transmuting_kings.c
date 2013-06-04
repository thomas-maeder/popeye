#include "conditions/transmuting_kings/transmuting_kings.h"
#include "pydata.h"
#include "solving/observation.h"

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

boolean transmuting_kings_generate_moves(Side side_moving, square sq_departure)
{
  boolean result = false;
  PieNam const *ptrans;

  transmuting_kings_lock_recursion = true;

  for (ptrans = transmpieces[side_moving]; *ptrans!=Empty; ++ptrans)
  {
    piece const ptrans_opponent = side_moving==White ? -*ptrans : *ptrans;
    Side const side_transmuting = advers(side_moving);

    if (number_of_pieces[side_transmuting][*ptrans]>0)
    {
      boolean is_king_transmuted;

      nextply();
      trait[nbply] = side_transmuting;
      is_king_transmuted = (*checkfunctions[*ptrans])(sq_departure,
                                                      *ptrans,
                                                      &validate_observation);
      finply();

      if (is_king_transmuted)
      {
        result = true;
        current_trans_gen = -ptrans_opponent;
        generate_moves_for_piece(side_moving,sq_departure,current_trans_gen);
        current_trans_gen = vide;
      }
    }
  }

  transmuting_kings_lock_recursion = false;

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
