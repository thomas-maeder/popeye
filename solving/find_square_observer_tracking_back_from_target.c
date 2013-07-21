#include "solving/find_square_observer_tracking_back_from_target.h"
#include "pydata.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/vaulting_kings.h"
#include "stipulation/stipulation.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            square sq_target,
                                                            evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];

  if (number_of_pieces[side_observing][King]>0)
  {
    if (calc_reflective_king[side_observing] && !transmuting_kings_lock_recursion)
    {
      if (CondFlag[side_observing==White ? whvault_king : blvault_king])
      {
        if (vaulting_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else if (CondFlag[side_observing==White ? whtrans_king : bltrans_king]
               || CondFlag[side_observing==White ? whsupertrans_king : blsupertrans_king])
      {
        if (transmuting_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else if (CondFlag[side_observing==White ? whrefl_king : blrefl_king])
      {
        if (reflective_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else
      {
        assert(0);
      }
    }
    else if (CondFlag[sting]
             && (*checkfunctions[Grasshopper])(sq_target, King, evaluate))
      return true;
    else if (roicheck(sq_target,King,evaluate))
      return true;
  }

  return find_square_observer_tracking_back_from_target(slices[si].next1,sq_target,evaluate);
}

boolean find_square_observer_tracking_back_from_target(slice_index si,
                                                       square sq_target,
                                                       evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];

  if (number_of_pieces[side_observing][Pawn]>0
      && pioncheck(sq_target,Pawn,evaluate))
    return true;

  if (number_of_pieces[side_observing][Knight]>0
      && cavcheck(sq_target,Knight,evaluate))
    return true;

  if (number_of_pieces[side_observing][Queen]>0
      && ridcheck(sq_target,vec_queen_start,vec_queen_end,Queen,evaluate))
    return true;

  if (number_of_pieces[side_observing][Rook]>0
      && ridcheck(sq_target,vec_rook_start,vec_rook_end,Rook,evaluate))
    return true;

  if (number_of_pieces[side_observing][Bishop]>0
      && ridcheck(sq_target,vec_bishop_start,vec_bishop_end,Bishop,evaluate))
    return true;

  if (flagfee)
  {
    PieNam const *pcheck;
    boolean result = false;

    for (pcheck = checkpieces; *pcheck; ++pcheck)
      if (number_of_pieces[side_observing][*pcheck]>0
          && (*checkfunctions[*pcheck])(sq_target, *pcheck, evaluate))
      {
        result = true;
        break;
      }

    return result;
  }
  else
    return false;
}
