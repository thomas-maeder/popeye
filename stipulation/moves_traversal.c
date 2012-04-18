#include "stipulation/moves_traversal.h"
#include "pystip.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/conditional_pipe.h"
#include "debugging/trace.h"

#include <assert.h>

static moves_visitor_map_type moves_children_traversers = { { 0 } };

static void stip_traverse_moves_pipe(slice_index si, stip_moves_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(next!=no_slice);
  stip_traverse_moves(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_branch(slice_index si, stip_moves_traversal *st)
{
  stip_length_type const save_remaining = st->remaining;
  stip_length_type const save_full_length = st->full_length;
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->context = stip_traversal_context_intro;

  stip_traverse_moves(si,st);

  st->context = save_context;
  st->full_length = save_full_length;
  st->remaining = save_remaining;
  TraceFunctionParam("->%u\n",st->remaining);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_attack_adapter(slice_index si,
                                               stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_intro)
  {
    assert(st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    assert(st->full_length==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    st->full_length = slices[si].u.branch.length-slack_length;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    st->context = stip_traversal_context_attack;

    stip_traverse_moves_pipe(si,st);

    st->context = stip_traversal_context_intro;
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_defense_adapter(slice_index si,
                                                stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_intro)
  {
    assert(st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    assert(st->full_length==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    st->full_length = slices[si].u.branch.length-slack_length;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    st->context = stip_traversal_context_defense;

    stip_traverse_moves_pipe(si,st);

    st->context = stip_traversal_context_intro;
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_ready_for_attack(slice_index si,
                                                 stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_attack);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_ready_for_defense(slice_index si,
                                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_defense);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_help_adapter(slice_index si,
                                             stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_intro)
  {
    assert(st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    assert(st->full_length==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    st->full_length = slices[si].u.branch.length-slack_length;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    st->context = stip_traversal_context_help;

    stip_traverse_moves_pipe(si,st);

    st->context = stip_traversal_context_intro;
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_setplay_fork(slice_index si,
                                             stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_pipe(si,st);
  stip_traverse_moves_branch(slices[si].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_traversal_context_type next_context(stip_traversal_context_type context)
{
  stip_traversal_context_type result;

  switch (context)
  {
    case stip_traversal_context_attack:
      result = stip_traversal_context_defense;
      break;

    case stip_traversal_context_defense:
      result = stip_traversal_context_attack;
      break;

    case stip_traversal_context_help:
      result = stip_traversal_context_help;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

static void stip_traverse_moves_move_played(slice_index si,
                                            stip_moves_traversal *st)
{
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->remaining>0);

  st->context = next_context(st->context);
  --st->remaining;
  TraceValue("->%u\n",st->remaining);
  stip_traverse_moves_pipe(si,st);
  ++st->remaining;
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_fork_on_remaining(slice_index si,
                                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=slices[si].u.fork_on_remaining.threshold)
    stip_traverse_moves(slices[si].u.fork_on_remaining.op2,st);
  else
    stip_traverse_moves(slices[si].u.fork_on_remaining.op1,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_dead_end(slice_index si,
                                         stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining>0)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_end_of_branch(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=1)
    stip_traverse_moves_branch(slices[si].u.fork.fork,st);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_binary(slice_index si, stip_moves_traversal *st)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.binary.op1!=no_slice)
    stip_traverse_moves(slices[si].u.binary.op1,st);
  if (slices[si].u.binary.op2!=no_slice)
    stip_traverse_moves(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_conditional_pipe(slice_index si,
                                                 stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_moves_pipe(si,st);
  stip_traverse_moves_branch(slices[si].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const special_moves_children_traversers[] =
{
   { STAttackAdapter,    &stip_traverse_moves_attack_adapter    },
   { STDefenseAdapter,   &stip_traverse_moves_defense_adapter   },
   { STReadyForAttack,   &stip_traverse_moves_ready_for_attack  },
   { STReadyForDefense,  &stip_traverse_moves_ready_for_defense },
   { STHelpAdapter,      &stip_traverse_moves_help_adapter      },
   { STSetplayFork,      &stip_traverse_moves_setplay_fork      },
   { STConstraintSolver, &stip_traverse_moves_setplay_fork      },
   { STMovePlayed,       &stip_traverse_moves_move_played       },
   { STHelpMovePlayed,   &stip_traverse_moves_move_played       },
   { STForkOnRemaining,  &stip_traverse_moves_fork_on_remaining },
   { STDeadEnd,          &stip_traverse_moves_dead_end          },
   { STDeadEndGoal,      &stip_traverse_moves_dead_end          }
};

enum { nr_special_moves_children_traversers = sizeof special_moves_children_traversers
                                              / sizeof special_moves_children_traversers[0] };

/* it's not so clear whether it is a good idea to traverse moves once slices
 * of the following types have been inserted ... */
static slice_type const dubiously_traversed_slice_types[] =
{
    STContinuationSolver,
    STThreatSolver,
    STThreatEnforcer,
    STNoShortVariations,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STTrivialEndFilter
};

enum { nr_dubiously_traversed_slice_types = sizeof dubiously_traversed_slice_types
                                            / sizeof dubiously_traversed_slice_types[0] };

/* Initialise a move traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st, void *param)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map = moves_children_traversers;

  for (i = 0; i!=nr_slice_types; ++i)
    st->map.visitors[i] = moves_children_traversers.visitors[i];

  for (unsigned int i = 0; i!=max_nr_slices; ++i)
    st->remaining_watermark[i] = 0;

  st->context = stip_traversal_context_intro;
  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->param = param;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the number of moves at the start of the traversal. Normally, this is
 * determined while traversing the stipulation. Only invoke
 * stip_moves_traversal_set_remaining() when the traversal is started in the
 * middle of a stipulation.
 * @param st to be initialised
 * @param remaining number of remaining moves (without slack)
 * @param full_length full number of moves of the initial branch (without slack)
 */
void stip_moves_traversal_set_remaining(stip_moves_traversal *st,
                                        stip_length_type remaining,
                                        stip_length_type full_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->remaining = remaining;
  st->full_length = full_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at some slice types
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 */
void stip_moves_traversal_override(stip_moves_traversal *st,
                                   moves_traversers_visitors const visitors[],
                                   unsigned int nr_visitors)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_structure(stip_moves_traversal *st,
                                                slice_structural_type type,
                                                stip_moves_visitor visitor)
{
  slice_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_structural_type(i)==type)
      st->map.visitors[i] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a functional type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_function(stip_moves_traversal *st,
                                               slice_functional_type type,
                                               stip_moves_visitor visitor)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_functional_type(i)==type)
      st->map.visitors[i] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          slice_type type,
                                          stip_moves_visitor visitor)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map.visitors[type] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of the stipulation
 * @param root start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->remaining);

  TraceEnumerator(slice_type,slices[root].type,"\n");
  assert(slices[root].type<=nr_slice_types);

  if (st->remaining_watermark[root]<=st->remaining)
  {
    stip_moves_visitor const operation = st->map.visitors[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
    st->remaining_watermark[root] = st->remaining+1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_noop(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    slice_type const type = slices[si].type;
    stip_moves_visitor const operation = moves_children_traversers.visitors[type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_moves_visitor get_default_children_moves_visitor(slice_type type)
{
  stip_moves_visitor result;

  switch (slice_type_get_structural_type(type))
  {
    case slice_structure_pipe:
    case slice_structure_branch:
      result = &stip_traverse_moves_pipe;
      break;

    case slice_structure_leaf:
      result = &stip_traverse_moves_noop;
      break;

    case slice_structure_binary:
      result = &stip_traverse_moves_binary;
      break;

    case slice_structure_fork:
      if (slice_type_get_functional_type(type)==slice_function_conditional_pipe)
        result = &stip_traverse_moves_conditional_pipe;
      else
        result = &stip_traverse_moves_end_of_branch;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Initialise stipulation traversal properties at start of program */
void init_moves_children_visitors(void)
{
  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      moves_children_traversers.visitors[i] = get_default_children_moves_visitor(i);
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_special_moves_children_traversers; ++i)
      moves_children_traversers.visitors[special_moves_children_traversers[i].type] = special_moves_children_traversers[i].visitor;
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_dubiously_traversed_slice_types; ++i)
      moves_children_traversers.visitors[dubiously_traversed_slice_types[i]] = &stip_traverse_moves_pipe;
  }
}
