#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>

static output_mode current_mode = output_mode_none;

static boolean areWeSolvingSetplay;

slice_index active_slice[maxply];

static stip_length_type move_depth;


/* Select the inital output mode
 * @param output_mode initial output mode
 */
void init_output_mode(output_mode initial_mode)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",initial_mode);

  current_mode = initial_mode;

  TraceFunctionExit(__func__);
  TraceText("\n");
}

typedef enum
{
  unknown_attack,
  continuation_attack,
  threat_attack
} output_attack_type;

static output_attack_type output_attack_types[maxply];
static unsigned int nr_continuations_written[maxply];
static unsigned int nr_defenses_written[maxply];

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
void output_start_setplay_level(void)
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
    areWeSolvingSetplay = true;

  TraceValue("%u\n",move_depth);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of set play)
 */
void output_end_setplay_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
    move_depth--;
  else
    areWeSolvingSetplay = false;

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
    Message(NewLine);

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

    assert(output_attack_types[nbply+1]==continuation_attack);

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
  TraceValue("%u",slices[0].starter);
  TraceValue("%u\n",areWeSolvingSetplay);

  if (areWeSolvingSetplay)
  {
    if (regular_starter==slices[0].starter)
      StdString("  1...");
    else
      StdString("  1...  ...");

    next_movenumber = 2;
  }
  else
  {
    if (regular_starter==slices[0].starter)
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
    TraceValue("%u\n",active_slice[current_ply]);
    if (slice!=active_slice[current_ply])
    {
      slice = active_slice[current_ply];
      if (slices[slice].type!=STLeaf)
      {
        next_movenumber = 1;
        starting_side = trait[current_ply];
      }
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

/* Write a move of the attacking side in direct play
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 * @param type of attack
 */
void write_attack(Goal goal, attack_type type)
{
  if (current_mode==output_mode_tree)
  {
    assert(output_attack_types[nbply]==continuation_attack
           || output_attack_types[nbply]==threat_attack);

    if (output_attack_types[nbply]==threat_attack
        && nr_continuations_written[move_depth]==0)
      Message(Threat);

    ++nr_continuations_written[move_depth];

    if (move_depth>1)
    {
      sprintf(GlobalStr,"%*c",8*move_depth-8,blank);
      StdString(GlobalStr);
    }
    sprintf(GlobalStr,"%3d.",move_depth);
    StdString(GlobalStr);
    ecritcoup(nbply,goal);

    if (move_depth==1)
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
  }
  else
  {
    if (goal!=no_goal)
      linesolution();
  }
}

/* Write a defender's move that does not reach a goal
 */
void write_defense(void)
{
  TraceFunctionEntry(__func__);

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
    TraceValue("%u\n",nr_continuations_written[move_depth+1]);

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
  {
    if (nbply==3 && nr_defenses_written[1]==0)
    {
      /* We only arrive here when solving a (semi-)rX1 problem and have
       * just found a move pair solving the stipulation. Write the key
       * move, then the final help move.
       * We don't write the key move earlier because we have only
       * found out that it is the key move when we found this final
       * defense.
       */
      int const first_movenumber = 1;
      ply const first_ply = 2;
      
      if (OptFlag[maxsols])
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);

      ResetPosition();
      initneutre(advers(trait[first_ply]));
      jouecoup_no_test(first_ply);

      sprintf(GlobalStr,"%3d.",first_movenumber);
      StdString(GlobalStr);
      ecritcoup(first_ply,no_goal);
      StdString("! ");
    }

    write_final_defense(goal);
  }
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
  if (current_mode==output_mode_tree)
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
