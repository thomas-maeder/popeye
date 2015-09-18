#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "optimisations/intelligent/intercept_check_by_black.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

static square const *being_blocked;
static unsigned int nr_being_blocked;

static void block_first(slice_index si);

/* Go on once all squares to be blocked have been blocked
 */
static void finalise_blocking(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_being_blocked>=1);

  if (nr_being_blocked==1)
    intelligent_stalemate_test_target_position(si);
  else
  {
    --nr_being_blocked;
    ++being_blocked;
    block_first(si);
    --being_blocked;
    ++nr_being_blocked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue by blocking the first remaining square
 */
static void block_first(slice_index si)
{
  unsigned int i;
  square const to_be_blocked = being_blocked[0];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(*where_to_start_placing_black_pieces<=to_be_blocked);

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      black[i].usage = piece_blocks;
      intelligent_place_black_piece(si,i,to_be_blocked,&finalise_blocking);
      black[i].usage = piece_is_unused;
    }

  occupy_square(to_be_blocked,Dummy,BIT(Black));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Place black blocks for mobile pieces (not the king)
 * @param to_be_blocked one ore more squares to be blocked
 * @param nr_to_be_blocked number of elements of to_be_blocked
 */
void intelligent_stalemate_black_block(slice_index si,
                                       square const to_be_blocked[8],
                                       unsigned int nr_to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParamListEnd();

  assert(nr_to_be_blocked>0);

  if (intelligent_reserve_masses(Black,nr_to_be_blocked,piece_blocks))
  {
    square const *save_being_blocked = being_blocked;
    unsigned int const save_nr_being_blocked = nr_being_blocked;
    unsigned int i;

    being_blocked = to_be_blocked;
    nr_being_blocked = nr_to_be_blocked;

    for (i = 0; i!=nr_to_be_blocked; ++i)
      occupy_square(to_be_blocked[i],Dummy,BIT(Black));

    block_first(si);
    intelligent_unreserve();

    for (i = 0; i!=nr_to_be_blocked; ++i)
      empty_square(to_be_blocked[i]);

    being_blocked = save_being_blocked;
    nr_being_blocked = save_nr_being_blocked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
