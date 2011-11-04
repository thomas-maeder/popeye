#include "optimisations/intelligent/mate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "solving/legal_move_finder.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/pin_black_piece.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Place any black white on some square
 * @param placed_on where to place any white piece
 */
static void place_any_white_piece_on(square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    unsigned int placed_index;
    /* don't try to place the white king - there is no mate position where that
     * would make sense */
    for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
      if (white[placed_index].usage==piece_is_unused)
      {
        white[placed_index].usage = piece_intercepts;
        intelligent_place_white_piece(placed_index,
                                      placed_on,
                                      &intelligent_mate_test_target_position);
        white[placed_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

/* Determine whether there are guarding pieces to be neutralised; if so: do it
 * and try again
 * @return true iff guarding pieces had to be neutralized (and the result was
 * tried)
 */
static boolean neutralise_guarding_pieces(void)
{
  square trouble;
  square trto;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  init_legal_move_finder();
  result = slice_has_solution(slices[current_start_slice].u.intelligent_mate_filter.fork)==has_solution;
  trouble = legal_move_finder_departure;
  trto = legal_move_finder_arrival;
  fini_legal_move_finder();

  if (result)
  {
    assert(trouble!=initsquare);

    intelligent_pin_black_piece(trouble,&intelligent_mate_test_target_position);

    {
      int const dir = CheckDir[Queen][trto-trouble];
      if (dir!=0)
      {
        square sq;
        for (sq = trouble+dir; sq!=trto; sq += dir)
          if (nr_reasons_for_staying_empty[sq]==0)
          {
            place_any_black_piece_on(sq);
            place_any_white_piece_on(sq);
            e[sq]= vide;
            spec[sq]= EmptySpec;
          }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));
  if (!neutralise_guarding_pieces())
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].diagram_square!=square_e1
        && intelligent_get_nr_remaining_moves(White)==0)
      fix_white_king_on_diagram_square();
    else if (!exists_redundant_white_piece())
    {
      /* Nail white king to diagram square if no white move remains; we can't do
       * this with the other white or black pieces because they might be
       * captured in the solution */
        solve_target_position();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
