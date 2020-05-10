#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "optimisations/orthodox_square_observation.h"
#include "position/move_diff_code.h"
#include "pieces/pieces.h"

#include "debugging/trace.h"
#include "debugging/assert.h"
#include <stdlib.h>

static vec_index_type current_direction;
static void (*go_on_after)(slice_index si);

/* Is the placed white king in check from a particular direction?
 * @param dir direction
 * @return true iff the placed white king is in check from dir
 */
static boolean check_from_direction(int dir)
{
  square curr = being_solved.king_square[White]-dir;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (is_on_board(curr))
  {
    boolean const is_diagonal = SquareCol(curr)==SquareCol(being_solved.king_square[White]);

    while (is_square_empty(curr))
      curr -= dir;

    if (TSTFLAG(being_solved.spec[curr],Black))
    {
      piece_walk_type const checker = get_walk_of_piece_on_square(curr);
      result = checker==Queen || checker==(is_diagonal ? Bishop : Rook);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue intercepting checks to the placed white king
 */
static void continue_intercepting_checks(slice_index si)
{
  vec_index_type const save_current_direction = current_direction;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do {
    ++current_direction;
  } while (current_direction<=vec_queen_end
           && !check_from_direction(vec[current_direction]));

  if (current_direction<=vec_queen_end)
    intelligent_intercept_check_by_black(si,
                                         vec[current_direction],
                                         &continue_intercepting_checks);
  else
    (*go_on_after)(si);

  current_direction = save_current_direction;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean guards_from(square white_king_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_diff_code[abs(white_king_square-being_solved.king_square[Black])]<9;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Place the white king; intercept checks if necessary
 * @param place_on where to place the king
 * @param go_on what to do after having placed the king?
 */
void intelligent_place_white_king(slice_index si,
                                  square place_on,
                                  void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceSquare(place_on);
  TraceFunctionParamListEnd();

  if (!guards_from(place_on)
      && !is_square_uninterceptably_observed_ortho(Black,place_on)
      && intelligent_reserve_white_king_moves_from_to(white[index_of_king].diagram_square,
                                                      place_on))
  {
    being_solved.king_square[White] = place_on;
    occupy_square(place_on,King,white[index_of_king].flags);

    current_direction = vec_queen_start-1;
    go_on_after = go_on;
    continue_intercepting_checks(si);

    being_solved.king_square[White] = initsquare;
    empty_square(place_on);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
