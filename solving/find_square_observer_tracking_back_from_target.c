#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            evalfunction_t *evaluate)
{
  if (number_of_pieces[trait[nbply]][King]>0
      && roicheck(King,evaluate))
    return true;

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

boolean find_square_observer_tracking_back_from_target_non_king(slice_index si,
                                                                evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];

  if (number_of_pieces[side_observing][Pawn]>0
      && pioncheck(Pawn,evaluate))
    return true;

  if (number_of_pieces[side_observing][Knight]>0
      && cavcheck(Knight,evaluate))
    return true;

  if (number_of_pieces[side_observing][Queen]>0
      && ridcheck(vec_queen_start,vec_queen_end,Queen,evaluate))
    return true;

  if (number_of_pieces[side_observing][Rook]>0
      && ridcheck(vec_rook_start,vec_rook_end,Rook,evaluate))
    return true;

  if (number_of_pieces[side_observing][Bishop]>0
      && ridcheck(vec_bishop_start,vec_bishop_end,Bishop,evaluate))
    return true;

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

boolean find_square_observer_tracking_back_from_target_fairy(slice_index si,
                                                             evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  PieNam const *pcheck;

  for (pcheck = checkpieces; *pcheck; ++pcheck)
    if (number_of_pieces[side_observing][*pcheck]>0
        && (*checkfunctions[*pcheck])(*pcheck,evaluate))
      return true;

  return is_square_observed_recursive(slices[si].next1,evaluate);
}
