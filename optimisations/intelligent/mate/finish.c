#include "optimisations/intelligent/mate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "solving/legal_move_finder.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/place_white_king.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Place any black piece on some square
 * @param placed_on where to place any black piece
 */
static void place_any_black_piece_on(square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
      if (black[placed_index].usage==piece_is_unused)
      {
        black[placed_index].usage = piece_intercepts;
        intelligent_place_black_piece(placed_index,
                                      placed_on,
                                      &intelligent_mate_test_target_position);
        black[placed_index].usage = piece_is_unused;
      }

    e[placed_on]= vide;
    spec[placed_on]= EmptySpec;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there a redundant white piece in the curren position?
 * @return true iff there is a redundant piece
 */
static boolean exists_redundant_white_piece(void)
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* check for redundant white pieces */
  for (bnp = boardnum; !result && *bnp!=initsquare; bnp++)
  {
    square const sq = *bnp;
    if (sq!=king_square[White] && e[sq]>obs)
    {
      piece const p = e[sq];
      Flags const sp = spec[sq];

      /* remove piece */
      e[sq] = vide;
      spec[sq] = EmptySpec;

      result = slice_has_solution(slices[current_start_slice].u.intelligent_mate_filter.goal_tester_fork)==has_solution;

      /* restore piece */
      e[sq] = p;
      spec[sq] = sp;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find a black king flight that may have been created while placing pieces
 * after guarding and blocking
 * @return the first flight square found; initsquare if none was found
 */
static square find_king_flight(void)
{
  slice_index const finder = slices[current_start_slice].u.intelligent_mate_filter.fork;
  square result;

  init_legal_move_finder();
  result = slice_has_solution(finder)==has_solution ? legal_move_finder_arrival : initsquare;
  fini_legal_move_finder();

  return result;
}

/* fix the white king on its diagram square
 */
static void fix_white_king_on_diagram_square(void)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0)
  {
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;
    intelligent_place_white_king(king_diagram_square,
                                 &intelligent_mate_test_target_position);
    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Test the position created by the taken operations; if the position is a
 * solvable target position: solve it; otherwise: improve it
 */
void intelligent_mate_test_target_position(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));

  {
    square const flight = find_king_flight();
    if (flight==initsquare)
    {
      /* Nail white king to diagram square if no white move remains; we can't do
       * this with the other unused white or black pieces because they might be
       * captured in the solution */
      if (white[index_of_king].usage==piece_is_unused
          && white[index_of_king].diagram_square!=square_e1
          && intelligent_get_nr_remaining_moves(White)==0)
        fix_white_king_on_diagram_square();
      else if (!exists_redundant_white_piece())
        solve_target_position();
    }
    else if (e[flight]==vide)
      place_any_black_piece_on(flight);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
