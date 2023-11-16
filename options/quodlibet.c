#include "options/quodlibet.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/modifier.h"
#include "solving/pipe.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef struct
{
  slice_index to_goal;
  boolean has_attack_ended;
} quodlibet_transformation_state;

static void remember_end_of_attack(slice_index si, stip_structure_traversal *st)
{
  quodlibet_transformation_state * const state = st->param;
  boolean const save_has_attack_ended = state->has_attack_ended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->has_attack_ended = true;
  stip_traverse_structure_children_pipe(si,st);
  state->has_attack_ended = save_has_attack_ended;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_end_of_branch(slice_index si,
                                                stip_structure_traversal *st)
{
  quodlibet_transformation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->has_attack_ended)
    state->to_goal = stip_deep_copy(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_direct_guards(slice_index si,
                                 stip_structure_traversal *st)
{
  quodlibet_transformation_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->level==structure_traversal_level_top
      && SLICE_U(si).branch.length>slack_length
      && state->to_goal!=no_slice)
    battle_branch_insert_direct_end_of_branch_goal(si,state->to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor transformers[] =
{
  { STAttackAdapter,   &insert_direct_guards                 },
  { STReadyForDefense, &remember_end_of_attack               },
  { STEndOfBranchGoal, &transform_to_quodlibet_end_of_branch }
};

enum { nr_transformers = sizeof transformers / sizeof transformers[0] };

/* Transform a stipulation tree to a quodlibet.
 * @param si identifies slice where to start
 * @return true iff the stipulation could be transformed
 */
static boolean transform(slice_index si)
{
  stip_structure_traversal st;
  quodlibet_transformation_state state = { no_slice, false };
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,transformers,nr_transformers);
  stip_traverse_structure(si,&st);

  result = state.to_goal!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Transform a stipulation tree to a quodlibet.
 * @param si identifies slice where to start
 */
void quodlibet_stipulation_modifier_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const stipulation_root_hook = stipulation_modifier_to_be_modified(si);
    if (stipulation_root_hook!=no_slice)
    {
      if (!transform(stipulation_root_hook))
      {
        output_plaintext_message(QuodlibetNotApplicable);
        output_plaintext_verifie_message(NewLine);
      }
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}
