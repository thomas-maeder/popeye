#include "stipulation/structure_traversal.h"
#include "pystip.h"
#include "stipulation/testing_pipe.h"
#include "debugging/trace.h"

#include <assert.h>

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

  TraceEnumerator(slice_type,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    stip_structure_visitor const operation = (*ops)[slices[si].type];
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

  if (slices[pipe].u.pipe.next!=no_slice)
    stip_traverse_structure(slices[pipe].u.pipe.next,st);

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

  st->context = stip_traversal_context_intro;
  stip_traverse_structure(slices[branch_entry].u.fork.fork,st);
  st->context = save_context;

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

  st->level = structure_traversal_level_setplay;
  stip_traverse_structure_next_branch(si,st);
  st->level = structure_traversal_level_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_attack_adpater(slice_index si,
                                                            stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_intro);

  st->context = stip_traversal_context_attack;
  st->level = structure_traversal_level_nested;
  stip_traverse_structure_children_pipe(si,st);
  st->level = save_level;
  st->context = stip_traversal_context_intro;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_defense_adapter(slice_index si,
                                                             stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_intro);

  st->context = stip_traversal_context_defense;
  st->level = structure_traversal_level_nested;
  stip_traverse_structure_children_pipe(si,st);
  st->level = save_level;
  st->context = stip_traversal_context_intro;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_ready_for_attack(slice_index si,
                                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_attack);

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_ready_for_defense(slice_index si,
                                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_defense);

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_help_adpater(slice_index si,
                                                          stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* STHelpAdaper slices are part of the loop in the beginning,
   * i.e. we may already be in help context when we arrive here */
  assert(st->context==stip_traversal_context_intro
         || st->context==stip_traversal_context_help);

  st->context = stip_traversal_context_help;
  st->level = structure_traversal_level_nested;
  stip_traverse_structure_children_pipe(si,st);
  st->level = save_level;
  st->context = save_context;

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

static void stip_traverse_structure_children_move_played(slice_index si,
                                                         stip_structure_traversal *st)
{
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  st->context = next_context(st->context);
  stip_traverse_structure_children_pipe(si,st);
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stip_traverse_structure_children_binary(slice_index si,
                                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  stip_traverse_structure(slices[si].u.binary.op2,st);

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

  if (slices[si].u.fork.fork!=no_slice)
    stip_traverse_structure_next_branch(si,st);

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

  if (slices[testing_pipe].u.fork.fork!=no_slice)
    stip_traverse_structure(slices[testing_pipe].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const special_children_traversers[] =
{
   { STSetplayFork,     &stip_traverse_structure_children_setplay_fork      },
   { STAttackAdapter,   &stip_traverse_structure_children_attack_adpater    },
   { STDefenseAdapter,  &stip_traverse_structure_children_defense_adapter   },
   { STReadyForAttack,  &stip_traverse_structure_children_ready_for_attack  },
   { STReadyForDefense, &stip_traverse_structure_children_ready_for_defense },
   { STHelpAdapter,     &stip_traverse_structure_children_help_adpater      },
   { STMovePlayed,      &stip_traverse_structure_children_move_played       },
   { STThreatSolver,    &stip_traverse_structure_children_binary            }
};

enum { nr_special_children_traversers = sizeof special_children_traversers
                                        / sizeof special_children_traversers[0] };

static stip_structure_visitor structure_children_traversers[nr_slice_types];

static stip_structure_visitor get_default_children_structure_visitor(slice_type type)
{
  stip_structure_visitor result;

  switch (slice_type_get_structural_type(type))
  {
    case slice_structure_pipe:
    case slice_structure_branch:
      result = &stip_traverse_structure_children_pipe;
      break;

    case slice_structure_leaf:
      result = &stip_structure_visitor_noop;
      break;

    case slice_structure_binary:
      result = &stip_traverse_structure_children_binary;
      break;

    case slice_structure_fork:
      if (slice_type_get_functional_type(type)==slice_function_testing_pipe)
        result = &stip_traverse_structure_children_testing_pipe;
      else
        result = &stip_traverse_structure_children_fork;
      break;

    default:
      assert(0);
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

/* Override the behavior of a structure traversal at slices of a structural type
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
                                  structure_traversers_visitors const visitors[],
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

  {
    unsigned int i;
    for (i = 0; i!=nr_special_children_traversers; ++i)
      structure_children_traversers[special_children_traversers[i].type] = special_children_traversers[i].visitor;
  }
}
