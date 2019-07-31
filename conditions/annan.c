#include "conditions/annan.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

ConditionLetteredVariantType annan_type;

static boolean annanises(Side side, square rear, square front)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(rear);
  TraceSquare(front);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[rear],side))
  {
    Flags const mask = BIT(side)|BIT(Royal);

    switch(annan_type)
    {
      case ConditionTypeA:
        result = true;
        break;

      case ConditionTypeB:
        result = !TSTFULLFLAGMASK(being_solved.spec[rear],mask);
        break;

      case ConditionTypeC:
        result = !TSTFULLFLAGMASK(being_solved.spec[front],mask);
        break;

      case ConditionTypeD:
        result = !TSTFULLFLAGMASK(being_solved.spec[rear],mask) && !TSTFULLFLAGMASK(being_solved.spec[front],mask);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void annan_generate_moves_for_piece(slice_index si)
{
  int const annaniser_dir = trait[nbply]==White ? -onerow : +onerow;
  square const annaniser_pos = curr_generation->departure+annaniser_dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (annanises(trait[nbply],annaniser_pos,curr_generation->departure))
    pipe_move_generation_different_walk_delegate(si,get_walk_of_piece_on_square(annaniser_pos));
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - annanised or originally
 * @return true iff the observation is valid
 */
boolean annan_enforce_observer_walk(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  Side const side_attacking = trait[nbply];
  numvec const dir_annaniser = side_attacking==White ? dir_down : dir_up;
  square const pos_annaniser = sq_departure+dir_annaniser;
  piece_walk_type walk;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (annanises(side_attacking,pos_annaniser,sq_departure))
    walk = get_walk_of_piece_on_square(pos_annaniser);
  else
    walk = get_walk_of_piece_on_square(sq_departure);

  if (walk==observing_walk[nbply])
    result = pipe_validate_observation_recursive_delegate(si);
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

  stip_traverse_structure_children(si,st);
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
