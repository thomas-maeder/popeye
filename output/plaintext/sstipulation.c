#include "output/plaintext/sstipulation.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/language_dependant.h"
#include "stipulation/branch.h"
#include "debugging/assert.h"

typedef enum
{
  write_state_begin,
  write_state_branch,
  write_state_end
} branch_position_state;

typedef enum
{
  play_attack,
  play_defense,
  play_help,
  play_series,

  nr_plays,
  play_unknown = nr_plays
} play_type;

typedef struct
{
    FILE *file;
    int nr_chars_written;
    branch_position_state branch_position;
    play_type play;
} state_type;

static void write_move_inverter(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  state->nr_chars_written += fprintf(state->file,"%s","-");
  stip_traverse_structure_children(si,st);
}

static void write_attack(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  play_type const save_play = state->play;

  if (state->branch_position==write_state_begin)
  {
    state->nr_chars_written += fprintf(state->file,
                                       "%u",
                                       SLICE_U(si).branch.length);
    if (SLICE_U(si).branch.min_length>1)
      state->nr_chars_written += fprintf(state->file,
                                         ":%u",
                                         SLICE_U(si).branch.min_length+1);
  }

  state->play = play_attack;
  stip_traverse_structure_children(si,st);
  state->play = save_play;
}

static void write_attack_played(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  assert(state->play==play_attack);

  if (state->branch_position==write_state_begin)
  {
    state->branch_position = write_state_branch;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_begin;
  }
  else
  {
    assert(state->branch_position==write_state_branch);
    state->branch_position = write_state_end;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_branch;
  }
}

static void write_defense(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  play_type const save_play = state->play;

  if (state->branch_position==write_state_begin)
  {
    if (branch_find_slice(STPlaySuppressor,si,st->context)!=no_slice)
      state->nr_chars_written += fprintf(state->file,"%s","/");
    state->nr_chars_written += fprintf(state->file,
                                       "%u",
                                       SLICE_U(si).branch.length);
    if (SLICE_U(si).branch.min_length>1)
      state->nr_chars_written += fprintf(state->file,
                                         ":%u",
                                         SLICE_U(si).branch.min_length);
  }

  state->play = play_defense;
  stip_traverse_structure_children(si,st);
  state->play = save_play;
}

static void write_defense_played(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  assert(state->play==play_defense);

  if (state->branch_position==write_state_begin)
  {
    state->branch_position = write_state_branch;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_begin;
  }
  else
  {
    assert(state->branch_position==write_state_branch);
    state->branch_position = write_state_end;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_branch;
  }
}

static void write_help(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (state->branch_position==write_state_begin)
  {
    play_type const save_play = state->play;
    slice_index const leg2 = branch_find_slice(STReadyForHelpMove,si,st->context);
    if (leg2==si)
    {
      state->nr_chars_written += fprintf(state->file,
                                         "%u",
                                         (SLICE_U(si).branch.length+1)/2);
      if (SLICE_U(si).branch.min_length>1)
        state->nr_chars_written += fprintf(state->file,
                                           ":%u",
                                           (SLICE_U(si).branch.min_length+1)/2);
      state->play = play_series;
    }
    else
    {
      state->nr_chars_written += fprintf(state->file,
                                         "%u",
                                         SLICE_U(si).branch.length);
      if (SLICE_U(si).branch.min_length>1)
        state->nr_chars_written += fprintf(state->file,
                                           ":%u",
                                           SLICE_U(si).branch.min_length);
      state->play = play_help;
    }

    stip_traverse_structure_children(si,st);
    state->play = save_play;
  }
  else
    stip_traverse_structure_children(si,st);
}

static void write_help_played(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (state->branch_position==write_state_begin)
  {
    state->branch_position = state->play==play_help ? write_state_branch : write_state_end;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_begin;
  }
  else if (state->branch_position==write_state_branch)
  {
    state->branch_position = write_state_end;
    stip_traverse_structure_children(si,st);
    state->branch_position = write_state_branch;
  }
}

static void write_move(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  char const play2char[nr_plays] = { 'a','d','h','s' };

  assert(state->play<nr_plays);
  state->nr_chars_written += fprintf(state->file,
                                     "%c",
                                     play2char[state->play]);

  stip_traverse_structure_children(si,st);
}

static void write_next_branch(slice_index si, stip_structure_traversal *st,
                              char const *entry, char const *exit)
{
  state_type * const state = st->param;
  branch_position_state const save_state = state->branch_position;
  play_type const save_play = state->play;

  state->nr_chars_written += fprintf(state->file,"%s",entry);

  state->branch_position = write_state_begin;
  state->play = play_unknown;
  stip_traverse_structure_next_branch(si,st);
  state->play = save_play;
  state->branch_position = save_state;

  state->nr_chars_written += fprintf(state->file,"%s",exit);
}

static void write_end_of_branch(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (state->branch_position==write_state_begin)
  {
    /* don't write the nested branch at the beginning */
    stip_traverse_structure_children_pipe(si,st);
    write_next_branch(si,st,"[","]");
  }
  else
  {
    write_next_branch(si,st,"[","]");
    stip_traverse_structure_children_pipe(si,st);
  }
}

static void write_end_of_branch_forced(slice_index si, stip_structure_traversal *st)
{
  write_next_branch(si,st,"[>","]");
  stip_traverse_structure_children_pipe(si,st);
}

static void write_constraint_tester(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  if (state->branch_position==write_state_begin)
  {
    /* don't write the constraint at the beginning */
    stip_traverse_structure_children_pipe(si,st);
    write_next_branch(si,st,"{","}");
  }
  else
  {
    write_next_branch(si,st,"{","}");
    stip_traverse_structure_children_pipe(si,st);
  }
}

static void write_goal_reached_tester(slice_index si, stip_structure_traversal *st)
{
  write_next_branch(si,st,"","");
}

static void write_mate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","#");
}

static void write_stalemate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","=");
}

static void write_doublestalemate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","==");
}

static void write_immobile(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","#=");
}

static void write_check(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","+");
}

static void write_target(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  square const s = SLICE_U(si).goal_handler.goal.target;

  state->nr_chars_written += fprintf(state->file,"%s","z");
  /* TODO avoid duplication with WriteSquare() */
  state->nr_chars_written += fprintf(state->file,"%c",('a' - nr_files_on_board + s%onerow));
  state->nr_chars_written += fprintf(state->file,"%c",('1' - nr_rows_on_board + s/onerow));
}

static void write_capture(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","x");
}

static void write_castling(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","00");
}

static void write_steingewinn(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","%");
}

static void write_enpassant(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","ep");
}

static void write_doublemate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","##");
}

static void write_countermate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","##!");
}

static void write_autostalemate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","!=");
}

static void write_automate(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","!#");
}

static void write_circuit(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","ct");
}

static void write_exchange(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","<>");
}

static void write_any(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","~");
}

static void write_dia(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","dia");
}

static void write_atob(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","a=>b");
}

static void write_chess81(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","c81");
}

static void write_kiss(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","k");
}

static void write_if_then_else(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  branch_position_state const save_state = state->branch_position;
  stip_traversal_context_type const save_context = st->context;
  play_type const save_play = state->play;

  state->nr_chars_written += fprintf(state->file,"%s","?");
  state->branch_position = write_state_begin;
  state->play = play_unknown;
  st->context = stip_traversal_context_intro;
  stip_traverse_structure(SLICE_U(si).if_then_else.condition,st);
  st->context = save_context;
  state->play = save_play;
  state->branch_position = save_state;
  state->nr_chars_written += fprintf(state->file,"%s","?");

  stip_traverse_structure_binary_operand1(si,st);
  stip_traverse_structure_binary_operand2(si,st);
}

static void write_not(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;
  state->nr_chars_written += fprintf(state->file,"%s","!");
  stip_traverse_structure_children_pipe(si,st);
}

static void write_and(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  stip_traverse_structure_binary_operand1(si,st);
  state->nr_chars_written += fprintf(state->file,"%s","&");
  stip_traverse_structure_binary_operand2(si,st);
}

static void write_or(slice_index si, stip_structure_traversal *st)
{
  state_type * const state = st->param;

  stip_traverse_structure_binary_operand1(si,st);
  state->nr_chars_written += fprintf(state->file,"%s","|");
  stip_traverse_structure_binary_operand2(si,st);
}

static structure_traversers_visitor const visitors[] = {
    { STMoveInverter, &write_move_inverter },
    { STReadyForAttack, &write_attack },
    { STAttackPlayed, &write_attack_played },
    { STReadyForDefense, &write_defense },
    { STDefensePlayed, &write_defense_played },
    { STReadyForHelpMove, &write_help },
    { STHelpMovePlayed, &write_help_played },
    { STMove, &write_move },
    { STEndOfBranch, &write_end_of_branch },
    { STEndOfBranchGoal, &write_end_of_branch },
    { STEndOfBranchGoalImmobile, &write_end_of_branch },
    { STEndOfBranchForced, &write_end_of_branch_forced },
    { STConstraintTester, &write_constraint_tester },
    { STGoalConstraintTester, &write_constraint_tester },
    { STGoalReachedTester, &write_goal_reached_tester },
    { STGoalMateReachedTester, &write_mate },
    { STGoalStalemateReachedTester, &write_stalemate },
    { STGoalDoubleStalemateReachedTester, &write_doublestalemate },
    { STGoalImmobileReachedTester, &write_immobile },
    { STGoalCheckReachedTester, &write_check },
    { STGoalTargetReachedTester, &write_target },
    { STGoalCaptureReachedTester, &write_capture },
    { STGoalCastlingReachedTester, &write_castling },
    { STGoalSteingewinnReachedTester, &write_steingewinn },
    { STGoalEnpassantReachedTester, &write_enpassant },
    { STGoalDoubleMateReachedTester, &write_doublemate },
    { STGoalCounterMateReachedTester, &write_countermate },
    { STGoalAutoStalemateReachedTester, &write_autostalemate },
	{ STGoalAutoMateReachedTester, &write_automate },
    { STGoalCircuitReachedTester, &write_circuit },
    { STGoalExchangeReachedTester, &write_exchange },
    { STGoalAnyReachedTester, &write_any },
    { STGoalProofgameReachedTester, &write_dia },
    { STGoalAToBReachedTester, &write_atob },
    { STGoalChess81ReachedTester, &write_chess81 },
    { STGoalKissReachedTester, &write_kiss },
    { STIfThenElse, &write_if_then_else },
    { STNot, &write_not },
    { STAnd, &write_and },
    { STOr, &write_or }
};
enum { nr_visitors = sizeof visitors / sizeof visitors[0] };

/* Write a structured stiplation
 * @param si identiifes the entry slice into the stipulation
 * @return number of characters written
 */
int WriteSStipulation(FILE *file, slice_index stipulation)
{
  Side const starter = SLICE_STARTER(stipulation);
  state_type state = { file, 0, write_state_begin, play_unknown };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation);
  TraceFunctionParamListEnd();

  TraceStipulation(stipulation);

  state.nr_chars_written += fprintf(file,"%s ",ColourTab[starter]);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override(&st,visitors,nr_visitors);
    stip_traverse_structure(stipulation,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.nr_chars_written);
  TraceFunctionResultEnd();
  return state.nr_chars_written;
}
