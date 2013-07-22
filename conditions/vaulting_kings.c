#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

boolean vaulting_kings_transmuting[nr_sides];
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
      if (!vaulting_kings_transmuting[side_attacking] && roicheck(sq_target,King,evaluate))
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

void vaulting_kings_generate_moves_for_piece(slice_index si,
                                             square sq_departure,
                                             PieNam p)
{
  boolean found_vaulter = false;
  Side const side = trait[nbply];

  if (p==King)
  {
    if (echecc(side))
    {
      PieNam const *pi_vaulter;
      for (pi_vaulter = king_vaulters[side]; *pi_vaulter!=Empty; ++pi_vaulter)
      {
        found_vaulter = true;
        current_trans_gen = *pi_vaulter;
        generate_moves_for_piece(slices[si].next1,sq_departure,*pi_vaulter);
        current_trans_gen = Empty;
      }
    }

    if (found_vaulter && vaulting_kings_transmuting[side])
    {
      /* don't generate non-vaulting moves */
    }
    else
      generate_moves_for_piece(slices[si].next1,sq_departure,King);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);
}

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 */
void vaulting_kings_initalise_solving(slice_index si)
{
  if (CondFlag[whvault_king])
  {
    if (king_vaulters[White][0]==Empty)
    {
      king_vaulters[White][0] = EquiHopper;
      king_vaulters[White][1] = Empty;
    }
    solving_instrument_move_generation(si,White,STVaultingKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,White,STVaultingKingIsSquareObserved);
  }

  if (CondFlag[blvault_king])
  {
    if (king_vaulters[Black][0]==Empty)
    {
      king_vaulters[Black][0] = EquiHopper;
      king_vaulters[Black][1] = Empty;
    }
    solving_instrument_move_generation(si,Black,STVaultingKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,Black,STVaultingKingIsSquareObserved);
  }
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si,
                                         square sq_target,
                                         evalfunction_t *evaluate)
{
  if (number_of_pieces[trait[nbply]][King]>0)
  {
    if (!transmuting_kings_lock_recursion
        && vaulting_kings_is_square_attacked_by_king(sq_target,evaluate))
      return true;
    else
      return is_square_observed_recursive(slices[si].next2,sq_target,evaluate);
  }
  else
    return is_square_observed_recursive(slices[si].next1,sq_target,evaluate);
}
