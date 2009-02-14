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

static unsigned int nr_color_inversions_in_ply[maxply];

slice_index active_slice[maxply];

static stip_length_type move_depth;

static captured_ply_type captured_ply[maxply+1];


static output_mode detect_output_mode(slice_index si)
{
  output_mode result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STMoveInverter:
      result = detect_output_mode(slices[si].u.move_inverter.next);
      break;

    case STBranchDirect:
    case STLeafDirect:
    case STLeafSelf:
      result = output_mode_tree;
      break;

    case STBranchHelp:
    case STBranchSeries:
    case STLeafHelp:
      result = output_mode_line;
      break;

    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      output_mode mode1 = detect_output_mode(op1);

      slice_index const op2 = slices[si].u.quodlibet.op2;
      output_mode mode2 = detect_output_mode(op2);

      if (mode2!=output_mode_none)
        result = mode2;
      else
        result = mode1;
      break;
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.reciprocal.op1;
      output_mode mode1 = detect_output_mode(op1);

      slice_index const op2 = slices[si].u.reciprocal.op2;
      output_mode mode2 = detect_output_mode(op2);

      if (mode2!=output_mode_none)
        result = mode2;
      else
        result = mode1;
      break;
    }

    default:
      result = output_mode_none;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Initialize based on the stipulation
 */
void init_output(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");


  current_mode = detect_output_mode(root_slice);
  assert(current_mode!=output_mode_none);

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
    output_attack_types[nbply+1] = unsolvability_attack;

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


/* Start a new output level consisting of set play
 */
void output_start_move_inverted_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (current_mode==output_mode_tree)
  {
    ++move_depth;
    nr_continuations_written[move_depth+1] = 1; /* prevent initial newline */
    nr_defenses_written[move_depth] = 0;
  }

  ++nr_color_inversions_in_ply[nbply+1];

  TraceValue("%u",nbply+1);
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
    --move_depth;

  --nr_color_inversions_in_ply[nbply+1];

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
      && move_depth>1
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
    TraceValue("%u\n",move_depth);
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
  int next_movenumber = 1;
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
  starting_side = slice_get_starter(root_slice);

  ResetPosition();

  TraceValue("%u\n",nr_color_inversions_in_ply[start_ply]);

  switch (nr_color_inversions_in_ply[start_ply])
  {
    case 2:
      StdString("  1...  ...");
      next_movenumber = 2;
      break;

    case 1:
      StdString("  1...");
      next_movenumber = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }

  TraceValue("%u\n",nbply);
  for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
  {
    TraceValue("%u",current_ply);
    TraceValue("%u",slice);
    TraceValue("%u\n",active_slice[current_ply]);
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

/* Write an attacking move along with indentation, move number and
 * attack type decoration (! or ?)
 * @param current_ply identifies ply in which move was played
 * @param goal identifies goal reached with attacking move
 * @param type identifies decoration to be added if move_depth==1
 */
static void write_numbered_indented_attack(ply current_ply,
                                           Goal goal,
                                           attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u\n",type);

  if (output_attack_types[current_ply]==threat_attack
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
  ecritcoup(current_ply,goal);

  capture_ply(&captured_ply[current_ply],current_ply);
  invalidate_ply_snapshot(&captured_ply[current_ply+1]);

  if (move_depth==1
      && output_attack_types[current_ply]!=unsolvability_attack)
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
 * @param type of attack
 */
void write_attack(attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",type);

  if (current_mode==output_mode_tree)
    write_numbered_indented_attack(nbply,no_goal,type);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write a defense, corrently numbered and indented.
 * @param current_ply identifies the ply where the defense was played in
 * @param goal goal reached by the defense
 */
static void write_numbered_indented_defense(ply current_ply, Goal goal)
{
  sprintf(GlobalStr,"%*c",8*move_depth-4,blank);
  StdString(GlobalStr);
  sprintf(GlobalStr,"%3d...",move_depth);
  StdString(GlobalStr);
  ecritcoup(current_ply,goal);
  Message(NewLine);

  capture_ply(&captured_ply[current_ply],current_ply);
  invalidate_ply_snapshot(&captured_ply[current_ply+1]);

  ++nr_defenses_written[move_depth];
}

/* Write a move of the attacking side in direct play
 * @param goal goal reached by the move (no_goal if no goal has been
 *             reached by the move)
 * @param type of attack
 */
void write_final_attack(Goal goal, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u\n",type);

  assert(goal!=no_goal);
  
  if (current_mode==output_mode_tree)
  {
    ply const start_ply = 2;
    TraceValue("%u",move_depth);
    TraceValue("%u",nr_defenses_written[move_depth-1]);
    TraceValue("%u",nbply);
    TraceValue("%u\n",parent_ply[nbply]);
    if (nbply>start_ply
        && output_attack_types[nbply]!=threat_attack
        && (nr_defenses_written[move_depth-1]==0
            || !is_ply_equal_to_captured(&captured_ply[parent_ply[nbply]],
                                         parent_ply[nbply])))
    {
      ply current_ply;
      ResetPosition();
      for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
      {
        initneutre(advers(trait[current_ply]));
        jouecoup_no_test(current_ply);
        if (current_ply==parent_ply[nbply])
        {
          --move_depth;
          write_numbered_indented_defense(current_ply,no_goal);
          ++move_depth;
        }
      }

      nr_defenses_written[move_depth] = 0;

      write_numbered_indented_attack(nbply,goal,type);
    }
    else
      write_numbered_indented_attack(nbply,goal,type);
  }
  else
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
    TraceValue("%u",move_depth);
    TraceValue("%u",nbply);
    TraceValue("%u",nr_defenses_written[move_depth]);
    TraceValue("%u\n",nr_continuations_written[move_depth+1]);

    if (nr_defenses_written[move_depth]==0
        && nr_continuations_written[move_depth+1]==0)
      Message(NewLine);

    write_numbered_indented_defense(nbply,no_goal);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the attack played in a specific ply, and moves preceding it,
 * if they haven't been written yet.
 * This is an indirectly recursive function
 * @param current_ply identifies the ply the defense was played in
 */
static void catchup_with_attack(ply current_ply);

/* Write the defense played in a specific ply, and moves preceding it,
 * if they haven't been written yet.
 * This is an indirectly recursive function
 * @param current_ply identifies the ply the defense was played in
 */
static void catchup_with_defense(ply current_ply)
{
  ply const start_ply = 2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",current_ply);

  TraceValue("%u\n",move_depth);

  if (current_ply>start_ply)
    catchup_with_attack(parent_ply[current_ply]);

  initneutre(advers(trait[current_ply]));
  jouecoup_no_test(current_ply);
  TraceCurrentMove(current_ply);

  if (!is_ply_equal_to_captured(&captured_ply[current_ply],current_ply))
    write_numbered_indented_defense(current_ply,no_goal);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the attack played in a specific ply, and moves preceding it,
 * if they haven't been written yet.
 * This is an indirectly recursive function
 * @param current_ply identifies the ply the defense was played in
 */
static void catchup_with_attack(ply current_ply)
{
  ply const start_ply = 2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",current_ply);

  TraceValue("%u\n",move_depth);

  if (current_ply>start_ply)
  {
    if (output_attack_types[current_ply]==threat_attack)
      catchup_with_attack(parent_ply[current_ply]);
    else
      catchup_with_defense(parent_ply[current_ply]);
  }

  ++move_depth;

  initneutre(advers(trait[current_ply]));
  jouecoup_no_test(current_ply);
  TraceCurrentMove(current_ply);

  if (!is_ply_equal_to_captured(&captured_ply[current_ply],current_ply))
  {
    write_numbered_indented_attack(current_ply,no_goal,attack_key);
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
    stip_length_type const save_move_depth = move_depth;
    TraceValue("%u",move_depth);
    TraceValue("%u",nr_continuations_written[move_depth]);
    TraceValue("%u",nbply);
    TraceValue("%u\n",output_attack_types[parent_ply[nbply]]);

    if (nr_continuations_written[move_depth]==0
        || !is_ply_equal_to_captured(&captured_ply[parent_ply[nbply]],
                                     parent_ply[nbply]))
    {
      ply const start_ply = 2;
      if (nbply>start_ply)
      {
        ResetPosition();
        move_depth = nr_color_inversions_in_ply[start_ply]>0 ? 1 : 0;
        catchup_with_attack(parent_ply[nbply]);
        nr_defenses_written[move_depth] = 0;

        initneutre(advers(trait[nbply]));
        jouecoup_no_test(nbply);
      }
    }
    else if (nr_defenses_written[move_depth]==0)
      Message(NewLine);

    write_numbered_indented_defense(nbply,goal);

    move_depth = save_move_depth;
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
  TraceFunctionEntry(__func__);
  TraceText("\n");

  Message(NewLine);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

void editcoup(ply ply_id, coup *mov, Goal goal); /* TODO */

/* Write a move as a refutation
 * @param c address of the structure representing the move
 */
static void write_refutation(coup *c)
{
  StdString("      1...");
  editcoup(nbply,c,no_goal);
  StdString(" !\n");

  ++nr_defenses_written[move_depth];
}

/* Write the refutations stored in a table
 * @param refutations table containing refutations
 */
void write_refutations(table refutations)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (table_length(refutations)!=0)
  {
    if (nr_defenses_written[move_depth]==0
        && nr_continuations_written[move_depth+1]==0)
      Message(NewLine);

    sprintf(GlobalStr,"%*c",4,blank);
    StdString(GlobalStr);
    Message(But);
    table_iterate(refutations,&write_refutation);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}
