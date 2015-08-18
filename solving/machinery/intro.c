#include "solving/machinery/intro.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "solving/battle_play/attack_adapter.h"
#include "solving/battle_play/defense_adapter.h"
#include "solving/help_play/adapter.h"
#include "debugging/assert.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

/* Initialise a spin_off_state_type object
 */
static void spin_off_state_init(spin_off_state_type *state)
{
  slice_index i;
  for (i = 0; i!=max_nr_slices; ++i)
    state->spun_off[i] = no_slice;
}

static void link_to_intro(slice_index si, stip_structure_traversal *st)
{
  stip_traverse_structure_children(si,st);

  /* make sure that the entry slices into the intro have a correct .prev value */
  link_to_branch(si,SLICE_NEXT1(si));
}

void hack_fork_make_intro(slice_index fork, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(fork,st);

  st->level = structure_traversal_level_nested;
  stip_traverse_structure_conditional_pipe_tester(fork,st);
  st->level = structure_traversal_level_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor intro_slice_inserters[] =
{
  { STAttackAdapter,     &attack_adapter_make_intro   },
  { STDefenseAdapter,    &defense_adapter_make_intro  },
  { STHelpAdapter,       &help_adapter_make_intro     },
  { STTemporaryHackFork, &hack_fork_make_intro        }
};

enum
{
  nr_intro_slice_inserters = (sizeof intro_slice_inserters
                              / sizeof intro_slice_inserters[0])
};

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void solving_insert_intro_slices(slice_index si)
{
  spin_off_state_type state;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(SLICE_TYPE(si)==STStartOfSolvingMachinery);

  spin_off_state_init(&state);
  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &link_to_intro);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &link_to_intro);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &link_to_intro);
  stip_structure_traversal_override(&st,
                                    intro_slice_inserters,
                                    nr_intro_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
