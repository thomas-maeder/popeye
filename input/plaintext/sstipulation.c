#include "input/plaintext/sstipulation.h"
#include "input/plaintext/stipulation.h"
#include "input/plaintext/goal.h"
#include "input/plaintext/token.h"
#include "output/output.h"
#include "output/mode.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "conditions/check_zigzag.h"
#include "position/pieceid.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/not.h"
#include "stipulation/proxy.h"
#include "solving/play_suppressor.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Advance to next1 token while parsing a structured stipulation
 * @param tok current position in current token
 * @return tok, if we are within the current token; next1 token otherwise
 */
static char *ParseStructuredStip_skip_whitespace(char *tok)
{
  if (tok[0]==0)
  {
    tok = ReadNextTokStr();
    TraceValue("%s",tok);
    TraceEOL();
  }

  return tok;
}

typedef enum
{
  expression_type_goal,
  expression_type_attack, /* this includes help and series */
  expression_type_defense
} expression_type;

/* Parse a stipulation expression
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_expression(char *tok,
                                            slice_index start,
                                            slice_index proxy,
                                            expression_type *type,
                                            unsigned int level);

/* Parse an stipulation operand
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of operand; no_slice if operand couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operand(char *tok,
                                         slice_index start,
                                         slice_index proxy,
                                         expression_type *type,
                                         unsigned int level);

/* Parse a parenthesised stipulation expression
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *
ParseStructuredStip_parenthesised_expression(char *tok,
                                             slice_index start,
                                             slice_index proxy,
                                             expression_type *type,
                                             unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_expression(tok+1,start,proxy,type,level);

  if (tok!=0)
  {
    /* allow space before closing parenthesis */
    tok = ParseStructuredStip_skip_whitespace(tok);

    if (tok[0]==')')
      ++tok;
    else
      pipe_set_successor(proxy,no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a not operator
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if operator couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_not(char *tok,
                                     slice_index start,
                                     slice_index proxy,
                                     expression_type *type,
                                     unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_operand(tok+1,start,proxy,type,level);
  if (tok!=0)
  {
    if (*type==expression_type_goal)
    {
      slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                   proxy,
                                                   stip_traversal_context_intro);
      assert(tester!=no_slice);
      pipe_append(SLICE_NEXT2(tester),alloc_not_slice());
      SLICE_U(tester).goal_handler.goal.type = goal_negated;
    }
    else
      pipe_append(proxy,alloc_not_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a move inversion
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if operator couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_move_inversion(char *tok,
                                                slice_index start,
                                                slice_index proxy,
                                                expression_type *type,
                                                unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_operand(tok+1,start,proxy,type,level);

  {
    slice_index const operand = SLICE_NEXT1(proxy);
    if (tok!=0 && operand!=no_slice)
    {
      slice_index const prototype = alloc_move_inverter_slice();
      slice_insertion_insert(proxy,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* extend expression_type with nested_branch_type_forced */
typedef enum
{
  nested_branch_type_goal,
  nested_branch_type_attack,
  nested_branch_type_defense,
  nested_branch_type_forced
} nested_branch_type;

/* Parse the length indication of a branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_length(char *tok,
                                               stip_length_type *min_length,
                                               stip_length_type *max_length)
{
  char *end;
  unsigned long length_long = strtoul(tok,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  if (end==tok || length_long==0 || length_long>UINT_MAX)
    tok = 0;
  else
  {
    tok = ParseStructuredStip_skip_whitespace(end);
    if (tok[0]==':')
    {
      *min_length = (stip_length_type)length_long;
      tok = ParseStructuredStip_skip_whitespace(tok+1);
      length_long = strtoul(tok,&end,10);
      if (end==tok || length_long==0 || length_long>UINT_MAX)
        tok = 0;
      else
      {
        *max_length = (stip_length_type)length_long;
        tok = end;
      }
    }
    else
    {
      *min_length = 0;
      *max_length = (stip_length_type)length_long;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make a "da branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @param level nesting level of the operand (0 means top level)
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_d(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length += 1;
  min_length += 1;

  if (min_length>=max_length)
    min_length = max_length-1;

  result = alloc_battle_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a nested branch
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy_operand identifier of proxy slice where to attach the branch
 * @param type address of object where to return the operand type of the branch
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_nested_branch(char *tok,
                                               slice_index start,
                                               slice_index proxy_operand,
                                               nested_branch_type *type,
                                               unsigned int level)
{
  expression_type * etype = (expression_type *)type;
  boolean is_forced = false;
  boolean is_suppressed = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy_operand);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  if (tok[0]=='>')
  {
    is_forced = true;
    ++tok;
  }

  if (tok[0]=='/')
  {
    is_suppressed = true;
    ++tok;
  }

  tok = ParseStructuredStip_expression(tok,start,proxy_operand,etype,level+1);

  if (tok!=0)
  {
    if (is_suppressed)
    {
      slice_index const prototype = alloc_play_suppressor_slice();
      slice_insertion_insert(proxy_operand,&prototype,1);
      if (branch_find_slice(STPlaySuppressor,
                            proxy_operand,
                            stip_traversal_context_intro)
          ==no_slice)
        pipe_append(proxy_operand,alloc_play_suppressor_slice());
    }

    if (is_forced)
    {
      if (*etype==expression_type_attack)
        *type = nested_branch_type_forced;
      else
        tok = 0;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an "a operand"
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param identifier of entry slice of "ad branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_d_operand(char *tok,
                                                  slice_index start,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;
      tok = ParseStructuredStip_nested_branch(tok+1,start,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            battle_branch_insert_self_end_of_branch_goal(branch,proxy_operand);
            break;

          case nested_branch_type_attack:
          case nested_branch_type_defense:
            battle_branch_insert_self_end_of_branch(branch,proxy_operand);
            break;

          case nested_branch_type_forced:
            tok = 0;
            break;

          default:
            assert(0);
            break;
        }
      }
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,start,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        if (nested_type==expression_type_goal)
          battle_branch_insert_attack_goal_constraint(branch,proxy_operand);
        else
          battle_branch_insert_attack_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an "a operand"
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param branch identifier of entry slice of "ad branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_a_operand(char *tok,
                                                  slice_index start,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,start,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            battle_branch_insert_direct_end_of_branch_goal(branch,proxy_operand);
            break;

          case nested_branch_type_attack:
          case nested_branch_type_forced:
            battle_branch_insert_end_of_branch_forced(branch,proxy_operand);
            break;

          case nested_branch_type_defense:
            battle_branch_insert_direct_end_of_branch(branch,proxy_operand);
            break;

          default:
            assert(0);
            break;
        }
      }
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,start,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        battle_branch_insert_defense_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a "da branch"
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_d(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index start,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_d(min_length,
                                                                 max_length);
    link_to_branch(proxy,battle_branch_make_postkeyplay(branch));

    tok = ParseStructuredStip_branch_d_operand(tok,start,proxy,level);
    if (tok!=0 && tok[0]=='a')
    {
      tok = ParseStructuredStip_branch_a_operand(tok+1,start,proxy,level);
      if (tok!=0 && level==0)
        select_output_mode(proxy,output_mode_tree);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "s branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_s(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length *= 2;
  max_length -= 1;

  if (min_length==0)
    min_length = 1;
  else
  {
    min_length *= 2;
    min_length -= 1;
    if (min_length>max_length)
      min_length = max_length;
  }

  result = alloc_series_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a "s operand"
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param identifier of entry slice of "s branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_s_operand(char *tok,
                                                  slice_index start,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,start,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            help_branch_set_end_goal(branch,proxy_operand,1);
            break;

          case nested_branch_type_forced:
            help_branch_set_end_forced(branch,proxy_operand,1);
            break;

          case nested_branch_type_attack:
          case nested_branch_type_defense:
            help_branch_set_end(branch,proxy_operand,1);
            break;

          default:
            assert(0);
            break;
        }
      }
      else
        tok = 0;
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,start,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        if (nested_type==expression_type_goal)
          series_branch_insert_goal_constraint(branch,proxy_operand);
        else
          series_branch_insert_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a series branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_s(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index start,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_s(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_s_operand(tok,start,proxy,level);
    if (tok!=0 && level==0)
      select_output_mode(proxy,output_mode_line);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "ad branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_a(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  min_length += 1;

  if (min_length>=max_length)
    min_length = max_length-1;

  result = alloc_battle_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse an "ad" branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_a(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index start,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    boolean parry = false;
    slice_index const branch = ParseStructuredStip_make_branch_a(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_a_operand(tok,start,proxy,level);

    if (tok!=0 && token_starts_with("?+?",tok))
    {
      tok += 3;
      parry = true;
    }

    if (tok!=0 && tok[0]=='d')
    {
      tok = ParseStructuredStip_branch_d_operand(tok+1,start,proxy,level);
      if (parry)
        battle_branch_insert_defense_check_zigzag(proxy);
      if (level==0 && tok!=0)
        select_output_mode(proxy, parry ? output_mode_line : output_mode_tree);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "hh branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_h(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  if (min_length==0)
    min_length = max_length%2;
  else if (min_length>max_length)
    min_length = max_length;

  result = alloc_help_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a "h operand"
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param branch identifier of entry slice of "hh branch"
 * @param parity indicates after which help move of the branch to insert
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_h_operand(char *tok,
                                                  slice_index start,
                                                  slice_index branch,
                                                  unsigned int parity,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",parity);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,start,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            help_branch_set_end_goal(branch,proxy_operand,parity);
            break;

          case nested_branch_type_forced:
            help_branch_set_end_forced(branch,proxy_operand,parity);
            break;

          case nested_branch_type_defense:
          case nested_branch_type_attack:
            help_branch_set_end(branch,proxy_operand,parity);
            break;

          default:
            assert(0);
            break;
        }
      }
      else
        tok = 0;
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,start,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}'
          && help_branch_insert_constraint(branch,proxy_operand,parity))
        ++tok;
      else
        tok = 0;
    }
    else
      break;
    TraceValue("%s",tok);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a help branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_h(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index start,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    boolean parry = false;
    slice_index const branch = ParseStructuredStip_make_branch_h(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_h_operand(tok,start,proxy,max_length,level);

    if (tok!=0 && token_starts_with("?+?",tok))
    {
      tok += 3;
      parry = true;
    }

    if (tok!=0 && tok[0]=='h')
    {
      tok = ParseStructuredStip_branch_h_operand(tok+1,start,proxy,max_length+1,level);
      if (parry)
        help_branch_insert_check_zigzag(proxy);
      if (level==0)
        select_output_mode(proxy,output_mode_line);
    }
    else
      tok = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation branch
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch(char *tok,
                                        slice_index start,
                                        slice_index proxy,
                                        expression_type *type,
                                        unsigned int level)
{
  stip_length_type min_length = 0;
  stip_length_type max_length = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_branch_length(tok,&min_length,&max_length);

  switch (tolower(tok[0]))
  {
    case 'd':
      *type = expression_type_defense;
      tok = ParseStructuredStip_branch_d(tok+1,min_length,max_length,start,proxy,level);
      break;

    case 's':
      *type = expression_type_attack;
      tok = ParseStructuredStip_branch_s(tok+1,min_length,max_length,start,proxy,level);
      break;

    case 'a':
      *type = expression_type_attack;
      tok = ParseStructuredStip_branch_a(tok+1,min_length,max_length,start,proxy,level);
      break;

    case 'h':
      *type = expression_type_attack;
      tok = ParseStructuredStip_branch_h(tok+1,min_length,max_length,start,proxy,level);
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an stipulation operand
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of operand; no_slice if operand couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operand(char *tok,
                                         slice_index start,
                                         slice_index proxy,
                                         expression_type *type,
                                         unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='(')
    tok = ParseStructuredStip_parenthesised_expression(tok,start,proxy,type,level);
  else if (tok[0]=='!')
    /* !d# - white at the move does *not* deliver mate */
    tok = ParseStructuredStip_not(tok,start,proxy,type,level);
  else if (tok[0]=='-')
    /* -3hh# - h#2 by the non-starter */
    tok = ParseStructuredStip_move_inversion(tok,start,proxy,type,level);
  else if (isdigit(tok[0]) && tok[0]!='0')
    /* e.g. 3ad# for a #2 - but not 00 (castling goal!)*/
    tok = ParseStructuredStip_branch(tok,start,proxy,type,level);
  else
  {
    /* e.g. d= for a =1 */
    *type = expression_type_goal;
    tok = ParseGoal(tok,start,proxy);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation operator
 * @param tok input token
 * @param result type of operator; no_slice_type if operand couldn't
 *               be parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operator(char *tok, slice_type *result)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='&')
  {
    ++tok;
    *result = STAnd;
  }
  else if (tok[0]=='|')
  {
    ++tok;
    *result = STOr;
  }
  else
    *result = no_slice_type;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation expression
 * @param tok input token
 * @param start index of entry into solving machinery
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_expression(char *tok,
                                            slice_index start,
                                            slice_index proxy,
                                            expression_type *type,
                                            unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const operand1 = alloc_proxy_slice();
    tok = ParseStructuredStip_operand(tok,start,operand1,type,level);
    if (tok!=0 && SLICE_NEXT1(operand1)!=no_slice)
    {
      slice_type operator_type;
      tok = ParseStructuredStip_operator(tok,&operator_type);
      if (tok!=0 && operator_type!=no_slice_type)
      {
        if (*type==expression_type_defense)
          tok = 0;
        else
        {
          slice_index const operand2 = alloc_proxy_slice();
          expression_type type2;
          tok = ParseStructuredStip_expression(tok,start,operand2,&type2,level);
          if (tok!=0 && SLICE_NEXT1(operand2)!=no_slice)
          {
            if (*type==type2)
              switch (operator_type)
              {
                case STAnd:
                {
                  slice_index const and = alloc_and_slice(operand1,operand2);
                  pipe_link(proxy,and);
                  break;
                }

                case STOr:
                {
                  slice_index const or = alloc_or_slice(operand1,operand2);
                  pipe_link(proxy,or);
                  break;
                }

                default:
                  assert(0);
                  break;
              }
            else
              tok = 0;
          }
        }
      }
      else
      {
        if (SLICE_PREV(SLICE_NEXT1(operand1))==operand1)
          pipe_link(proxy,SLICE_NEXT1(operand1));
        else
          pipe_set_successor(proxy,SLICE_NEXT1(operand1));

        dealloc_slice(operand1);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse starter of stipulation
 * @param tok input token
 * @return starter; no_side if starter couldn't be parsed
 */
static Side ParseStructuredStip_starter(char *tok)
{
  Side result = no_side;
  Side ps;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* We don't make any unsafe assumptions here; PieSpec enumerators
   * are initialised in terms of nr_sides */
  ps = GetUniqIndex(nr_sides,ColourTab,tok);
  if (ps>nr_sides)
    output_plaintext_input_error_message(PieSpecNotUniq,0);
  else if (ps<nr_sides)
    result = ps;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a structured stipulation (keyword sstipulation)
 * @param start index of entry into solving machinery
 * @return remainder of input token; 0 if parsing failed
 */
char *ParseStructuredStip(char *tok, slice_index start)
{
  slice_index const root_slice_hook = alloc_proxy_slice();
  Side starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  stipulation_reset();

  starter = ParseStructuredStip_starter(tok);
  if (starter!=no_side)
  {
    expression_type type;
    tok = ReadNextTokStr();
    tok = ParseStructuredStip_expression(tok,start,root_slice_hook,&type,0);
    if (tok==0)
      tok = ReadNextTokStr();
    else if (SLICE_NEXT1(root_slice_hook)!=no_slice)
    {
      solving_impose_starter(root_slice_hook,starter);
      move_effect_journal_do_insert_sstipulation(start,root_slice_hook);
    }
  }

  /* signal to our caller that the stipulation has changed */
  SLICE_STARTER(root_slice_hook) = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Remember the original stipulation for restoration after the stipulation has
 * been modified by a twinning
 * @param start input position at start of parsing the stipulation
 * @param stipulation identifies the entry slice into the stipulation
 */
void move_effect_journal_do_insert_sstipulation(slice_index start,
                                                  slice_index stipulation)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_input_sstipulation,move_effect_reason_diagram_setup);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",stipulation);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_input_sstipulation,
                                       &move_effect_journal_undo_insert_stipulation,
                                       0);
  slice_instrument_with_stipulation(start,stipulation);

  entry->u.input_stipulation.start_index = start;
  entry->u.input_stipulation.stipulation = stipulation;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
