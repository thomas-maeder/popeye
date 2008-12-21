#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyint.h"
#include "trace.h"

static output_mode current_mode = output_mode_none;

/* Select the inital output mode
 * @param output_mode initial output mode
 */
void init_output_mode(output_mode initial_mode)
{
  current_mode = initial_mode;
}

slice_index active_slice[maxply];


/* TODO */
extern boolean SatzFlag;

static void linesolution(void)
{
  int next_movenumber;
  Side starting_side;
  Goal end_marker;
  slice_index slice;
  ply current_ply;
      
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

  flag_writinglinesolution= true;
  slice = active_slice[2];
  starting_side = regular_starter;

  ResetPosition();

  if (regular_starter!=slices[0].starter)
  {
    StdString("  1...");
    next_movenumber = 2;
  }
  else if (SatzFlag)
  {
    StdString("  1...  ...");
    next_movenumber = 2;
  }
  else
    next_movenumber = 1;

  TraceValue("%u\n",nbply);
  for (current_ply = 2; current_ply<=nbply; ++current_ply)
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

  flag_writinglinesolution= false;

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
    if (DrohFlag)
    {
      Message(Threat);
      DrohFlag = false;
    }

    Tabulate();
    sprintf(GlobalStr,"%3d.",zugebene);
    StdString(GlobalStr);
    ecritcoup(nbply,goal);

    if (zugebene==1)
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

/* Conclude writing an attacker's move; each call to write_attack()
 * should be followed by a call to write_attack_conclusion(),
 * indicating whether we want to signal zugzwang in the ouptut.
 * @param type should we signal zugzwang in the output?
 */
void write_attack_conclusion(attack_conclusion_type type)
{
  if (current_mode==output_mode_tree)
    Message(type==attack_with_zugzwang ? Zugzwang : NewLine);
}

/* Write a move by the defending side in direct/self/reflex play.
 * @param goal if !=no_goal, the corresponding mark is appended
 */
void write_defense(Goal goal)
{
  if (current_mode==output_mode_tree)
  {
    Tabulate();
    sprintf(GlobalStr,"%3d...",zugebene);
    StdString(GlobalStr);
    ecritcoup(nbply,goal);
    Message(NewLine);
  }
  else
    linesolution();
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
    Tabulate();
    Message(But);
    for (n = tabsol.cp[t]; n>tabsol.cp[t-1]; n--)
    {
      Tabulate();
      StdString("  1...");
      editcoup(nbply,&tabsol.liste[n],no_goal);
      StdString(" !\n");
    }
  }
  StdChar('\n');
}
