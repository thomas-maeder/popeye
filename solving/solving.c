#include "solving/solving.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "pymsg.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/battle_play/threat.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/find_shortest.h"
#include "solving/battle_play/check_detector.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/continuation.h"
#include "trace.h"

#include <assert.h>

typedef enum
{
  solver_insertion_setplay,
  solver_insertion_root,
  solver_insertion_nested
} solver_insertion_level;

typedef enum
{
  solver_insertion_global,
  solver_insertion_attack,
  solver_insertion_defense,
  solver_insertion_help,
  solver_insertion_series
} solver_insertion_context;

typedef struct
{
  solver_insertion_level level;
  solver_insertion_context context;
  output_mode mode;
} solver_insertion_state;

static void remember_output_mode(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->mode==output_mode_none)
  {
    state->mode = slices[si].u.output_mode_selector.mode;
    stip_traverse_structure_children(si,st);
    state->mode = output_mode_none;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_setplay_fork(slice_index si,
                                        stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->level==solver_insertion_root);

  stip_traverse_structure_pipe(si,st);

  state->level = solver_insertion_setplay;
  stip_traverse_structure_next_branch(si,st);
  state->level = solver_insertion_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_nest(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  state->level = solver_insertion_nested;
  state->context = solver_insertion_global;
  stip_traverse_structure_next_branch(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_attack_adapter(slice_index si,
                                          stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->context = solver_insertion_attack;

  stip_traverse_structure_children(si,st);

  if (save_state.level==solver_insertion_root)
  {
    if (save_state.mode==output_mode_tree)
    {
      if (OptFlag[solvariantes])
      {
        if (!OptFlag[nothreat])
          stip_insert_threat_handlers(si);
      }
      else
      {
        slice_index const prototype = alloc_play_suppressor_slice();
        battle_branch_insert_slices(si,&prototype,1);
      }

      if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
      {
        branch_insert_try_solvers(si,get_max_nr_refutations());
        {
          slice_index const prototype = alloc_refutations_solver();
          battle_branch_insert_slices(si,&prototype,1);
        }
      }
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_attack(slice_index si,
                                  stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->level = solver_insertion_nested;

  stip_traverse_structure_children(si,st);

  if (save_state.level==solver_insertion_nested)
  {
    if (length>=min_length+2)
    {
      slice_index const proto = alloc_find_shortest_slice(length,min_length);
      battle_branch_insert_slices(si,&proto,1);
    }

    if (min_length>slack_length_battle+1)
    {
      slice_index const prototypes[] =
      {
        alloc_min_length_optimiser_slice(length,min_length),
        alloc_min_length_guard(length-1,min_length-1)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  if (length>slack_length_battle)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  if (state->mode==output_mode_tree
      && slices[si].u.branch.length>slack_length_battle)
  {
    slice_index const prototype = alloc_check_detector_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_defense_adapter(slice_index si,
                                           stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->level = solver_insertion_nested;
  state->context = solver_insertion_defense;

  stip_traverse_structure_children(si,st);

  if (save_state.level==solver_insertion_root)
  {
    if (save_state.mode==output_mode_tree)
    {
      if (!OptFlag[nothreat])
        stip_insert_threat_handlers(si);

      if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
        Message(TryPlayNotApplicable);
    }
    else
    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      if (!OptFlag[restart] && length>=min_length+2)
      {
        {
          slice_index const ready1 = branch_find_slice(STReadyForAttack,si);
          slice_index const ready2 = branch_find_slice(STReadyForAttack,ready1);
          slice_index const ready3 = branch_find_slice(STReadyForAttack,ready2);
          slice_index const shortcut_start = alloc_pipe(STShortSolutionsStart);
          assert(ready1!=no_slice);
          assert(ready2!=no_slice);
          assert(ready3!=no_slice);
          battle_branch_insert_slices(ready3,&shortcut_start,1);
        }
        {
          slice_index const dest = branch_find_slice(STShortSolutionsStart,si);
          slice_index const prototypes[] =
          {
            alloc_find_shortest_slice(length,min_length),
            alloc_fork_on_remaining_slice(dest,length-1-slack_length_battle)
          };
          enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
          assert(dest!=no_slice);
          battle_branch_insert_slices(si,prototypes,nr_prototypes);
        }
      }
    }
  }
  else
  {
    if (slices[si].u.branch.length>slack_length_battle)
    {
      {
        slice_index const prototype = alloc_continuation_solver_slice();
        battle_branch_insert_slices(si,&prototype,1);
      }
      if (save_state.level==solver_insertion_setplay)
      {
        unsigned int const max_nr_refutations = UINT_MAX;
        branch_insert_try_solvers(si,max_nr_refutations);
      }
    }
  }

  {
    slice_index const prototype = alloc_check_detector_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_defense(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (min_length>slack_length_battle+1)
  {
    slice_index const prototype = alloc_min_length_guard(length-1,min_length-1);
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_help(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->level = solver_insertion_nested;
  state->context = solver_insertion_help;

  if (save_state.level==solver_insertion_nested)
  {
    stip_traverse_structure_children(si,st);

    if (save_state.context==solver_insertion_global && length-min_length>=2)
    {
      slice_index const prototype = alloc_find_shortest_slice(length,min_length);
      help_branch_insert_slices(si,&prototype,1);
    }
  }
  else
  {
    stip_traverse_structure_children(si,st);

    if (length-min_length>=2 && length>=slack_length_help+2)
    {
      slice_index const ready1 = branch_find_slice(STReadyForHelpMove,si);
      slice_index const ready2 = branch_find_slice(STReadyForHelpMove,ready1);
      slice_index const ready3 = branch_find_slice(STReadyForHelpMove,ready2);
      slice_index const prototypes[] =
      {
        alloc_find_by_increasing_length_slice(length,min_length),
        alloc_fork_on_remaining_slice(ready3,length-1-slack_length_help)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_series(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  solver_insertion_state const save_state = *state;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->context = solver_insertion_series;
  state->level = solver_insertion_nested;

  if (save_state.level==solver_insertion_nested)
  {
    stip_traverse_structure_children(si,st);

    if (save_state.context==solver_insertion_global && length-min_length>=2)
    {
      slice_index const prototype = alloc_find_shortest_slice(length,min_length);
      series_branch_insert_slices(si,&prototype,1);
    }
  }
  else
  {
    stip_traverse_structure_children(si,st);

    if (length-min_length>=2)
    {
      slice_index const ready1 = branch_find_slice(STReadyForSeriesMove,si);
      slice_index const ready2 = branch_find_slice(STReadyForSeriesMove,ready1);
      slice_index const ready3 = branch_find_slice(STReadyForSeriesMove,ready2);
      slice_index const prototypes[] =
      {
        alloc_find_by_increasing_length_slice(length,min_length),
        alloc_fork_on_remaining_slice(ready3,length-1-slack_length_series)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      series_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const strategy_inserters[] =
{
  { STOutputModeSelector,      &remember_output_mode           },
  { STSetplayFork,             &insert_solvers_setplay_fork    },
  { STAttackAdapter,           &insert_solvers_attack_adapter  },
  { STReadyForAttack,          &insert_solvers_attack          },
  { STDefenseAdapter,          &insert_solvers_defense_adapter },
  { STReadyForDefense,         &insert_solvers_defense         },
  { STHelpAdapter,             &insert_solvers_help            },
  { STSeriesAdapter,           &insert_solvers_series          },
  { STConstraint,              &insert_solvers_nest            },
  { STEndOfBranch,             &insert_solvers_nest            },
  { STEndOfBranchForced,       &insert_solvers_nest            },
  { STEndOfBranchGoal,         &insert_solvers_nest            },
  { STEndOfBranchGoalImmobile, &insert_solvers_nest            }
};

enum
{
  nr_strategy_inserters = (sizeof strategy_inserters
                           / sizeof strategy_inserters[0])
};

/* Instrument the stipulation structure with solving slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_solvers(slice_index root_slice)
{
  stip_structure_traversal st;
  solver_insertion_state state =
  {
    solver_insertion_root,
    solver_insertion_global,
    output_mode_none
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    strategy_inserters,
                                    nr_strategy_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
