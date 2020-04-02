#include "optimisations/intelligent/intercept_black_move.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Intercept a move with a white piece on some square
 * @param intercept_on where to intercept
 * @param go_on what to do after each successful interception?
 */
static void white_piece_on(slice_index si,
                           square intercept_on,
                           void (*go_on)(slice_index si))
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(intercept_on);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; ++i)
    if (white[i].usage==piece_is_unused)
    {
      white[i].usage = piece_intercepts;
      intelligent_place_white_piece(si,i,intercept_on,go_on);
      white[i].usage = piece_is_unused;
    }

  empty_square(intercept_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a move with a white piece
 * @param dir_from_rider direction from rider giving check to white king
 * @param go_on what to do after each successful interception?
 */
static void white_piece(slice_index si,
                        square from, square to,
                        void (*go_on)(slice_index si))
{
  int const diff = to-from;
  int const dir = CheckDir(Queen)[diff];
  square intercept_on;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(dir!=0);

  for (intercept_on = from+dir; intercept_on!=to; intercept_on += dir)
  {
    assert(is_square_empty(intercept_on));
    if (nr_reasons_for_staying_empty[intercept_on]==0)
      white_piece_on(si,intercept_on,go_on);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a move with a black piece on a square
 * @param intercept_on what square
 * @param go_on what to do after each successful interception?
 */
static void black_piece_on(slice_index si,
                           square intercept_on,
                           boolean is_diagonal,
                           void (*go_on)(slice_index si))
{
  unsigned int intercepter_index;

  TraceFunctionEntry(__func__);
  TraceSquare(intercept_on);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  for (intercepter_index = 1; intercepter_index<MaxPiece[Black]; ++intercepter_index)
    if (black[intercepter_index].usage==piece_is_unused)
    {
      black[intercepter_index].usage = piece_intercepts;
      intelligent_place_black_piece(si,intercepter_index,intercept_on,go_on);
      black[intercepter_index].usage = piece_is_unused;
    }

  empty_square(intercept_on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a move with a black piece
 * @param from departure square of move to be intercepted
 * @param to arrival square
 * @param go_on what to do after each successful interception?
 */
static void black_piece(slice_index si,
                        square from,
                        square to,
                        void (*go_on)(slice_index si))
{
  int const diff = to-from;
  int const dir = CheckDir(Queen)[diff];
  square const start = from+dir;
  boolean const is_diagonal = SquareCol(from)==SquareCol(start);
  square intercept_on;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(dir!=0);

  for (intercept_on = start; intercept_on!=to; intercept_on += dir)
  {
    assert(is_square_empty(intercept_on));
    if (nr_reasons_for_staying_empty[intercept_on]==0
        && *where_to_start_placing_black_pieces<=intercept_on)
      black_piece_on(si,intercept_on,is_diagonal,go_on);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Intercept a black move
 * @param from departure square of move to be intercepted
 * @param to arrival square
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_black_move(slice_index si,
                                      square from, square to,
                                      void (*go_on)(slice_index si))
{
  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1,piece_intercepts))
  {
    black_piece(si,from,to,go_on);
    intelligent_unreserve();
  }

  if (intelligent_reserve_masses(White,1,piece_intercepts))
  {
    white_piece(si,from,to,go_on);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
