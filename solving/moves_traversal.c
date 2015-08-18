#include "solving/moves_traversal.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static moves_visitor_map_type moves_children_traversers = { { 0 } };

static void stip_traverse_moves_pipe(slice_index si, stip_moves_traversal *st)
{
  slice_index const next = SLICE_NEXT1(si);

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
    st->full_length = SLICE_U(si).branch.length-slack_length;
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
    st->full_length = SLICE_U(si).branch.length-slack_length;
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
    st->full_length = SLICE_U(si).branch.length-slack_length;
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
  stip_traverse_moves_branch(SLICE_NEXT2(si),st);

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
      result = stip_traversal_context_none;
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

  if (st->remaining<=SLICE_U(si).fork_on_remaining.threshold)
    stip_traverse_moves(SLICE_NEXT2(si),st);
  else
    stip_traverse_moves(SLICE_NEXT1(si),st);

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

  stip_traverse_moves_branch(SLICE_NEXT2(si),st);
  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse operand 1 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_binary_operand1(slice_index binary_slice,
                                         stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",binary_slice);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT1(binary_slice)!=no_slice)
    stip_traverse_moves(SLICE_NEXT1(binary_slice),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse operand 2 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_binary_operand2(slice_index binary_slice,
                                         stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",binary_slice);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT2(binary_slice)!=no_slice)
    stip_traverse_moves(SLICE_NEXT2(binary_slice),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_binary(slice_index si, stip_moves_traversal *st)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_binary_operand1(si,st);
  stip_traverse_moves_binary_operand2(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_testing_pipe_tester(slice_index testing_pipe,
                                                    stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParamListEnd();

  assert(slice_type_get_contextual_type(SLICE_TYPE(testing_pipe))
         ==slice_contextual_testing_pipe);

  if (SLICE_NEXT2(testing_pipe)!=no_slice)
  {
    stip_traversal_activity_type const save_activity = st->activity;
    st->activity = stip_traversal_activity_testing;
    stip_traverse_moves(SLICE_NEXT2(testing_pipe),st);
    st->activity = save_activity;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_testing_pipe(slice_index testing_pipe,
                                             stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParamListEnd();

  stip_traverse_moves_testing_pipe_tester(testing_pipe,st);
  stip_traverse_moves_pipe(testing_pipe,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_conditional_pipe_tester(slice_index conditional_pipe,
                                                        stip_moves_traversal *st)
{
  stip_traversal_activity_type const save_activity = st->activity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",conditional_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  assert(slice_type_get_contextual_type(SLICE_TYPE(conditional_pipe))
         ==slice_contextual_conditional_pipe);

  st->activity = stip_traversal_activity_testing;
  stip_traverse_moves_branch(SLICE_NEXT2(conditional_pipe),st);
  st->activity = save_activity;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_moves_conditional_pipe(slice_index conditional_pipe,
                                                 stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",conditional_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_moves_pipe(conditional_pipe,st);
  stip_traverse_moves_conditional_pipe_tester(conditional_pipe,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stop_recursion(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  st->remaining_watermark[si] = st->remaining+1;
  stip_traverse_moves_pipe(si,st);

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
   { STAttackPlayed,     &stip_traverse_moves_move_played       },
   { STDefensePlayed,    &stip_traverse_moves_move_played       },
   { STHelpMovePlayed,   &stip_traverse_moves_move_played       },
   { STForkOnRemaining,  &stip_traverse_moves_fork_on_remaining },
   { STDeadEnd,          &stip_traverse_moves_dead_end          },
   { STDeadEndGoal,      &stip_traverse_moves_dead_end          },
   { STRecursionStopper, &stop_recursion                        }
};

enum { nr_special_moves_children_traversers = sizeof special_moves_children_traversers
                                              / sizeof special_moves_children_traversers[0] };

/* it's not so clear whether it is a good idea to traverse moves once slices
 * of the following types have been inserted ... */
static slice_type const dubiously_traversed_slice_types[] =
{
    STContinuationSolver,
    STThreatEnforcer,
    STNoShortVariations,
    STMaxNrNonTrivial,
    STMaxThreatLength
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

  for (i = 0; i!=max_nr_slices; ++i)
    st->remaining_watermark[i] = 0;

  st->context = stip_traversal_context_intro;
  st->activity = stip_traversal_activity_solving;
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

/* Override the behavior of a moves traversal at slices of a contextual type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_contextual(stip_moves_traversal *st,
                                                 slice_contextual_type type,
                                                 stip_moves_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_contextual_type(i)==type)
      st->map.visitors[i] = visitor;
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

  TraceEnumerator(slice_type,SLICE_TYPE(root),"\n");
  assert(SLICE_TYPE(root)<=nr_slice_types);

  if (st->remaining_watermark[root]<=st->remaining)
  {
    stip_moves_visitor const operation = st->map.visitors[SLICE_TYPE(root)];
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

  TraceEnumerator(slice_type,SLICE_TYPE(si),"\n");
  assert(SLICE_TYPE(si)<=nr_slice_types);

  {
    slice_type const type = SLICE_TYPE(si);
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

    case slice_structure_fork:
      switch (slice_type_get_contextual_type(type))
      {
        case slice_contextual_testing_pipe:
          result = &stip_traverse_moves_testing_pipe;
          break;

        case slice_contextual_conditional_pipe:
          result = &stip_traverse_moves_conditional_pipe;
          break;

        case slice_contextual_binary:
          result = &stip_traverse_moves_binary;
          break;

        default:
          result = &stip_traverse_moves_end_of_branch;
          break;
      }
      break;

    default:
      assert(0);
      result = 0;
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
