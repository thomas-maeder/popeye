#include "optimisations/observation.h"
#include "conditions/conditions.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
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

static slice_type const ortho_validation_slice_types_non_proxy[] =
{
    STEnforceObserverSide,
    STEnforceObserverWalk
};

enum {
  nr_ortho_validation_slice_types_non_proxy = sizeof ortho_validation_slice_types_non_proxy / sizeof ortho_validation_slice_types_non_proxy[0]
};

typedef struct
{
    boolean non_ortho_seen;
    unsigned int ortho_seen[nr_ortho_validation_slice_types_non_proxy];
} ortho_validation_trival_state_type;

static unsigned int is_ortho_validation_slice_type(slice_type type)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_ortho_validation_slice_types_non_proxy; ++i)
    if (type==ortho_validation_slice_types_non_proxy[i])
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static void is_validation_branch_remainder_ortho(slice_index si,
                                                 stip_structure_traversal *st)
{
  ortho_validation_trival_state_type * const state = st->param;
  slice_type const type = slices[si].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_type_get_functional_type(type)==slice_function_proxy)
    stip_traverse_structure_children(si,st);
  else
  {
    unsigned int const pos = is_ortho_validation_slice_type(type);
    if (pos==nr_ortho_validation_slice_types_non_proxy)
      state->non_ortho_seen = true;
    else
    {
      ++state->ortho_seen[pos];
      stip_traverse_structure_children(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_validation_branch_ortho(slice_index entry)
{
  ortho_validation_trival_state_type state = { false };
  boolean all_ortho_seen = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",entry);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &is_validation_branch_remainder_ortho);
    stip_traverse_structure(entry,&st);
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_ortho_validation_slice_types_non_proxy; ++i)
      if (state.ortho_seen[i]==0)
        all_ortho_seen = false;
  }

  TraceValue("%u",state.non_ortho_seen);
  TraceValue("%u",all_ortho_seen);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",!state.non_ortho_seen && all_ortho_seen);
  TraceFunctionResultEnd();
  return !state.non_ortho_seen && all_ortho_seen;
}

static slice_type const ortho_observation_slice_types_non_proxy[] =
{
    STOr,
    STObserveWithOrtho,
    STTrackBackFromTargetAccordingToObserverWalk
};

enum {
  nr_ortho_observation_slice_types_non_proxy = sizeof ortho_observation_slice_types_non_proxy / sizeof ortho_observation_slice_types_non_proxy[0]
};

static boolean is_ortho_observation_slice_type(slice_type type)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_ortho_observation_slice_types_non_proxy; ++i)
    if (type==ortho_observation_slice_types_non_proxy[i])
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void is_observation_branch_remainder_ortho(slice_index si,
                                                  stip_structure_traversal *st)
{
  slice_type const type = slices[si].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_type_get_functional_type(type)==slice_function_proxy
      || is_ortho_observation_slice_type(type))
    stip_traverse_structure_children(si,st);
  else
  {
    boolean * const result = st->param;
    *result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_observation_branch_ortho(slice_index entry)
{
  boolean result = true;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",entry);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &is_observation_branch_remainder_ortho);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &is_observation_branch_remainder_ortho);
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
    if (CondFlag[whvault_king] || CondFlag[blvault_king])
      /* no optimisation */ ;
    else if (CondFlag[facetoface] || CondFlag[backtoback] || CondFlag[cheektocheek]
        || CondFlag[bicolores]
        || CondFlag[whtrans_king] || CondFlag[bltrans_king]
        || CondFlag[whsupertrans_king] || CondFlag[blsupertrans_king])
      is_square_observed_insert_slice(si,alloc_pipe(STDontTryObservingWithNonExistingWalkBothSides));
    else
      is_square_observed_insert_slice(si,alloc_pipe(STDontTryObservingWithNonExistingWalk));
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
      && is_observation_branch_ortho(slices[temporary_hack_is_square_observed[side]].next2))
    stip_instrument_is_square_observed_testing(si,side,STIsSquareObservedOrtho);
  else
  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,&side);
    stip_structure_traversal_override_single(&st,
                                             STOptimisingObserverWalk,
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

  result = (is_validation_branch_ortho(slices[temporary_hack_check_validator[side]].next2)
            && is_validation_branch_ortho(slices[temporary_hack_observation_validator[side]].next2)
            && is_validation_branch_ortho(slices[temporary_hack_observation_geometry_validator[side]].next2)
            && is_validation_branch_ortho(slices[temporary_hack_observer_validator[side]].next2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
