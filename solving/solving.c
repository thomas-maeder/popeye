#include "solving/solving.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "pymsg.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/find_shortest.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/check_detector.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/continuation.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "trace.h"

#include <assert.h>

static void remember_output_mode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*mode==output_mode_none)
  {
    *mode = slices[si].u.output_mode_selector.mode;
    stip_traverse_structure_children(si,st);
    *mode = output_mode_none;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battle_insert_find_shortest(slice_index si)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>=min_length+2)
  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si);
    slice_index const attack = branch_find_slice(STReadyForAttack,defense);
    slice_index const proto = alloc_find_shortest_slice(length,min_length);
    assert(defense!=no_slice);
    assert(attack!=no_slice);
    battle_branch_insert_slices(attack,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battle_insert_min_length_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si);
    if (defense!=no_slice)
    {
      stip_length_type const length = slices[defense].u.branch.length;
      stip_length_type const min_length = slices[defense].u.branch.min_length;

      if (min_length>slack_length_battle+1)
      {
        slice_index const prototype = alloc_min_length_guard(length-1,min_length-1);
        battle_branch_insert_slices(defense,&prototype,1);

        if (min_length>slack_length_battle+2)
        {
          slice_index const prototypes[] =
          {
            alloc_min_length_optimiser_slice(length-1,min_length-1),
            alloc_min_length_guard(length-2,min_length-2)
          };
          enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
          slice_index const attack = branch_find_slice(STReadyForAttack,defense);
          assert(attack!=no_slice);
          battle_branch_insert_slices(attack,prototypes,nr_prototypes);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_attack_adapter(slice_index si,
                                          stip_structure_traversal *st)
{
  output_mode const * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_root)
  {
    if (*mode==output_mode_tree)
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

  battle_insert_find_shortest(si);
  battle_insert_min_length_handlers(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_defense_adapter(slice_index si,
                                           stip_structure_traversal *st)
{
  output_mode const * const mode = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_root)
  {
    if (*mode==output_mode_tree)
    {
      if (!OptFlag[nothreat])
        stip_insert_threat_handlers(si);

      if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
        Message(TryPlayNotApplicable);
    }
  }
  else
  {
    if (length>slack_length_battle)
    {
      {
        slice_index const prototype = alloc_continuation_solver_slice();
        battle_branch_insert_slices(si,&prototype,1);
      }
      if (st->level==structure_traversal_level_setplay)
      {
        unsigned int const max_nr_refutations = UINT_MAX;
        branch_insert_try_solvers(si,max_nr_refutations);
      }
    }
  }

  battle_insert_find_shortest(si);
  battle_insert_min_length_handlers(si);

  {
    slice_index const prototype = alloc_check_detector_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solvers_attack(slice_index si,
                                  stip_structure_traversal *st)
{
  output_mode const * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.branch.length>slack_length_battle)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    battle_branch_insert_slices(si,&prototype,1);

    if (*mode==output_mode_tree)
    {
      slice_index const prototype = alloc_check_detector_slice();
      battle_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_ready_for_move_in_loop(slice_index ready_root)
{
  slice_index result = ready_root;
  do
  {
    result = branch_find_slice(STReadyForHelpMove,result);
  } while ((slices[result].u.branch.length-slack_length_help)%2
           !=(slices[ready_root].u.branch.length-slack_length_help)%2);
  return result;
}

static void insert_solvers_help_adapter(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (length-min_length>=2)
  {
    if (st->level==structure_traversal_level_nested)
    {
      if (st->context==stip_traversal_context_global)
      {
        slice_index const prototype =
            alloc_find_shortest_slice(length,min_length);
        help_branch_insert_slices(si,&prototype,1);
      }
    }
    else /* root or set play */
    {
      if (length>=slack_length_help+2)
      {
        {
          slice_index const prototype =
              alloc_find_by_increasing_length_slice(length,min_length);
          help_branch_insert_slices(si,&prototype,1);
        }
        {
          slice_index const ready_root = branch_find_slice(STReadyForHelpMove,si);
          slice_index const ready_loop = find_ready_for_move_in_loop(ready_root);
          slice_index const proxy_root = alloc_proxy_slice();
          slice_index const proxy_loop = alloc_proxy_slice();
          pipe_set_successor(proxy_loop,ready_loop);
          pipe_link(slices[ready_root].prev,
                    alloc_fork_on_remaining_slice(proxy_root,proxy_loop,
                                                  length-1-slack_length_help));
          pipe_link(proxy_root,ready_root);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_move_generator_slice();
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void insert_single_move_generator_with_king_capture(slice_index si,
                                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const proto = alloc_single_move_generator_with_king_capture_slice();
    branch_insert_slices(slices[si].u.fork.fork,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const strategy_inserters[] =
{
  { STOutputModeSelector, &remember_output_mode           },
  { STAttackAdapter,      &insert_solvers_attack_adapter  },
  { STDefenseAdapter,     &insert_solvers_defense_adapter },
  { STHelpAdapter,        &insert_solvers_help_adapter    },
  { STReadyForAttack,     &insert_solvers_attack          },
  { STGeneratingMoves,    &insert_move_generator          },
  { STBrunnerDefenderFinder, &insert_single_move_generator_with_king_capture }
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
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override(&st,
                                    strategy_inserters,
                                    nr_strategy_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
