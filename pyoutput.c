#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>

static output_mode current_mode = output_mode_none;

static unsigned int margin;

extern boolean SatzFlag;

slice_index active_slice[maxply];

static stip_length_type zugebene;


/* Select the inital output mode
 * @param output_mode initial output mode
 */
void init_output_mode(output_mode initial_mode)
{
  current_mode = initial_mode;
}

void output_indent(void)
{
  margin += 4;
}

void output_outdent(void)
{
  assert(margin>=4);
  margin -= 4;
}

/* Write the appropriate amount of whitespace for the following output
 * to be correctely indented.
 */
void write_indentation() {
  /* sprintf() would print 1 blank if mage is ==0! */
  assert(margin>=4);
  if (margin-4>0)
  {
    sprintf(GlobalStr,"%*c",margin-4,blank);
    StdString(GlobalStr);
  }
}

typedef enum
{
  unknown_attack,
  continuation_attack,
  threat_attack
} output_attack_type;

static output_attack_type output_attack_types[maxply];
static unsigned int nr_continuations_written[maxply];


/* Start a new output level consisting of post-key only play
 */
void output_start_postkeyonly_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  zugebene++;

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of post-key only play)
 */
void output_end_postkeyonly_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  Message(NewLine);

  zugebene--;

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of set play
 */
void output_start_setplay_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  output_indent();
  zugebene++;

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* End the inner-most output level (which consists of set play)
 */
void output_end_setplay_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  zugebene--;
  output_outdent();

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of threats
 */
void output_start_threat_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  output_indent();
  zugebene++;

  /* nbply will be increased by genmove() in a moment */
  output_attack_types[nbply+1] = threat_attack;
  nr_continuations_written[zugebene] = 0;

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

  if (nr_continuations_written[zugebene]==0)
    write_attack_conclusion(attack_with_zugzwang);

  TraceValue("%u",nbply);
  TraceValue("%u\n",output_attack_types[nbply+1]);

  assert(output_attack_types[nbply+1]==threat_attack);
  output_attack_types[nbply+1] = unknown_attack;

  zugebene--;
  output_outdent();

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Start a new output level consisting of regular continuations
 */
void output_start_continuation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  output_indent();
  zugebene++;

  /* nbply will be increased by genmove() in a moment */
  output_attack_types[nbply+1] = continuation_attack;
  nr_continuations_written[zugebene] = 0;

  TraceValue("%u",nbply);
  TraceValue("%u\n",output_attack_types[nbply+1]);

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

  if (zugebene==2 && nr_continuations_written[zugebene]==0)
    write_refutation_mark();

  zugebene--;
  output_outdent();

  TraceValue("%u",nbply);
  TraceValue("%u\n",output_attack_types[nbply+1]);

  assert(output_attack_types[nbply+1]==continuation_attack);

  output_attack_types[nbply+1] = unknown_attack;

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
        && nr_continuations_written[zugebene]==0)
      write_attack_conclusion(attack_with_threat);

    ++nr_continuations_written[zugebene];

    write_indentation();
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
  {
    message_id_t message_id;

    switch (type)
    {
      case attack_with_zugzwang:
        message_id = Zugzwang;
        break;

      case attack_with_threat:
        message_id = Threat;
        break;

      case attack_with_nothing:
        message_id = NewLine;
        break;

      default:
        assert(0);
        break;
    }

    Message(message_id);
  }
}

/* Write a move by the defending side in direct/self/reflex play.
 * @param goal if !=no_goal, the corresponding mark is appended
 */
void write_defense(Goal goal)
{
  if (current_mode==output_mode_tree)
  {
    write_indentation();
    sprintf(GlobalStr,"%3d...",zugebene);
    StdString(GlobalStr);
    ecritcoup(nbply,goal);
    Message(NewLine);
  }
  else
    linesolution();
}

/* Mark the defense about to be written as refutation
 */
void write_refutation_mark(void)
{
    margin += 2;
    write_indentation();
    Message(Refutation);
    margin -= 2;
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
    write_indentation();
    Message(But);
    for (n = tabsol.cp[t]; n>tabsol.cp[t-1]; n--)
    {
      write_indentation();
      StdString("  1...");
      editcoup(nbply,&tabsol.liste[n],no_goal);
      StdString(" !\n");
    }
  }
  StdChar('\n');
}
