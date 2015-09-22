#include "stipulation/structure_traversal.h"
#include "stipulation/testing_pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Dispatch a slice structure operation to a slice based on its type
 * @param si identifies slice
 * @param ops contains addresses of visitors per slice type
 * @param st address of structure defining traversal
 */
static
void stip_structure_visit_slice(slice_index si,
                                stip_structure_visitor (*ops)[nr_slice_types],
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,SLICE_TYPE(si),"\n");
  assert(SLICE_TYPE(si)<=nr_slice_types);

  {
    stip_structure_visitor const operation = (*ops)[SLICE_TYPE(si)];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Query the traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",st->traversed[si]);
  TraceFunctionResultEnd();
  return st->traversed[si];
}

/* (Approximately) depth-first traversal of the stipulation
 * @param root entry slice into stipulation
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->context);

  if (root!=no_slice)
    if (st->traversed[root]==slice_not_traversed)
    {
      /* avoid infinite recursion */
      st->traversed[root] = slice_being_traversed;
      stip_structure_visit_slice(root,&st->map.visitors,st);
      st->traversed[root] = slice_traversed;
    }

  TraceValue("%u\n",st->context);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_traversal_context_type context_after_move(stip_traversal_context_type context)
{
  stip_traversal_context_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children_pipe(slice_index pipe,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT1(pipe)!=no_slice)
    switch (SLICE_TYPE(pipe))
    {
      case STAttackAdapter:
      {
        structure_traversal_level_type const save_level = st->level;
        assert(st->context==stip_traversal_context_intro);
        st->context = stip_traversal_context_attack;
        st->level = structure_traversal_level_nested;
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->level = save_level;
        st->context = stip_traversal_context_intro;
        break;
      }

      case STDefenseAdapter:
      {
        structure_traversal_level_type const save_level = st->level;
        /* STDefenseAdapter slices are part of the loop in the beginning,
         * i.e. we may already be in defense context when we arrive here */
        assert(st->context==stip_traversal_context_intro
               || st->context==stip_traversal_context_defense);
        st->context = stip_traversal_context_defense;
        st->level = structure_traversal_level_nested;
        TraceText("STDefenseAdapter -> next1\n");
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->level = save_level;
        st->context = stip_traversal_context_intro;
        break;
      }

      case STHelpAdapter:
      {
        structure_traversal_level_type const save_level = st->level;
        stip_traversal_context_type const save_context = st->context;
        /* STHelpAdaper slices are part of the loop in the beginning,
         * i.e. we may already be in help context when we arrive here */
        assert(st->context==stip_traversal_context_intro
               || st->context==stip_traversal_context_help);
        st->context = stip_traversal_context_help;
        st->level = structure_traversal_level_nested;
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->level = save_level;
        st->context = save_context;
        break;
      }

      case STReadyForAttack:
        assert(st->context==stip_traversal_context_attack);
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        break;

      case STReadyForDefense:
        assert(st->context==stip_traversal_context_defense);
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        break;

      case STReadyForHelpMove:
        assert(st->context==stip_traversal_context_help);
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        break;

      case STAttackPlayed:
      case STDefensePlayed:
      {
        stip_traversal_context_type const save_context = st->context;
        st->context = context_after_move(st->context);
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->context = save_context;
        break;
      }

      case STGeneratingMovesForPiece:
      {
        stip_traversal_context_type const save_context = st->context;
        assert(st->context==stip_traversal_context_intro);
        st->context = stip_traversal_context_move_generation;
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->context = save_context;
        break;
      }

      case STTestingIfSquareIsObserved:
      {
        stip_traversal_context_type const save_context = st->context;
        assert(st->context==stip_traversal_context_intro);
        st->context = stip_traversal_context_test_square_observation;
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        st->context = save_context;
        break;
      }

      default:
        stip_traverse_structure(SLICE_NEXT1(pipe),st);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue a traversal at the start of a branch; this function is typically
 * invoked by an end of branch slice
 * @param branch_entry entry slice into branch
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure_next_branch(slice_index branch_entry,
                                         stip_structure_traversal *st)
{
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch_entry);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT2(branch_entry)!=no_slice)
  {
    st->context = stip_traversal_context_intro;
    stip_traverse_structure(SLICE_NEXT2(branch_entry),st);
    st->context = save_context;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse the setplay side of a set play fork slice
 * @param si identifies the slice
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children_setplay_fork_setplay(slice_index si,
                                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->level==structure_traversal_level_top);

  st->level = structure_traversal_level_setplay;
  stip_traverse_structure_next_branch(si,st);
  st->level = structure_traversal_level_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_setplay_fork(slice_index si,
                                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->level==structure_traversal_level_top);

  stip_traverse_structure_children_pipe(si,st);
  stip_traverse_structure_children_setplay_fork_setplay(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse operand 1 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_binary_operand1(slice_index binary_slice,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",binary_slice);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure(SLICE_NEXT1(binary_slice),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse operand 2 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_binary_operand2(slice_index binary_slice,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",binary_slice);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure(SLICE_NEXT2(binary_slice),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void stip_traverse_structure_children_binary(slice_index binary_slice,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",binary_slice);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_binary_operand1(binary_slice,st);
  stip_traverse_structure_binary_operand2(binary_slice,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_zigzag_jump(slice_index jump,
                                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",jump);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_binary(jump,st);
  stip_traverse_structure(SLICE_U(jump).if_then_else.condition,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_fork(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (SLICE_NEXT2(si)!=no_slice)
    stip_traverse_structure_next_branch(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse the next branch from an end of branch slice
 * @param end_of_branch identifies the testing pipe
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_end_of_branch_next_branch(slice_index end_of_branch,
                                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",end_of_branch);
  TraceFunctionParamListEnd();

  assert(slice_type_get_contextual_type(SLICE_TYPE(end_of_branch))
         ==slice_contextual_end_of_branch);
  if (SLICE_NEXT2(end_of_branch)!=no_slice)
    stip_traverse_structure_next_branch(end_of_branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_end_of_branch(slice_index si,
                                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  stip_traverse_structure_end_of_branch_next_branch(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse the tester of a conditional pipe
 * @param testing_pipe identifies the testing pipe
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_conditional_pipe_tester(slice_index conditional_pipe,
                                                     stip_structure_traversal *st)
{
  stip_traversal_activity_type const save_activity = st->activity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",conditional_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  assert(slice_type_get_contextual_type(SLICE_TYPE(conditional_pipe))
         ==slice_contextual_conditional_pipe);

  st->activity = stip_traversal_activity_testing;
  stip_traverse_structure_next_branch(conditional_pipe,st);
  st->activity = save_activity;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_conditional_pipe(slice_index si,
                                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  stip_traverse_structure_conditional_pipe_tester(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse the tester of a testing pipe
 * @param testing_pipe identifies the testing pipe
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_testing_pipe_tester(slice_index testing_pipe,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  assert(slice_type_get_contextual_type(SLICE_TYPE(testing_pipe))
         ==slice_contextual_testing_pipe);

  if (SLICE_NEXT2(testing_pipe)!=no_slice)
  {
    stip_traversal_activity_type const save_activity = st->activity;
    st->activity = stip_traversal_activity_testing;
    stip_traverse_structure(SLICE_NEXT2(testing_pipe),st);
    st->activity = save_activity;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_testing_pipe(slice_index testing_pipe,
                                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(testing_pipe,st);
  stip_traverse_structure_testing_pipe_tester(testing_pipe,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor structure_children_traversers[nr_slice_types];

static stip_structure_visitor get_default_children_structure_visitor(slice_type type)
{
  stip_structure_visitor result;

  switch (type)
  {
    case STIfThenElse:
      result = &stip_traverse_structure_children_zigzag_jump;
      break;

    default:
      switch (slice_type_get_structural_type(type))
      {
        case slice_structure_pipe:
        case slice_structure_branch:
          result = &stip_traverse_structure_children_pipe;
          break;

        case slice_structure_leaf:
          result = &stip_structure_visitor_noop;
          break;

        case slice_structure_fork:
          switch (slice_type_get_contextual_type(type))
          {
            case slice_contextual_testing_pipe:
              result = &stip_traverse_structure_children_testing_pipe;
              break;

            case slice_contextual_conditional_pipe:
              result = &stip_traverse_structure_children_conditional_pipe;
              break;

            case slice_contextual_binary:
              result = &stip_traverse_structure_children_binary;
              break;

            case slice_contextual_end_of_branch:
              result = &stip_traverse_structure_children_end_of_branch;
              break;

            default:
              result = &stip_traverse_structure_children_fork;
              break;
          }
          break;

        default:
          assert(0);
          result = 0;
          break;
      }
      break;
  }

  return result;
}

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st, void *param)
{
  {
    unsigned int i;
    for (i = 0; i!=max_nr_slices; ++i)
      st->traversed[i] = slice_not_traversed;
  }

  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      st->map.visitors[i] = structure_children_traversers[i];
  }

  st->level = structure_traversal_level_top;
  st->context = stip_traversal_context_intro;
  st->activity = stip_traversal_activity_solving;

  st->param = param;
}

/* Initialise a nested structure traversal structure with default visitors, but
 * the level and context of a parent traversal
 * @param st to be initialised
 * @param parent parent traversal
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init_nested(stip_structure_traversal *st,
                                          stip_structure_traversal const *parent,
                                          void *param)
{
  {
    unsigned int i;
    for (i = 0; i!=max_nr_slices; ++i)
      st->traversed[i] = slice_not_traversed;
  }

  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      st->map.visitors[i] = structure_children_traversers[i];
  }

  st->level = parent->level;
  st->context = parent->context;
  st->activity = parent->activity;

  st->param = param;
}

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor (note: subclasses of type
 *             are not affected by
 *             stip_structure_traversal_override_by_structure()! )
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_structure(stip_structure_traversal *st,
                                                    slice_structural_type type,
                                                    stip_structure_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_structural_type(i)==type)
      st->map.visitors[i] = visitor;
}

/* Override the behavior of a structure traversal at slices of a functional type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_function(stip_structure_traversal *st,
                                                   slice_functional_type type,
                                                   stip_structure_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_functional_type(i)==type)
      st->map.visitors[i] = visitor;
}

/* Override the behavior of a structure traversal at slices of a contextual type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_contextual(stip_structure_traversal *st,
                                                    slice_contextual_type type,
                                                    stip_structure_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (slice_type_get_contextual_type(i)==type)
      st->map.visitors[i] = visitor;
}

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_single(stip_structure_traversal *st,
                                              slice_type type,
                                              stip_structure_visitor visitor)
{
  st->map.visitors[type] = visitor;
}

/* Override some of the visitors of a traversal
 * @param st to be initialised
 * @param visitors overriding visitors
 * @param nr_visitors number of visitors
 */
void
stip_structure_traversal_override(stip_structure_traversal *st,
                                  structure_traversers_visitor const visitors[],
                                  unsigned int nr_visitors)
{
  unsigned int i;
  for (i = 0; i!=nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_visit_slice(si,&structure_children_traversers,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise stipulation traversal properties at start of program */
void init_structure_children_visitors(void)
{
  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      structure_children_traversers[i] = get_default_children_structure_visitor(i);
  }

  structure_children_traversers[STSetplayFork] = &stip_traverse_structure_children_setplay_fork;
}
