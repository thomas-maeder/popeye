#include "optimisations/observation.h"
#include "conditions/conditions.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

boolean dont_try_observing_with_non_existing_walk(slice_index si,
                                                  validator_id evaluate)
{
  return (number_of_pieces[trait[nbply]][observing_walk[nbply]]>0
          && is_square_observed_recursive(slices[si].next1,evaluate));
}

boolean dont_try_observing_with_non_existing_walk_both_sides(slice_index si,
                                                             validator_id evaluate)
{
  return (number_of_pieces[White][observing_walk[nbply]]+number_of_pieces[Black][observing_walk[nbply]]>0
          && is_square_observed_recursive(slices[si].next1,evaluate));
}

static slice_type const ortho_check_slice_types_non_proxy[] =
{
    STOr,
    STObserveWithKing,
    STObserveWithOrthoNonKing,
    STEnforceObserverWalk,
    STTrackBackFromTargetAccordingToObserverWalk
};

enum {
  nr_ortho_check_slice_types_non_proxy = sizeof ortho_check_slice_types_non_proxy / sizeof ortho_check_slice_types_non_proxy[0]
};

static boolean is_ortho_check_slice_type(slice_type type)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_ortho_check_slice_types_non_proxy; ++i)
    if (type==ortho_check_slice_types_non_proxy[i])
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void is_branch_remainder_ortho(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_type const type = slices[si].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_type_get_functional_type(type)==slice_function_proxy
      || is_ortho_check_slice_type(type))
    stip_traverse_structure_children(si,st);
  else
  {
    boolean * const result = st->param;
    *result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_branch_ortho(slice_index entry)
{
  boolean result = true;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",entry);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &is_branch_remainder_ortho);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &is_branch_remainder_ortho);
  stip_traverse_structure(entry,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_filter(slice_index si, stip_structure_traversal *st)
{
  Side const * const side = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].starter==*side)
  {
    if (CondFlag[facetoface] || CondFlag[backtoback] || CondFlag[cheektocheek])
      is_square_observed_insert_slice(si,STDontTryObservingWithNonExistingWalkBothSides);
    else
      is_square_observed_insert_slice(si,STDontTryObservingWithNonExistingWalk);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_side(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (is_observation_trivially_validated(side)
      && is_branch_ortho(slices[temporary_hack_is_square_observed[side]].next2))
    stip_instrument_is_square_observed_testing(si,side,STIsSquareObservedOrtho);
  else
  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,&side);
    stip_structure_traversal_override_single(&st,
                                             STDeterminedObserverWalk,
                                             &insert_filter);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise the square observation machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_square_observed(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  optimise_side(si,White);
  optimise_side(si,Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether observations are trivially validated (allowing for some
 * optimisations)
 * @param side for which side?
 */
boolean is_observation_trivially_validated(Side side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  result = (is_branch_ortho(slices[temporary_hack_check_validator[side]].next2)
            && is_branch_ortho(slices[temporary_hack_observation_validator[side]].next2)
            && is_branch_ortho(slices[temporary_hack_observation_geometry_validator[side]].next2)
            && is_branch_ortho(slices[temporary_hack_observer_validator[side]].next2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
