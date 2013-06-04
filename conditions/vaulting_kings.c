#include "conditions/vaulting_kings.h"
#include "pydata.h"
#include "conditions/transmuting_kings/transmuting_kings.h"

#include <assert.h>

boolean calc_transmuting_king[nr_sides];
PieNam king_vaulters[nr_sides][PieceCount];
static unsigned int nr_king_vaulters[nr_sides];

void reset_king_vaulters(void)
{
  nr_king_vaulters[White] = 0;
  nr_king_vaulters[Black] = 0;

  king_vaulters[White][0] = Empty;
  king_vaulters[Black][0] = Empty;
}

void append_king_vaulter(Side side, PieNam p)
{
  king_vaulters[side][nr_king_vaulters[side]] = p;
  ++nr_king_vaulters[side];
  king_vaulters[side][nr_king_vaulters[side]] = Empty;
}

/* Does the king of side trait[nbply] attack a particular square
 * (while vaulting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * true iff the king attacks sq_target
 */
boolean vaulting_kings_is_square_attacked_by_king(square sq_target,
                                                  evalfunction_t *evaluate)
{
  Side const side_attacking = trait[nbply];

  enum
  {
    king_not_vaulting,
    king_vaulting_no_attack,
    attack,
  } result = king_not_vaulting;

  transmuting_kings_lock_recursion = true;

  if (echecc(side_attacking))
  {
    result = king_vaulting_no_attack;

    {
      PieNam const *pi_vaulter;
      for (pi_vaulter = king_vaulters[side_attacking]; *pi_vaulter; ++pi_vaulter)
        if ((*checkfunctions[*pi_vaulter])(sq_target,King,evaluate))
        {
          result = attack;
          break;
        }
    }
  }

  transmuting_kings_lock_recursion = false;

  switch (result)
  {
    case king_not_vaulting:
      if (roicheck(sq_target,King,evaluate))
        return true;
      break;

    case king_vaulting_no_attack:
      if (!calc_transmuting_king[side_attacking] && roicheck(sq_target,King,evaluate))
        return true;
      break;

    case attack:
      return true;
      break;

    default:
      assert(0);
      break;
  }

  return false;
}

boolean vaulting_kings_generate_moves(Side side, square sq_departure)
{
  boolean result = false;

  transmuting_kings_lock_recursion = true;

  if (echecc(side))
  {
    PieNam const *pi_vaulter;
    for (pi_vaulter = king_vaulters[side]; *pi_vaulter!=Empty; ++pi_vaulter)
    {
      result = true;
      current_trans_gen = *pi_vaulter;
      generate_moves_for_piece(side,sq_departure,*pi_vaulter);
      current_trans_gen = vide;
    }
  }

  transmuting_kings_lock_recursion = false;

  return result;
}

void vaulting_kings_initalise_solving(void)
{
  if (king_vaulters[White][0]==Empty)
  {
    king_vaulters[White][0] = EquiHopper;
    king_vaulters[White][1] = Empty;
  }

  if (king_vaulters[Black][0]==Empty)
  {
    king_vaulters[Black][0] = EquiHopper;
    king_vaulters[Black][1] = Empty;
  }
}
