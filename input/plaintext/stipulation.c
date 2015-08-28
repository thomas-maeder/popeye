#include "input/plaintext/stipulation.h"
#include "input/plaintext/goal.h"
#include "input/plaintext/token.h"
#include "input/plaintext/problem.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/message.h"
#include "conditions/check_zigzag.h"
#include "position/position.h"
#include "position/pieceid.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/not.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>
#include <string.h>

char AlphaStip[200];

static void alloc_reci_end(slice_index proxy_nonreci,
                           slice_index proxy_reci,
                           slice_index proxy_to_nonreci,
                           slice_index proxy_to_reci)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_nonreci);
  TraceFunctionParam("%u",proxy_reci);
  TraceFunctionParam("%u",proxy_to_nonreci);
  TraceFunctionParam("%u",proxy_to_reci);
  TraceFunctionParamListEnd();

  {
    slice_index const branch_nonreci = alloc_help_branch(2,2);
    slice_index const branch_reci = alloc_help_branch(1,1);

    help_branch_set_end_goal(branch_nonreci,proxy_to_nonreci,1);
    link_to_branch(proxy_nonreci,branch_nonreci);

    help_branch_set_end_goal(branch_reci,proxy_to_reci,1);
    link_to_branch(proxy_reci,branch_reci);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseReciGoal(char *tok,
                           slice_index start,
                           slice_index proxy_nonreci,
                           slice_index proxy_reci)
{
  char *result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  if (*tok=='(')
  {
    char const *closingParenPos = strchr(tok,')');
    if (closingParenPos!=0)
    {
      slice_index const proxy_to_reci = alloc_proxy_slice();
      tok = ParseGoal(tok+1,start,proxy_to_reci);
      if (tok!=0)
      {
        if (tok==closingParenPos)
        {
          slice_index const proxy_to_nonreci = alloc_proxy_slice();
          result = ParseGoal(tok+1,start,proxy_to_nonreci);
          if (result!=NULL)
          {
            slice_index const nonreci = SLICE_NEXT1(proxy_to_nonreci);
            SLICE_STARTER(nonreci) = Black;
            alloc_reci_end(proxy_nonreci,proxy_reci,
                           proxy_to_nonreci,proxy_to_reci);
          }
        }
        else
          output_plaintext_input_error_message(UnrecStip, 0);
      }
    }
    else
      output_plaintext_input_error_message(UnrecStip, 0);
  }
  else
  {
    slice_index const proxy_to_nonreci = alloc_proxy_slice();
    result = ParseGoal(tok,start,proxy_to_nonreci);
    if (result!=NULL)
    {
      slice_index const nonreci_testing = SLICE_NEXT1(proxy_to_nonreci);
      slice_index const nonreci_tester = SLICE_NEXT1(nonreci_testing);
      slice_index const proxy_to_reci = stip_deep_copy(proxy_to_nonreci);
      alloc_reci_end(proxy_nonreci,proxy_reci,
                     proxy_to_nonreci,proxy_to_reci);
      SLICE_STARTER(nonreci_tester) = Black;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseReciEnd(char *tok, slice_index start, slice_index proxy)
{
  slice_index op1;
  slice_index op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  op1 = alloc_proxy_slice();
  op2 = alloc_proxy_slice();

  tok = ParseReciGoal(tok,start,op1,op2);
  if (SLICE_NEXT1(op1)!=no_slice && SLICE_NEXT1(op2)!=no_slice)
  {
    slice_index const reci = alloc_and_slice(op1,op2);
    pipe_link(proxy,reci);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseLength(char *tok, stip_length_type *length)
{
  char *end;
  unsigned long tmp_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  if (tok!=0 && *tok==0)
    /* allow white space before length, e.g. "dia 4" */
  {
    tok = ReadNextTokStr();
    if (tok!=0)
      strcat(AlphaStip,tok); /* append to printed stipulation */
  }

  tmp_length = strtoul(tok,&end,10);
  TraceValue("%ld\n",tmp_length);

  if (tok==end || tmp_length>UINT_MAX)
  {
    output_plaintext_input_error_message(WrongInt,0);
    tok = 0;
  }
  else
  {
    *length = tmp_length;
    tok = end;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseBattleLength(char *tok, stip_length_type *length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    if (*length==0)
    {
      output_plaintext_input_error_message(WrongInt,0);
      tok = 0;
    }
    else
    {
      /* we count half moves in battle play */
      *length *= 2;
      *length -= 1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

typedef enum
{
  play_length_minimum,
  play_length_exact
} play_length_type;

static char *ParseBattle(char *tok,
                         slice_index start,
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length,
                         boolean ends_on_defense)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",ends_on_defense);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,start,proxy_goal);
  if (result!=0)
  {
    stip_length_type length = 0;
    result = ParseBattleLength(result,&length);
    if (ends_on_defense)
      ++length;
    if (result!=0)
    {
      stip_length_type const min_length = (play_length==play_length_minimum
                                           ? 1
                                           : length-1);
      link_to_branch(proxy,alloc_battle_branch(length,min_length));
      solving_impose_starter(proxy,White);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static void attach_help_branch(stip_length_type length,
                               slice_index proxy,
                               slice_index branch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  if (length%2==1)
  {
    slice_index const inverter = alloc_move_inverter_slice();
    pipe_link(proxy,inverter);
    link_to_branch(inverter,branch);
  }
  else
    link_to_branch(proxy,branch);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void select_output_mode(slice_index si, output_mode mode)
{
  slice_index const prototype = alloc_output_mode_selector(mode);
  slice_insertion_insert(si,&prototype,1);
}

static char *ParseHelpLength(char *tok,
                             stip_length_type *length,
                             stip_length_type *min_length,
                             play_length_type play_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    /* we count half moves in help play */
    *length *= 2;

    if (strncmp(tok,".5",2)==0)
    {
      ++*length;
      tok += 2;
      *min_length = 1;
    }
    else
      *min_length = 0;

    if (play_length==play_length_exact)
      *min_length = *length;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseHelp(char *tok,
                       slice_index start,
                       slice_index proxy,
                       slice_index proxy_goal,
                       play_length_type play_length,
                       boolean shorten)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",shorten);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,start,proxy_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseHelpLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      slice_index const branch = alloc_help_branch(length,min_length);
      if (shorten)
        help_branch_shorten(branch);
      attach_help_branch(length,proxy,branch);
      select_output_mode(proxy,output_mode_line);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseHelpDia(char *tok,
                          slice_index start,
                          slice_index proxy,
                          slice_index proxy_next,
                          play_length_type play_length)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_next);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,start,proxy_next);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseHelpLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      link_to_branch(proxy,alloc_help_branch(length,min_length));
      select_output_mode(proxy,output_mode_line);
      help_branch_set_end_goal(proxy,proxy_next,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseSeriesLength(char *tok,
                               stip_length_type *length,
                               stip_length_type *min_length,
                               play_length_type play_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    if (*length==0)
    {
      output_plaintext_input_error_message(WrongInt,0);
      tok = 0;
    }
    else
    {
      /* we count half moves in series play */
      *length *= 2;
      *length -= 1;
      if (play_length==play_length_minimum)
        *min_length = 1;
      else
        *min_length = *length;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseSeries(char *tok,
                         slice_index start,
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,start,proxy_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseSeriesLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      slice_index const branch = alloc_series_branch(length,min_length);
      link_to_branch(proxy,branch);
      select_output_mode(proxy,output_mode_line);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_index MakeSemireflexBranch(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = alloc_battle_branch(1,0);
    result = alloc_proxy_slice();
    link_to_branch(result,branch);
    battle_branch_insert_direct_end_of_branch_goal(branch,proxy_to_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_index MakeReflexBranch(slice_index proxy_to_semi)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_semi);
  TraceFunctionParamListEnd();

  result = stip_deep_copy(proxy_to_semi);
  pipe_append(result,alloc_not_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce the sequence of slices that tests whether a self stipulation has been
 * solved
 * @param proxy_to_goal identifies sequence of slices testing for the goal
 * @return identifier of the entry slice
 */
static slice_index MakeEndOfSelfPlay(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_defense_branch(1,1);
  battle_branch_insert_self_end_of_branch_goal(result,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParsePlay(char *tok,
                       slice_index start,
                       slice_index proxy,
                       play_length_type play_length)
{
  /* seriesmovers with introductory moves */
  char *result = 0;
  char *arrowpos;
  slice_index const proxy_next = alloc_proxy_slice();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (token_starts_with("exact-",tok))
  {
    play_length = play_length_exact;
    tok += 6;
  }

  arrowpos = strstr(tok,"->");
  if (arrowpos!=0)
  {
    char *end;
    unsigned long const intro_len= strtoul(tok,&end,10);
    if (intro_len<1 || tok==end || end!=arrowpos)
      output_plaintext_input_error_message(WrongInt, 0);
    else
    {
      result = ParsePlay(arrowpos+2,start,proxy_next,play_length);
      if (result!=0 && SLICE_NEXT1(proxy_next)!=no_slice)
      {
        /* >=1 move of starting side required */
        slice_index const branch = alloc_series_branch(2*intro_len-1,1);
        help_branch_set_end(branch,proxy_next,1);
        link_to_branch(proxy,branch);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (token_starts_with("ser-reci-h",tok))
  {
    /* skip over "ser-reci-h" */
    tok = ParseReciEnd(tok+10,start,proxy_next);
    if (tok!=0 && SLICE_NEXT1(proxy_next)!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const branch = alloc_series_branch(length-1,min_length+1);
        help_branch_set_end(branch,proxy_next,1);
        link_to_branch(proxy,branch);

        solving_impose_starter(proxy_next,Black);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (token_starts_with("ser-hs",tok))
  {
    tok = ParseGoal(tok+6,start,proxy_next); /* skip over "ser-hs" */
    if (tok!=0)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_next);

        /* in ser-hs, the series is 1 half-move longer than in usual
         * series play! */
        if (length==0)
          pipe_link(proxy,defense_branch);
        else
        {
          slice_index const series = alloc_series_branch(length,min_length);

          slice_index const help_proxy = alloc_proxy_slice();
          slice_index const help = alloc_help_branch(1,1);
          link_to_branch(help_proxy,help);
          help_branch_set_end_forced(help_proxy,defense_branch,1);
          help_branch_set_end(series,help_proxy,1);
          link_to_branch(proxy,series);
        }

        solving_impose_starter(proxy_next,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (token_starts_with("ser-h",tok))
  {
    result = ParseSeries(tok+5,start,proxy,proxy_next,play_length); /* skip over "ser-h" */
    if (result!=0)
    {
      slice_index const help = alloc_help_branch(1,1);
      help_branch_set_end_goal(help,proxy_next,1);
      help_branch_set_end(proxy,help,1);

      {
        slice_index const next = SLICE_NEXT1(proxy_next);
        assert(next!=no_slice);
        if (SLICE_TYPE(next)==STGoalReachedTester
            && SLICE_U(next).goal_handler.goal.type==goal_proofgame)
          solving_impose_starter(proxy_next,White);
        else
          solving_impose_starter(proxy_next,Black);
      }
    }
  }

  else if (token_starts_with("ser-s",tok))
  {
    result = ParseSeries(tok+5,start,proxy,proxy_next,play_length); /* skip over "ser-s" */
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_next),1);
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("ser-r",tok))
  {
    result = ParseSeries(tok+5,start,proxy,proxy_next,play_length); /* skip over "ser-r" */
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_next);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      series_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi));
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("ser-",tok))
  {
    result = ParseSeries(tok+4,start,proxy,proxy_next,play_length); /* skip over "ser-" */
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_next,1);
      solving_impose_starter(proxy_next,Black);
    }
  }

  else if (token_starts_with("phser-r",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+7, /* skip over phser-r */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_next);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
      {
        help_branch_insert_check_zigzag(proxy);
        solving_impose_starter(proxy_next,White);
      }
      else
        result = 0;
    }
  }

  else if (token_starts_with("phser-s",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+7, /* skip over phser-s */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_next),1);
      help_branch_insert_check_zigzag(proxy);
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("phser-",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+6, /* skip over phser- */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_next,1);
      help_branch_insert_check_zigzag(proxy);
      solving_impose_starter(proxy_next,Black);
    }
  }

  else if (token_starts_with("pser-hs",tok))
  {
    tok = ParseGoal(tok+7,start,proxy_next); /* skip over "ser-hs" */
    if (tok!=0)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const series = alloc_help_branch(length,min_length);
        slice_index const help_proxy = alloc_proxy_slice();
        slice_index const help = alloc_help_branch(1,1);
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_next);
        link_to_branch(help_proxy,help);
        help_branch_set_end_forced(help_proxy,defense_branch,1);
        help_branch_set_end(series,help_proxy,1);
        link_to_branch(proxy,series);
        help_branch_insert_check_zigzag(proxy);
        solving_impose_starter(proxy_next,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (token_starts_with("pser-h",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+6, /* skip over pser-h */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const to_goal = SLICE_NEXT1(proxy_next);
      slice_index const nested = alloc_help_branch(1,1);
      help_branch_set_end_goal(nested,proxy_next,1);
      help_branch_set_end(proxy,nested,1);
      help_branch_insert_check_zigzag(proxy);
      if (SLICE_TYPE(to_goal)==STGoalReachedTester
          && SLICE_U(to_goal).goal_handler.goal.type==goal_proofgame)
        solving_impose_starter(proxy_next,White);
      else
        solving_impose_starter(proxy_next,Black);
    }
  }

  else if (token_starts_with("pser-r",tok))
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok+6, /* skip over pser-r */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_next);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      battle_branch_insert_defense_check_zigzag(proxy);
      select_output_mode(proxy,output_mode_line);
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("pser-s",tok))
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok+6, /* skip over pser-s */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      battle_branch_insert_direct_end_of_branch(proxy,
                                                MakeEndOfSelfPlay(proxy_next));
      solving_impose_starter(proxy_next,Black);
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (token_starts_with("pser-",tok))
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok+5, /* skip over pser- */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_next);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (token_starts_with("reci-h",tok))
  {
    char * const tok2 = ParseReciEnd(tok+6, /* skip over "reci-h" */
                                     start,proxy_next);
    if (tok2!=0 && SLICE_NEXT1(proxy_next)!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseHelpLength(tok2,&length,&min_length,play_length);

      if (length==1)
      {
        /* at least 2 half moves requried for a reciprocal stipulation */
        output_plaintext_input_error_message(StipNotSupported,0);
        result = 0;
      }

      if (result!=0)
      {
        if (length==2)
        {
          pipe_link(proxy,SLICE_NEXT1(proxy_next));
          dealloc_slice(proxy_next);
        }
        else
        {
          stip_length_type const min_length2 = (min_length<2
                                                ? min_length
                                                : min_length-2);
          slice_index const branch = alloc_help_branch(length-2,min_length2);
          help_branch_set_end(branch,proxy_next,1);
          attach_help_branch(length,proxy,branch);
        }

        solving_impose_starter(proxy_next,Black);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (token_starts_with("dia",tok))
  {
    result = ParseHelpDia(tok,start,proxy,proxy_next,play_length);
    if (result!=0)
      solving_impose_starter(proxy,White);
  }
  else if (token_starts_with("a=>b",tok))
  {
    result = ParseHelpDia(tok,start,proxy,proxy_next,play_length);
    if (result!=0)
      solving_impose_starter(proxy,Black);
  }

  else if (token_starts_with("hs",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+2, /* skip over "hs" */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_next),1);
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("hr",tok))
  {
    boolean const shorten = true;
    result = ParseHelp(tok+2, /* skip over "hr" */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_next);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
        solving_impose_starter(proxy_next,White);
      else
        result = 0;
    }
  }

  else if (token_starts_with("h",tok))
  {
    boolean const shorten = false;
    result = ParseHelp(tok+1, /* skip over "h" */
                       start,
                       proxy,proxy_next,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_next,1);
      solving_impose_starter(proxy_next,Black);
    }
  }

  else if (token_starts_with("semi-r",tok))
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok+6, /* skip over "semi-r" */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      battle_branch_insert_end_of_branch_forced(proxy,
                                                MakeSemireflexBranch(proxy_next));
      select_output_mode(proxy,output_mode_tree);
      solving_impose_starter(proxy_next,White);
    }
  }

  else if (token_starts_with("s",tok))
  {
    boolean const ends_on_defense = true;
    result = ParseBattle(tok+1, /* skip over 's' */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_self_end_of_branch_goal(proxy,proxy_next);
    }
  }

  else if (token_starts_with("r",tok))
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok+1, /* skip over 'r' */
                         start,
                         proxy,proxy_next,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_next);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      select_output_mode(proxy,output_mode_tree);
      solving_impose_starter(proxy_next,White);
    }
  }

  else
  {
    boolean const ends_on_defense = false;
    result = ParseBattle(tok,start,proxy,proxy_next,play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_next);
    }
  }

  if (result==0)
    dealloc_slices(proxy_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

char *ParseStip(char *tok, slice_index start)
{
  slice_index const root_slice_hook = input_find_stipulation(start);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  stipulation_reset();

  strcpy(AlphaStip,tok);
  if (ParsePlay(tok,start,root_slice_hook,play_length_minimum))
  {
    if (SLICE_NEXT1(root_slice_hook)!=no_slice
        && ActStip[0]=='\0')
      strcpy(ActStip, AlphaStip);
  }

  tok = ReadNextTokStr();

  /* signal to our caller that the stipulation has changed */
  SLICE_STARTER(root_slice_hook) = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
