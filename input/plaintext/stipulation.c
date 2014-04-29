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
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/not.h"
#include "stipulation/proxy.h"

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
      tok = ParseGoal(tok+1,proxy_to_reci);
      if (tok!=0)
      {
        if (tok==closingParenPos)
        {
          slice_index const proxy_to_nonreci = alloc_proxy_slice();
          result = ParseGoal(tok+1,proxy_to_nonreci);
          if (result!=NULL)
          {
            slice_index const nonreci = slices[proxy_to_nonreci].next1;
            slices[nonreci].starter = Black;
            alloc_reci_end(proxy_nonreci,proxy_reci,
                           proxy_to_nonreci,proxy_to_reci);
          }
        }
        else
          IoErrorMsg(UnrecStip, 0);
      }
    }
    else
      IoErrorMsg(UnrecStip, 0);
  }
  else
  {
    slice_index const proxy_to_nonreci = alloc_proxy_slice();
    result = ParseGoal(tok,proxy_to_nonreci);
    if (result!=NULL)
    {
      slice_index const nonreci_testing = slices[proxy_to_nonreci].next1;
      slice_index const nonreci_tester = slices[nonreci_testing].next1;
      slice_index const proxy_to_reci = stip_deep_copy(proxy_to_nonreci);
      alloc_reci_end(proxy_nonreci,proxy_reci,
                     proxy_to_nonreci,proxy_to_reci);
      slices[nonreci_tester].starter = Black;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseReciEnd(char *tok, slice_index proxy)
{
  slice_index op1;
  slice_index op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  op1 = alloc_proxy_slice();
  op2 = alloc_proxy_slice();

  tok = ParseReciGoal(tok,op1,op2);
  if (slices[op1].next1!=no_slice && slices[op2].next1!=no_slice)
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
    IoErrorMsg(WrongInt,0);
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
      IoErrorMsg(WrongInt,0);
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
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length,
                         boolean ends_on_defense)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",ends_on_defense);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
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
  branch_insert_slices(si,&prototype,1);
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
                       slice_index proxy,
                       slice_index proxy_goal,
                       play_length_type play_length,
                       boolean shorten)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",shorten);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
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
                          slice_index proxy,
                          play_length_type play_length)
{
  char *result;
  slice_index proxy_to_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  proxy_to_goal = alloc_proxy_slice();
  result = ParseGoal(tok,proxy_to_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseHelpLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      link_to_branch(proxy,alloc_help_branch(length,min_length));
      select_output_mode(proxy,output_mode_line);
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
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
      IoErrorMsg(WrongInt,0);
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
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
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
                       slice_index root_slice_hook,
                       slice_index proxy,
                       play_length_type play_length)
{
  /* seriesmovers with introductory moves */
  char *arrowpos = strstr(tok,"->");
  char *result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (arrowpos!=0)
  {
    char *end;
    unsigned long const intro_len= strtoul(tok,&end,10);
    if (intro_len<1 || tok==end || end!=arrowpos)
      IoErrorMsg(WrongInt, 0);
    else
    {
      slice_index const proxy_next = alloc_proxy_slice();
      result = ParsePlay(arrowpos+2,root_slice_hook,proxy_next,play_length);
      if (result!=0 && slices[proxy_next].next1!=no_slice)
      {
        /* >=1 move of starting side required */
        slice_index const branch = alloc_series_branch(2*intro_len-1,1);
        help_branch_set_end(branch,proxy_next,1);
        link_to_branch(proxy,branch);
        select_output_mode(proxy,output_mode_line);
      }
      else
        dealloc_slices(proxy_next);
    }
  }

  else if (strncmp("exact-", tok, 6) == 0)
    result = ParsePlay(tok+6,root_slice_hook,proxy,play_length_exact);

  else if (strncmp("ser-reci-h",tok,10) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();

    /* skip over "ser-reci-h" */
    tok = ParseReciEnd(tok+10,proxy_next);
    if (tok!=0 && slices[proxy_next].next1!=no_slice)
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
    else
      dealloc_slices(proxy_next);
  }

  else if (strncmp("ser-hs",tok,6)==0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    tok = ParseGoal(tok+6,proxy_to_goal); /* skip over "ser-hs" */
    if (tok==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result==0)
        dealloc_slices(proxy_to_goal);
      else
      {
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_to_goal);

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

        solving_impose_starter(proxy_to_goal,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("ser-h",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-h" */
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const help = alloc_help_branch(1,1);
      help_branch_set_end_goal(help,proxy_to_goal,1);
      help_branch_set_end(proxy,help,1);

      {
        slice_index const next = slices[proxy_to_goal].next1;
        assert(next!=no_slice);
        if (slices[next].type==STGoalReachedTester
            && slices[next].u.goal_handler.goal.type==goal_proofgame)
          solving_impose_starter(proxy_to_goal,White);
        else
          solving_impose_starter(proxy_to_goal,Black);
      }
    }
  }

  else if (strncmp("ser-s",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-s" */
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("ser-r",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-r" */
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      series_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi));
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("ser-",tok,4) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+4,proxy,proxy_to_goal,play_length); /* skip over "ser-" */
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      solving_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("phser-r",tok,7) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+7, /* skip over phser-r */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
      {
        help_branch_insert_check_zigzag(proxy);
        solving_impose_starter(proxy_to_goal,White);
      }
      else
        result = 0;
    }
  }

  else if (strncmp("phser-s",tok,7) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+7, /* skip over phser-s */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      help_branch_insert_check_zigzag(proxy);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("phser-",tok,6) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+6, /* skip over phser- */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      help_branch_insert_check_zigzag(proxy);
      solving_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("pser-hs",tok,7) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    tok = ParseGoal(tok+7,proxy_to_goal); /* skip over "ser-hs" */
    if (tok==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result==0)
        dealloc_slices(proxy_to_goal);
      else
      {
        slice_index const series = alloc_help_branch(length,min_length);
        slice_index const help_proxy = alloc_proxy_slice();
        slice_index const help = alloc_help_branch(1,1);
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_to_goal);
        link_to_branch(help_proxy,help);
        help_branch_set_end_forced(help_proxy,defense_branch,1);
        help_branch_set_end(series,help_proxy,1);
        link_to_branch(proxy,series);
        help_branch_insert_check_zigzag(proxy);
        solving_impose_starter(proxy_to_goal,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("pser-h",tok,6) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+6, /* skip over pser-h */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const to_goal = slices[proxy_to_goal].next1;
      slice_index const nested = alloc_help_branch(1,1);
      help_branch_set_end_goal(nested,proxy_to_goal,1);
      help_branch_set_end(proxy,nested,1);
      help_branch_insert_check_zigzag(proxy);
      if (slices[to_goal].type==STGoalReachedTester
          && slices[to_goal].u.goal_handler.goal.type==goal_proofgame)
        solving_impose_starter(proxy_to_goal,White);
      else
        solving_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("pser-r",tok,6) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over pser-r */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      battle_branch_insert_defense_check_zigzag(proxy);
      select_output_mode(proxy,output_mode_line);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("pser-s",tok,6) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over pser-s */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      battle_branch_insert_direct_end_of_branch(proxy,
                                                MakeEndOfSelfPlay(proxy_to_goal));
      solving_impose_starter(proxy_to_goal,Black);
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (strncmp("pser-",tok,5) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+5, /* skip over pser- */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_to_goal);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (strncmp("reci-h",tok,6) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    char * const tok2 = ParseReciEnd(tok+6, /* skip over "reci-h" */
                                     proxy_next);
    if (tok2!=0 && slices[proxy_next].next1!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseHelpLength(tok2,&length,&min_length,play_length);

      if (length==1)
      {
        /* at least 2 half moves requried for a reciprocal stipulation */
        IoErrorMsg(StipNotSupported,0);
        result = 0;
      }

      if (result==0)
        dealloc_slices(proxy_next);
      else
      {
        if (length==2)
        {
          pipe_link(proxy,slices[proxy_next].next1);
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

  else if (strncmp("dia",tok,3)==0)
  {
    result = ParseHelpDia(tok,proxy,play_length);
    if (result!=0)
      solving_impose_starter(proxy,White);
  }
  else if (strncmp("a=>b",tok,4)==0)
  {
    result = ParseHelpDia(tok,proxy,play_length);
    if (result!=0)
      solving_impose_starter(proxy,Black);
  }

  else if (strncmp("hs",tok,2)==0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+2, /* skip over "hs" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("hr",tok,2)==0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+2, /* skip over "hr" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
        solving_impose_starter(proxy_to_goal,White);
      else
        result = 0;
    }
  }

  else if (*tok=='h')
  {
    boolean const shorten = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+1, /* skip over "h" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      solving_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("semi-r",tok,6)==0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over "semi-r" */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      battle_branch_insert_end_of_branch_forced(proxy,
                                                MakeSemireflexBranch(proxy_to_goal));
      select_output_mode(proxy,output_mode_tree);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else if (*tok=='s')
  {
    boolean const ends_on_defense = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+1, /* skip over 's' */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_self_end_of_branch_goal(proxy,proxy_to_goal);
    }
  }

  else if (*tok=='r')
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+1, /* skip over 'r' */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      select_output_mode(proxy,output_mode_tree);
      solving_impose_starter(proxy_to_goal,White);
    }
  }

  else
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok,proxy,proxy_to_goal,play_length,ends_on_defense);
    if (result==0)
      dealloc_slices(proxy_to_goal);
    else
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_to_goal);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

char *ParseStip(slice_index root_slice_hook)
{
  char *tok = ReadNextTokStr();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  CLRFLAGMASK(some_pieces_flags,PieceIdMask);

  strcpy(AlphaStip,tok);
  if (ParsePlay(tok,root_slice_hook,root_slice_hook,play_length_minimum))
  {
    if (slices[root_slice_hook].next1!=no_slice
        && ActStip[0]=='\0')
      strcpy(ActStip, AlphaStip);
  }

  tok = ReadNextTokStr();

  /* signal to our caller that the stipulation has changed */
  slices[root_slice_hook].starter = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
