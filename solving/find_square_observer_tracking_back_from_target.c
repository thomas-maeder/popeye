#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

PieNam observing_walk[maxply+1];

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            evalfunction_t *evaluate)
{
  observing_walk[nbply] = King;
  if (number_of_pieces[trait[nbply]][King]>0
      && roicheck(evaluate))
    return true;

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

boolean find_square_observer_tracking_back_from_target_non_king(slice_index si,
                                                                evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];

  observing_walk[nbply] = Pawn;
  if (number_of_pieces[side_observing][Pawn]>0
      && pioncheck(evaluate))
    return true;

  observing_walk[nbply] = Knight;
  if (number_of_pieces[side_observing][Knight]>0
      && cavcheck(evaluate))
    return true;

  observing_walk[nbply] = Queen;
  if (number_of_pieces[side_observing][Queen]>0
      && ridcheck(vec_queen_start,vec_queen_end,evaluate))
    return true;

  observing_walk[nbply] = Rook;
  if (number_of_pieces[side_observing][Rook]>0
      && ridcheck(vec_rook_start,vec_rook_end,evaluate))
    return true;

  observing_walk[nbply] = Bishop;
  if (number_of_pieces[side_observing][Bishop]>0
      && ridcheck(vec_bishop_start,vec_bishop_end,evaluate))
    return true;

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

boolean find_square_observer_tracking_back_from_target_fairy(slice_index si,
                                                             evalfunction_t *evaluate)
{
  Side const side_observing = trait[nbply];
  PieNam const *pcheck;

  for (pcheck = checkpieces; *pcheck; ++pcheck)
  {
    observing_walk[nbply] = *pcheck;
    if (number_of_pieces[side_observing][*pcheck]>0
        && (*checkfunctions[*pcheck])(evaluate))
      return true;
  }

  return is_square_observed_recursive(slices[si].next1,evaluate);
}
