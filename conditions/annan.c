#include "conditions/annan.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

annan_type_type annan_type;

static boolean annanises(Side side, square rear, square front)
{
  if (TSTFLAG(spec[rear],side))
    switch(annan_type)
    {
      case annan_type_A:
        return true;

      case annan_type_B:
        return rear!=king_square[side];

      case annan_type_C:
        return front!=king_square[side];

      case annan_type_D:
        return rear!=king_square[side] && front!=king_square[side];

      default:
        assert(0);
        return true;
    }
  else
    return false;
}

/* Determine whether a square is observed in Annan Chess
* @param si identifies tester slice
* @param sq_target square potentially observed
* @return true iff sq_target is observed
*/
boolean annan_is_square_observed(slice_index si,
                                 square sq_target,
                                 evalfunction_t *evaluate)
{
  Side const side_attacking = trait[nbply];
  numvec const dir_annaniser = side_attacking==White ? dir_down : dir_up;
  square annan_sq[nr_squares_on_board];
  PieNam annan_p[nr_squares_on_board];
  int annan_cnt = 0;
  boolean result;
  unsigned int i;
  square square_a = side_attacking==White ? square_a8 : square_a1;

  for (i = nr_rows_on_board-1; i>0; --i, square_a += dir_annaniser)
  {
    square pos_annanised = square_a;
    unsigned int j;
    for (j = nr_files_on_board; j>0; --j, ++pos_annanised)
    {
      square const pos_annaniser = pos_annanised+dir_annaniser;
      if (TSTFLAG(spec[pos_annanised],side_attacking)
          && annanises(side_attacking,pos_annaniser,pos_annanised))
      {
        annan_sq[annan_cnt] = pos_annanised;
        annan_p[annan_cnt] = get_walk_of_piece_on_square(pos_annanised);
        ++annan_cnt;
        replace_piece(pos_annanised,get_walk_of_piece_on_square(pos_annaniser));
      }
    }
  }

  result = is_square_observed_recursive(slices[si].next1,sq_target,evaluate);

  while (annan_cnt--)
    replace_piece(annan_sq[annan_cnt],annan_p[annan_cnt]);

  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void annan_generate_moves_for_piece(slice_index si,
                                    square sq_departure,
                                    PieNam p)
{
  int const annaniser_dir = trait[nbply]==White ? -onerow : +onerow;
  square const annaniser_pos = sq_departure+annaniser_dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (annanises(trait[nbply],annaniser_pos,sq_departure))
  {
    PieNam const annaniser = get_walk_of_piece_on_square(annaniser_pos);
    generate_moves_for_piece(slices[si].next1,sq_departure,annaniser);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void annan_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STAnnanMovesForPieceGenerator);
  stip_instrument_is_square_observed_testing(si,nr_sides,STAnnanIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
