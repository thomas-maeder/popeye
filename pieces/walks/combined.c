#include "pieces/walks/combined.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/kangaroo.h"
#include "pieces/pieces.h"

static boolean pawnedpiececheck(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec(sq_target),capturable))
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,evaluate))
      result = true;
  }

  return result;
}

boolean amazone_check(validator_id evaluate)
{
  return  leapers_check(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean empress_check(validator_id evaluate)
{
  return  leapers_check(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean princess_check(validator_id evaluate)
{
  return  leapers_check(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean dragon_check(validator_id evaluate)
{
  if (leapers_check(vec_knight_start,vec_knight_end,evaluate))
    return true;

  return pawnedpiececheck(evaluate);
}

boolean gryphon_check(validator_id evaluate)
{
  return riders_check(vec_bishop_start,vec_bishop_end,evaluate);
}

boolean ship_check(validator_id evaluate)
{
  if (riders_check(vec_rook_start,vec_rook_end,evaluate))
    return true;

  return pawnedpiececheck(evaluate);
}

boolean gral_check(validator_id evaluate)
{
  return leapers_check(vec_alfil_start, vec_alfil_end, evaluate)
      || rider_hoppers_check(vec_rook_start,vec_rook_end, evaluate);
}


boolean scorpion_check(validator_id evaluate)
{
  return  leapers_check(vec_queen_start,vec_queen_end, evaluate)
      || rider_hoppers_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean dolphin_check(validator_id evaluate)
{
  return  rider_hoppers_check(vec_queen_start,vec_queen_end, evaluate)
      || kangaroo_check(evaluate);
}
