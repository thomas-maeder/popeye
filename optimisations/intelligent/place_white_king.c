#include "optimisations/intelligent/place_white_king.h"
#include "pydata.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static vec_index_type current_direction;
static void (*go_on_after)(void);

/* Is the placed white king in check from a particular direction?
 * @param dir direction
 * @return true iff the placed white king is in check from dir
 */
static boolean check_from_direction(int dir)
{
  square curr = king_square[White]-dir;
  boolean const is_diagonal = SquareCol(curr)==SquareCol(king_square[White]);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  while (e[curr]==vide)
    curr -= dir;

  result = e[curr]==dn || e[curr]==(is_diagonal ? fn : tn);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue intercepting checks to the placed white king
 */
static void continue_intercepting_checks(void)
{
  vec_index_type const save_current_direction = current_direction;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do {
    ++current_direction;
  } while (current_direction<=vec_queen_end
           && !check_from_direction(vec[current_direction]));

  if (current_direction<=vec_queen_end)
    intelligent_intercept_check_by_black(vec[current_direction],
                                         &continue_intercepting_checks);
  else
    (*go_on_after)();

  current_direction = save_current_direction;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean guards_from(square white_king_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_diff_code[abs(white_king_square-king_square[Black])]<9;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean uninterceptably_attacked(square s)
{
  boolean result;

  nextply();
  trait[nbply] = Black;

  result = ((*checkfunctions[Pawn])(s,Pawn,eval_ortho)
            || (*checkfunctions[Knight])(s,Knight,eval_ortho)
            || (*checkfunctions[Fers])(s,Bishop,eval_ortho)
            || (*checkfunctions[Wesir])(s,Rook,eval_ortho)
            || (*checkfunctions[ErlKing])(s,Queen,eval_ortho));

  finply();

  return result;
}

/* Place the white king; intercept checks if necessary
 * @param place_on where to place the king
 * @param go_on what to do after having placed the king?
 */
void intelligent_place_white_king(square place_on, void (*go_on)(void))
{
  TraceFunctionEntry(__func__);
  TraceSquare(place_on);
  TraceFunctionParamListEnd();

  if (!guards_from(place_on)
      && !uninterceptably_attacked(place_on)
      && intelligent_reserve_white_king_moves_from_to(white[index_of_king].diagram_square,
                                                      place_on))
  {
    king_square[White] = place_on;
    SetPiece(roib,place_on,white[index_of_king].flags);

    current_direction = vec_queen_start-1;
    go_on_after = go_on;
    continue_intercepting_checks();

    king_square[White] = initsquare;
    e[place_on] = vide;
    spec[place_on] = EmptySpec;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
