#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyint.h"
#include "pyslice.h"
#include "py1.h"
#include "trace.h"

#include <assert.h>

typedef enum
{
  output_mode_tree, /* typical in direct/self/reflex play */
  output_mode_line, /* typical in help/series play */

  output_mode_none
} output_mode;

static output_mode current_mode = output_mode_none;

static boolean does_ply_invert_colors[maxply];

slice_index active_slice[maxply];

static stip_length_type move_depth;


/* Initialize based on the stipulation
 */
void init_output(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  TraceValue("%u\n",slices[root_slice].type);
  switch (slices[root_slice].type)
  {
    case STBranchDirect:
      current_mode = output_mode_tree;
      break;

    case STBranchHelp:
    case STBranchSeries:
      current_mode = output_mode_line;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

typedef enum
{
  unknown_attack,
  continuation_attack,
  threat_attack,
  unsolvability_attack /* forced reflex mate by attacker */
} output_attack_type;

static output_attack_type output_attack_types[maxply];
static unsigned int nr_continuations_written[maxply];
static unsigned int nr_defenses_written[maxply];
static numecoup nbcou_of_last_attack_written[maxply];

/* Start a new output level consisting of forced reflex mates etc.
 */
void output_start_unsolvability_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
    output_attack_types[move_depth+1] = unsolvability_attack;

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of post-key only play)
 */
void output_end_unsolvability_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of post-key only play
 */
void output_start_postkeyonly_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    move_depth++;
    nr_continuations_written[move_depth] = 0;
    nr_defenses_written[move_depth] = 0;
  
    TraceValue("%u\n",move_depth);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of post-key only play)
 */
void output_end_postkeyonly_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    Message(NewLine);
    move_depth--;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of set play
 */
void output_start_move_inverted_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    move_depth++;
    nr_continuations_written[move_depth+1] = 1; /* prevent initial newline */
    nr_defenses_written[move_depth] = 0;
  }
  else
    does_ply_invert_colors[nbply+1] = true;

  TraceValue("%u\n",move_depth);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of set play)
 */
void output_end_move_inverted_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
    move_depth--;
  else
    does_ply_invert_colors[nbply+1] = false;

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of post-key play
 */
void output_start_postkey_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    nr_defenses_written[move_depth] = 0;
    nr_continuations_written[move_depth+1] = 0;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of post-key play)
 */
void output_end_postkey_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree
      && nr_defenses_written[move_depth]==0
      && nr_continuations_written[move_depth+1]==0)
    write_end_of_solution();

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of threats
 */
void output_start_threat_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    move_depth++;
    nr_continuations_written[move_depth] = 0;
    nr_defenses_written[move_depth] = 0;
    nr_continuations_written[move_depth+1] = 0;

    /* nbply will be increased by genmove() in a moment */
    output_attack_types[nbply+1] = threat_attack;
  }

  TraceValue("%u",move_depth);
  TraceValue("%u",nbply);
  TraceValue("%u\n",output_attack_types[nbply+1]);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of threats)
 */
void output_end_threat_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    if (nr_continuations_written[move_depth]==0)
    {
      Message(Zugzwang);
      ++nr_continuations_written[move_depth];
    }
  
    TraceValue("%u",nbply);
    TraceValue("%u\n",output_attack_types[nbply+1]);

    assert(output_attack_types[nbply+1]==threat_attack);
    output_attack_types[nbply+1] = unknown_attack;

    move_depth--;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of regular continuations
 */
void output_start_continuation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    move_depth++;

    nr_continuations_written[move_depth] = 0;
    nr_continuations_written[move_depth+1] = 0;
    nr_defenses_written[move_depth] = 0;

    nbcou_of_last_attack_written[move_depth] = UINT_MAX;

    /* nbply will be increased by genmove() in a moment */
    output_attack_types[nbply+1] = continuation_attack;

    TraceValue("%u",move_depth);
    TraceValue("%u",nbply);
    TraceValue("%u\n",output_attack_types[nbply+1]);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of regular
 * continuations)
 */
void output_end_continuation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    if (move_depth==2 && nr_continuations_written[move_depth]==0)
      write_refutation_mark();

    move_depth--;

    TraceValue("%u",nbply);
    TraceValue("%u\n",output_attack_types[nbply+1]);

    output_attack_types[nbply+1] = unknown_attack;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of leaf variations
 */
void output_start_leaf_variation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
    Message(NewLine);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of leaf variations)
 */
void output_end_leaf_variation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  TraceFunctionExit(__func__);
  TraceText("\n");
}

static void linesolution(void)
{
  int next_movenumber;
  Side starting_side;
  Goal end_marker;
  slice_index slice;
  ply current_ply;

  ply const start_ply = 2;
      
  TraceFunctionEntry(__func__);
  TraceText("\n");

#if !defined(DATABASE)
  if (isIntelligentModeActive)
  {
    if (SolAlreadyFound())
      return;
    else
    {
      if (OptFlag[maxsols])
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }
    StoreSol();
  }
  else
  {
    if (OptFlag[maxsols])
      solutions++;
    if (OptFlag[beep])
      BeepOnSolution(maxbeep);
  }
#endif

  slice = active_slice[start_ply];
  starting_side = regular_starter;

  ResetPosition();

  TraceValue("%u",regular_starter);
  TraceValue("%u\n",does_ply_invert_colors[start_ply]);

  if (does_ply_invert_colors[start_ply])
  {
    if (regular_starter==slice_get_starter(root_slice))
      StdString("  1...");
    else
      StdString("  1...  ...");

    next_movenumber = 2;
  }
  else
  {
    if (regular_starter==slice_get_starter(root_slice))
      next_movenumber = 1;
    else
    {
      StdString("  1...");
      next_movenumber = 2;
    }
  }

  TraceValue("%u\n",nbply);
  for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
  {
    TraceValue("%u",current_ply);
    TraceValue("%u",slice);
    TraceValue("%u",active_slice[current_ply]);
    TraceValue("%u\n",does_ply_invert_colors[current_ply]);
    if (slice!=active_slice[current_ply])
    {
      if (slices[slice].type==STBranchSeries
          && slices[active_slice[current_ply]].type==STBranchSeries)
      {
        next_movenumber = 1;
        starting_side = trait[current_ply];
      }

      slice = active_slice[current_ply];
    }

    TraceValue("%u",trait[current_ply]);
    TraceValue("%u\n",starting_side);
    if (trait[current_ply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }

    end_marker = (nbply==current_ply
                  ? slices[active_slice[current_ply]].u.leaf.goal
                  : no_goal);
    initneutre(advers(trait[current_ply]));
    jouecoup_no_test(current_ply);
    ecritcoup(current_ply,end_marker);
  }

  Message(NewLine);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

static captured_ply_type captured_ply[maxply+1];

/* Write an attacking move along with indentation, move number and
 * attack type decoration (! or ?)
 * @param ply identifies ply in which move was played
 * @param goal identifies goal reached with attacking move
 * @param type identifies decoration to be added if move_depth==1
 */
static void write_numbered_indented_attack(ply ply,
                                           Goal goal,
                                           attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%u\n",goal);

  TraceValue("%u",nbcou);
  TraceValue("%u",ply);
  TraceValue("%u",repere[ply]);
  TraceValue("%u\n",output_attack_types[ply]);

  if (output_attack_types[ply]==threat_attack
      && nr_continuations_written[move_depth]==0)
    Message(Threat);

  ++nr_continuations_written[move_depth];
  nbcou_of_last_attack_written[move_depth] = nbcou;

  if (move_depth>1)
  {
    sprintf(GlobalStr,"%*c",8*move_depth-8,blank);
    StdString(GlobalStr);
  }
  sprintf(GlobalStr,"%3d.",move_depth);
  StdString(GlobalStr);
  ecritcoup(ply,goal);

  capture_ply(&captured_ply[ply],ply);

  if (move_depth==1
      && output_attack_types[ply]!=unsolvability_attack)
  {
    switch (type)
    {
      case attack_try:
        StdString("? ");
        break;

      case attack_key:
        StdString("! ");
        if (OptFlag[maxsols])
          solutions++;
        if (OptFlag[beep])
          BeepOnSolution(maxbeep);
        break;

      default:
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write a move of the attacking side in direct play
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 * @param type of attack
 */
void write_attack(Goal goal, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u\n",type);

  if (current_mode==output_mode_tree)
    write_numbered_indented_attack(nbply,goal,type);
  else if (goal!=no_goal)
    linesolution();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write a defender's move that does not reach a goal
 */
void write_defense(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    TraceValue("%u",nr_defenses_written[move_depth]);
    TraceValue("%u\n",nr_continuations_written[move_depth+1]);

    if (nr_defenses_written[move_depth]==0
        && nr_continuations_written[move_depth+1]==0)
      Message(NewLine);

    ++nr_defenses_written[move_depth];

    sprintf(GlobalStr,"%*c",8*move_depth-4,blank);
    StdString(GlobalStr);
    sprintf(GlobalStr,"%3d...",move_depth);
    StdString(GlobalStr);
    ecritcoup(nbply,no_goal);
    Message(NewLine);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write a defender's final move
 * @param goal goal reached by the move (!=no_goal)
 */
void write_final_defense(Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",goal);

  assert(goal!=no_goal);

  if (current_mode==output_mode_tree)
  {
    TraceValue("%u",nr_defenses_written[move_depth]);
    TraceValue("%u",nr_continuations_written[move_depth]);
    TraceValue("%u",nbcou_of_last_attack_written[move_depth]);
    TraceValue("%u",nbply);
    TraceValue("%u",repere[nbply]);
    TraceValue("%u\n",output_attack_types[parent_ply[nbply]]);

    if (nr_continuations_written[move_depth]==0
        || !is_ply_equal_to_captured(&captured_ply[parent_ply[nbply]],
                                     parent_ply[nbply]))
    {
      ply const start_ply = 2;
      ply current_ply;
      ResetPosition();
      for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
      {
        initneutre(advers(trait[current_ply]));
        jouecoup_no_test(current_ply);
        if (current_ply==parent_ply[nbply])
          write_numbered_indented_attack(current_ply,no_goal,attack_key);
      }

      nr_defenses_written[move_depth] = 0;
    }

    if (nr_defenses_written[move_depth]==0
        && nr_continuations_written[move_depth+1]==0)
      Message(NewLine);

    ++nr_defenses_written[move_depth];

    sprintf(GlobalStr,"%*c",8*move_depth-4,blank);
    StdString(GlobalStr);
    sprintf(GlobalStr,"%3d...",move_depth);
    StdString(GlobalStr);
    ecritcoup(nbply,goal);
    Message(NewLine);
  }
  else
    linesolution();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the final move in a help leaf
 * @param goal goal reached by the move (!=no_goal)
 */
void write_final_help_move(Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",goal);

  assert(goal!=no_goal);

  if (current_mode==output_mode_tree)
    write_final_defense(goal);
  else
    linesolution();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Mark the defense about to be written as refutation
 */
void write_refutation_mark(void)
{
    sprintf(GlobalStr,"%*c",8*move_depth-2,blank);
    StdString(GlobalStr);
    Message(Refutation);
}

/* Write the end of a solution
 */
void write_end_of_solution(void)
{
  if (current_mode==output_mode_tree
      && output_attack_types[nbply]!=unsolvability_attack)
    Message(NewLine);
}

/* Write the end of a solution phase
 */
void write_end_of_solution_phase(void)
{
  Message(NewLine);
}

void editcoup(ply ply_id, coup *mov, Goal goal); /* TODO */

/* Write the refutations stored in a table
 * @param t table containing refutations
 */
void write_refutations(int t)
{
  if (tabsol.cp[t]!=tabsol.cp[t-1])
  {
    int n;

    if (nr_defenses_written[move_depth]==0
        && nr_continuations_written[move_depth+1]==0)
      Message(NewLine);

    sprintf(GlobalStr,"%*c",4,blank);
    StdString(GlobalStr);
    Message(But);
    for (n = tabsol.cp[t]; n>tabsol.cp[t-1]; n--)
    {
      StdString("      1...");
      editcoup(nbply,&tabsol.liste[n],no_goal);
      StdString(" !\n");

      ++nr_defenses_written[move_depth];
    }
  }
  Message(NewLine);
}
