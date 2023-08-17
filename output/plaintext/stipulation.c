#include "output/plaintext/stipulation.h"
#include "input/plaintext/goal.h"
#include "solving/pipe.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <string.h>

typedef struct
{
    FILE *file;
    unsigned int nr_chars_written;
    stip_length_type length;
    structure_traversal_level_type branch_level;
    Goal reci_goal;
} state_type;

static boolean is_pser(slice_index si, stip_structure_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = branch_find_slice(STIfThenElse,si,st->context)!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void write_attack(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  stip_length_type const save_length = state->length;
  structure_traversal_level_type const save_level = state->branch_level;

  state->branch_level = st->level;
  state->length = SLICE_U(si).branch.length;

  if (st->level==structure_traversal_level_top
      && state->length>slack_length+2
      && SLICE_U(si).branch.min_length==state->length-1)
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","exact-");

  if (is_pser(si,st))
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","pser-");

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_top)
    state->nr_chars_written += (unsigned int)fprintf(state->file,
                                                     "%u",
                                                     (SLICE_U(si).branch.length+1)/2);

  state->length = save_length;
  state->branch_level = save_level;
}

static void write_defense(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  stip_length_type const save_length = state->length;
  structure_traversal_level_type const save_level = state->branch_level;

  state->branch_level = st->level;
  state->length = SLICE_U(si).branch.length;

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_top)
    state->nr_chars_written += (unsigned int)fprintf(state->file,
                                                     "%u",
                                                     SLICE_U(si).branch.length/2+1);

  state->length = save_length;
  state->branch_level = save_level;
}

static boolean is_series(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_index const ready1 = branch_find_slice(STReadyForHelpMove,si,stip_traversal_context_help);
  assert(ready1!=no_slice);
  slice_index const ready2 = branch_find_slice(STReadyForHelpMove,ready1,stip_traversal_context_help);
  result = ready1==ready2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_intro_series(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const end = branch_find_slice(STEndOfBranch,si,stip_traversal_context_help);
    if (end!=no_slice)
    {
      slice_index const ready1 = branch_find_slice(STReadyForHelpMove,SLICE_NEXT2(end),stip_traversal_context_intro);
      if (ready1!=no_slice)
      {
        slice_index const ready2 = branch_find_slice(STReadyForHelpMove,ready1,stip_traversal_context_help);
        if (ready1==ready2)
          result = true;
        else
        {
          // TODO this is a hack to correctly deal with pser intro series
          result = branch_find_slice(STIfThenElse,ready2,stip_traversal_context_help)!=no_slice;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_help_play_implicit(slice_index si, stip_structure_traversal *st)
{
  boolean result = false;
  state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if(state->branch_level==structure_traversal_level_top)
  {
    slice_index const end_goal = branch_find_slice(STEndOfBranchGoal,
                                                   si,
                                                   st->context);
    if (end_goal!=no_slice)
    {
      slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                   SLICE_NEXT2(end_goal),
                                                   st->context);
      if (tester!=no_slice)
      {
        goal_type const type = SLICE_U(tester).goal_handler.goal.type;
        result = type==goal_proofgame || type==goal_atob;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_help_reci(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const end = branch_find_slice(STEndOfBranch,si,stip_traversal_context_help);
    if (end!=no_slice)
    {
      slice_index const and_slice = branch_find_slice(STAnd,SLICE_NEXT2(end),stip_traversal_context_intro);
      result = and_slice!=no_slice;
    }
    else
      result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void write_help(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->reci_goal.type!=no_goal)
    stip_traverse_structure_children(si,st);
  else
  {
    if (st->level==structure_traversal_level_top
        && state->length>slack_length+2
        && SLICE_U(si).branch.min_length>=state->length-1)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","exact-");

    if (is_help_reci(si))
    {
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s", "reci-");
      state->length += 2;
    }

    if (is_pser(si,st))
    {
      if (branch_find_slice(STEndOfBranchGoal,si,stip_traversal_context_help)!=no_slice
          || branch_find_slice(STEndOfBranchGoalImmobile,si,stip_traversal_context_help)!=no_slice)
      {
        state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","phser-");
        ++state->length;
      }
      else if (branch_find_slice(STEndOfBranchForced,si,stip_traversal_context_help)!=no_slice)
        state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","phser-");
      else
      {
        state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","pser-");
        ++state->length;
      }
    }
    else if (!is_help_play_implicit(si,st))
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s", "h");

    stip_traverse_structure_children(si,st);

    if (st->level==structure_traversal_level_top
        || is_pser(si,st)) // TODO this is a hack to correctly deal with pser stipulations
    {
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",state->length/2);
      if (state->length%2==1)
        state->nr_chars_written += (unsigned int)fprintf(state->file,"%s",".5");
    }
    else
    {
      /* h part of a ser-h - no need to write length */
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_series(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_top
      && state->length>slack_length+2
      && SLICE_U(si).branch.min_length>=state->length-1)
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","exact-");

  if (is_intro_series(si))
  {
    if (state->length%2==1)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",(state->length+1)/2);
    else if (state->length>1)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",state->length/2);
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","->");
    stip_traverse_structure_children(si,st);
  }
  else
  {
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s", "ser-");

    if (is_help_reci(si))
    {
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s", "reci-h");
      state->length += 2;
    }

    stip_traverse_structure_children(si,st);
    if (state->length%2==1)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",(state->length+1)/2);
    else if (state->length>1)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",state->length/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_help_adapter(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  stip_length_type const save_length = state->length;
  structure_traversal_level_type const save_level = state->branch_level;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->branch_level = st->level;
  state->length = SLICE_U(si).branch.length;

  if (is_series(si))
    write_series(si,st);
  else
    write_help(si,st);

  state->length = save_length;
  state->branch_level = save_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean skip_quodlibet_direct(slice_index si, stip_structure_traversal *st)
{
  slice_index const goal = branch_find_slice(STEndOfBranchGoal,si,st->context);
  slice_index const forced = branch_find_slice(STEndOfBranchForced,si,st->context);
  return (goal!=no_slice && goal!=si) || (forced!=no_slice && forced!=si);
}

static void write_end_goal(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (st->context==stip_traversal_context_defense
      && skip_quodlibet_direct(si,st))
    /* this is a quodlibet - write the other one */
    stip_traverse_structure_children_pipe(si,st);
  else
  {
    if (st->context==stip_traversal_context_attack)
    {
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","s");
      ++state->length;
    }

    stip_traverse_structure_next_branch(si,st);
  }
}

static void detect_constrained_attack(slice_index si, stip_structure_traversal *st)
{
  if (st->context==stip_traversal_context_attack)
  {
    boolean * const result = st->param;
    *result = true;
  }
  else
    stip_traverse_structure_children(si,st);
}

static boolean is_attack_constrained(slice_index si, stip_structure_traversal *st)
{
  boolean result = false;

  /* we needed a nested traversal because in semi-r#n option postkey,
   * STEndOfBranchForced precedes STConstraintsTester */
  stip_structure_traversal st_nested;
  stip_structure_traversal_init(&st_nested,&result);
  st_nested.context = st->context;
  stip_structure_traversal_override_by_structure(&st_nested,
                                                 slice_structure_fork,
                                                 &stip_traverse_structure_children);
  stip_structure_traversal_override_single(&st_nested,
                                           STConstraintTester,
                                           &detect_constrained_attack);
  stip_traverse_structure(si,&st_nested);

  return result;
}

static void write_end_forced(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (st->context==stip_traversal_context_defense)
  {
    if (!is_attack_constrained(si,st) && state->length>slack_length)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","semi-");
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","r");
  }
  else if (st->context==stip_traversal_context_help)
  {
    boolean const is_self = branch_find_slice(STDefenseAdapter,SLICE_NEXT2(si),stip_traversal_context_intro)!=no_slice;
    if (!is_self)
      state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","r");
    ++state->length;
  }

  stip_traverse_structure_children(si,st);
}

static unsigned int WriteSquare(FILE *file, square s)
{
  unsigned int result = 0;

  /* TODO avoid duplication with WriteSquare() */
  result += (unsigned int)fprintf(file,"%c",(int)getBoardFileLabel((s%onerow) - nr_files_on_board));
  result += (unsigned int)fprintf(file,"%c",(int)getBoardRowLabel((s/onerow) - nr_rows_on_board));

  return result;
}

static unsigned int WriteGoal(FILE *file, Goal goal)
{
  unsigned int result = 0;

  result += (unsigned int)fprintf(file,"%s",get_goal_symbol(goal.type));
  if (goal.type==goal_target || goal.type==goal_kiss)
    result += WriteSquare(file,goal.target);

  return result;
}

static void write_goal_reached(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  Goal const goal = SLICE_U(si).goal_handler.goal;

  if (state->reci_goal.type!=no_goal
      && (state->reci_goal.type!=goal.type
          || (goal.type==goal_target && state->reci_goal.target!=goal.target)))
  {
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","(");
    state->nr_chars_written += WriteGoal(state->file,state->reci_goal);
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s",")");
  }

  state->nr_chars_written += WriteGoal(state->file,goal);
}

static void write_reci_goal(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  slice_index end2 = branch_find_slice(STEndOfBranchGoal,SLICE_NEXT2(si),stip_traversal_context_intro);
  if (end2==no_slice)
    end2 = branch_find_slice(STEndOfBranchGoalImmobile,SLICE_NEXT2(si),stip_traversal_context_intro);
  assert(end2!=no_slice);

  {
    slice_index const goal2 = branch_find_slice(STGoalReachedTester,SLICE_NEXT2(end2),stip_traversal_context_help);
    assert(goal2!=no_slice);

    state->reci_goal = SLICE_U(goal2).goal_handler.goal;
    stip_traverse_structure_binary_operand1(si,st);
    state->reci_goal.type = no_goal;
  }
}

static void write_and(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (st->level==structure_traversal_level_top)
  {
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%s","reci-h");
    write_reci_goal(si,st);
    state->nr_chars_written += (unsigned int)fprintf(state->file,"%u",1u);
  }
  else
    write_reci_goal(si,st);
}

static structure_traversers_visitor const visitors[] = {
    { STAttackAdapter, &write_attack },
    { STDefenseAdapter, &write_defense },
    { STHelpAdapter, &write_help_adapter },
    { STConstraintTester, &stip_traverse_structure_children_pipe },
    { STEndOfBranchGoal, &write_end_goal },
    { STEndOfBranchForced, &write_end_forced },
    { STGoalReachedTester, &write_goal_reached },
    { STGoalConstraintTester, &stip_traverse_structure_children_pipe },
    { STAnd, &write_and },
    { STIfThenElse, &stip_structure_visitor_noop }
};
enum { nr_visitors = sizeof visitors / sizeof visitors[0] };

/* Write a traditional stipulation
 * @param file where to write to
 * @param si identiifes the entry slice into the stipulation
 * @return number of characters written
 */
unsigned int WriteStipulation(FILE *file, slice_index stipulation)
{
  state_type state = { file, 0, UINT_MAX, structure_traversal_level_top, { no_goal, initsquare } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation);
  TraceFunctionParamListEnd();

  TraceStipulation(stipulation);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override(&st,visitors,nr_visitors);
    stip_traverse_structure(stipulation,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.nr_chars_written);
  TraceFunctionResultEnd();
  return state.nr_chars_written;
}
