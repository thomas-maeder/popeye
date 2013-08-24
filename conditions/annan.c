#include "conditions/annan.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

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

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void annan_generate_moves_for_piece(slice_index si, PieNam p)
{
  int const annaniser_dir = trait[nbply]==White ? -onerow : +onerow;
  square const annaniser_pos = curr_generation->departure+annaniser_dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (annanises(trait[nbply],annaniser_pos,curr_generation->departure))
  {
    PieNam const annaniser = get_walk_of_piece_on_square(annaniser_pos);
    generate_moves_for_piece(slices[si].next1,annaniser);
  }
  else
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - annanised or originally
 * @return true iff the observation is valid
 */
boolean annan_enforce_observer_walk(slice_index si)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  Side const side_attacking = trait[nbply];
  numvec const dir_annaniser = side_attacking==White ? dir_down : dir_up;
  square const pos_annaniser = sq_departure+dir_annaniser;
  PieNam walk;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (annanises(side_attacking,pos_annaniser,sq_departure))
    walk = get_walk_of_piece_on_square(pos_annaniser);
  else
    walk = get_walk_of_piece_on_square(sq_departure);

  if (walk==observing_walk[nbply])
    result = validate_observation_recursive(slices[si].next1);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_enforce_annanised_walk(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_substitute(si,alloc_pipe(STAnnanEnforceObserverWalk));

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

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STEnforceObserverWalk,
                                             &substitute_enforce_annanised_walk);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
