#include "input/plaintext/twin.h"
#include "input/plaintext/token.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "input/plaintext/sstipulation.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/geometry/square.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/twinning.h"
#include "output/plaintext/protocol.h"
#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/total_invisible.h"
#include "position/underworld.h"
#include "position/effects/piece_creation.h"
#include "position/effects/board_transformation.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "position/effects/walk_change.h"
#include "position/effects/piece_exchange.h"
#include "position/effects/total_side_exchange.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/stipulation.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/modifier.h"
#include "solving/goals/prerequisite_guards.h"
#include "solving/machinery/twin.h"
#include "solving/pipe.h"
#include "solving/duplex.h"
#include "solving/proofgames.h"
#include "solving/zeroposition.h"
#include "utilities/table.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/heartbeat.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static void TwinResetPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_twinning);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseTwinningMove(void)
{
  square sq1;
  square sq2;

  {
    char *tok = ReadNextTokStr();
    tok = ParseSquare(tok,&sq1);
  }

  {
    char *tok = ReadNextTokStr();
    tok = ParseSquare(tok,&sq2);
  }

  if (sq1==initsquare || sq2==initsquare)
    output_plaintext_error_message(WrongSquareList);
  else if (is_square_empty(sq1))
  {
    WriteSquare(&output_plaintext_engine,stderr,sq1);
    output_plaintext_error_message(NothingToRemove);
  }
  else
  {
    if (!is_square_empty(sq2))
      move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,sq2);

    move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                          sq1,sq2);
  }

  return ReadNextTokStr();
}

static char *ParseTwinningExchange(void)
{
  square sq1;
  square sq2;

  {
    char *tok = ReadNextTokStr();
    tok = ParseSquare(tok,&sq1);
  }

  {
    char *tok = ReadNextTokStr();
    tok = ParseSquare(tok,&sq2);
  }

  if (sq1==initsquare || sq2==initsquare)
    output_plaintext_error_message(WrongSquareList);
  else if (is_square_empty(sq1))
  {
    WriteSquare(&output_plaintext_engine,stderr,sq1);
    output_plaintext_error_message(NothingToRemove);
  }
  else if (is_square_empty(sq2))
  {
    WriteSquare(&output_plaintext_engine,stderr,sq2);
    output_plaintext_error_message(NothingToRemove);
  }
  else
    move_effect_journal_do_piece_exchange(move_effect_reason_diagram_setup,
                                          sq1,sq2);

  return ReadNextTokStr();
}

static SquareTransformation detect_rotation(char const tok[])
{
  if (strcmp(tok,"90")==0)
    return rot90;
  else if (strcmp(tok,"180")==0)
    return rot180;
  else if (strcmp(tok,"270")==0)
    return rot270;
  else
    return nr_square_transformation;
}

static char *ParseTwinningRotate(void)
{
  char *tok = ReadNextTokStr();
  SquareTransformation const rotation = detect_rotation(tok);

  if (rotation==nr_square_transformation)
    output_plaintext_input_error_message(UnrecRotMirr);
  else
    move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                rotation);

  return ReadNextTokStr();
}

static char *ParseTwinningMirror(void)
{
  char *tok = ReadNextTokStr();
  TwinningMirrorType indexx = GetUniqIndex(TwinningMirrorCount,TwinningMirrorTab,tok);

  if (indexx>TwinningMirrorCount)
    output_plaintext_input_error_message(OptNotUniq);
  else
  {
    switch (indexx)
    {
      case TwinningMirrora1h1:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1h1);
        break;

      case TwinningMirrora1a8:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1a8);
        break;

      case TwinningMirrora1h8:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1h8);
        break;

      case TwinningMirrora8h1:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra8h1);
        break;

      default:
        output_plaintext_input_error_message(UnrecRotMirr);
        break;
    }
  }

  return ReadNextTokStr();
}

static void do_shift(square from, square to)
{
  echiquier board = { 0 };
  Flags spec[maxsquare+5] = { 0 };
  int const vector = to-from;

  square const *bnp;

  for (bnp = boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp))
    {
      square const to = *bnp + vector;
      board[to] = get_walk_of_piece_on_square(*bnp);
      spec[to] = being_solved.spec[*bnp];
      set_walk_of_piece_on_square(*bnp, Empty);
      CLEARFL(being_solved.spec[*bnp]);
    }

  for (bnp = boardnum; *bnp; bnp++)
    if (board[*bnp]!=Empty)
    {
      set_walk_of_piece_on_square(*bnp, board[*bnp]);
      being_solved.spec[*bnp] = spec[*bnp];
    }
}

/* Execute a twinning that shifts the entire position
 */
static void move_effect_journal_do_twinning_shift(square from, square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_twinning_shift,move_effect_reason_twinning);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  entry->u.twinning_shift.from = from;
  entry->u.twinning_shift.to = to;

  do_shift(from,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_twinning_shift(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_shift(entry->u.twinning_shift.to,entry->u.twinning_shift.from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseTwinningShift(void)
{
  char *tok = ReadNextTokStr();
  square sq1;
  tok = ParseSquare(tok,&sq1);

  if (sq1==initsquare)
  {
    output_plaintext_error_message(WrongSquareList);
    return tok;
  }
  else
  {
    char *tok = ReadNextTokStr();
    square sq2;
    tok = ParseSquare(tok,&sq2);

    if (sq2==initsquare)
    {
      output_plaintext_error_message(WrongSquareList);
      return tok;
    }
    else
    {
      if (twin_twinning_shift_validate(sq1,sq2))
      {
        /* we don't redo shift twinnings */
        move_effect_journal_set_effect_doers(move_effect_twinning_shift,
                                             &undo_twinning_shift,
                                             0);

        move_effect_journal_do_twinning_shift(sq1,sq2);
      }
      else
        output_plaintext_error_message(PieceOutside);

      return ReadNextTokStr();
    }
  }
}

static boolean HandleRemovalSquare(square s, void *dummy)
{
  if (get_walk_of_piece_on_square(s)>=King)
  {
    move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,s);
    return true;
  }
  else
  {
    WriteSquare(&output_plaintext_engine,stderr,s);
    output_plaintext_error_message(NothingToRemove);
    return false;
  }
}

static char *ParseTwinningRemove(void)
{
  char * const squares_tok = ReadNextTokStr();
  char *tok = ParseSquareList(squares_tok,HandleRemovalSquare,0);
  if (tok==squares_tok)
    output_plaintext_input_error_message(MissngSquareList);
  else if (tok==0)
    /* HandleRemovalSquare dealt with the problem */;
  else if (*tok!=0)
    output_plaintext_error_message(WrongSquareList);

  return ReadNextTokStr();
}

static char *ParseTwinningPolish(void)
{
  position_total_side_exchange_initialise();
  move_effect_journal_do_total_side_exchange(move_effect_reason_twinning);
  return ReadNextTokStr();
}

static void do_substitute_all(piece_walk_type from, piece_walk_type to)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceWalk(from);
  TraceWalk(to);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (get_walk_of_piece_on_square(*bnp)==from)
      move_effect_journal_do_walk_change(move_effect_reason_twinning,*bnp,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Execute a twinning that substitutes a walk for another
 */
static void move_effect_journal_do_twinning_substitute(piece_walk_type from,
                                                       piece_walk_type to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_twinning_substitute,move_effect_reason_twinning);

  TraceFunctionEntry(__func__);
  TraceWalk(from);
  TraceWalk(to);
  TraceFunctionParamListEnd();

  entry->u.piece_walk_change.from = from;
  entry->u.piece_walk_change.to = to;
  entry->u.piece_walk_change.on = initsquare;

  do_substitute_all(from,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_twinning_substitute(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* nothing */

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseTwinningSubstitute(void)
{
  char *tok = ReadNextTokStr();

  piece_walk_type p_old;
  tok = ParsePieceWalkToken(tok,&p_old);

  if (p_old==nr_piece_walks)
    output_plaintext_input_error_message(WrongPieceName);
  else
  {
    piece_walk_type p_new;
    tok = ParsePieceWalkToken(tok,&p_new);

    if (p_new==nr_piece_walks)
      output_plaintext_input_error_message(WrongPieceName);
    else
    {
      /* we don't redo substitute twinnings */
      move_effect_journal_set_effect_doers(move_effect_twinning_substitute,
                                           &undo_twinning_substitute,
                                           0);
      move_effect_journal_do_twinning_substitute(p_old,p_new);
    }
  }

  return tok;
}

static move_effect_journal_index_type find_original_condition(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_diagram_setup+1];
  move_effect_journal_index_type curr;

  for (curr = move_effect_journal_base[ply_diagram_setup]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_input_condition)
      return curr;

  return move_effect_journal_index_null;
}

/* Remember the original condition for restoration after the condition has been
 * modified by a twinning
 * @param start input position at start of parsing the condition
 */
static void move_effect_journal_do_remember_condition(fpos_t start)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_input_condition,
                                                                                    move_effect_reason_diagram_setup);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  entry->u.input_complex.start = start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_input_condition(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_twinning || nbply==ply_diagram_setup);

  InitCond();

  /* restore the original condition (if any) if we are undoing a
   * condition twinning
   */
  if (nbply==ply_twinning)
  {
    move_effect_journal_index_type const idx_cond = find_original_condition();
    if (idx_cond!=move_effect_journal_index_null)
    {
      move_effect_journal_entry_type const * const cond = &move_effect_journal[idx_cond];
      InputStartReplay(cond->u.input_complex.start);
      ParseCond(ReadNextTokStr());
      InputEndReplay();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseTwinning(char *tok,
                           slice_index start,
                           unsigned int twin_number)
{
  TwinningType initial_twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);
  boolean continued = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",twin_number);
  TraceFunctionParamListEnd();

  if (initial_twinning==TwinningContinued)
  {
    continued = true;
    tok = ReadNextTokStr();
  }
  else if (initial_twinning<TwinningCount)
    TwinResetPosition();

  output_notify_twinning(start,twin_regular+twin_number,continued);

  while (tok)
  {
    TwinningType const twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);

    if (twinning>TwinningCount)
    {
      output_plaintext_input_error_message(ComNotUniq);
      tok = ReadNextTokStr();
    }
    else if (twinning==TwinningCount)
      break;
    else
      switch(twinning)
      {
        case TwinningContinued:
        {
          output_plaintext_message(ContinuedFirst);
          tok = ReadNextTokStr();
          break;
        }
        case TwinningMove:
          tok = ParseTwinningMove();
          break;
        case TwinningExchange:
          tok = ParseTwinningExchange();
          break;
        case TwinningRotate:
          tok = ParseTwinningRotate();
          break;
        case TwinningMirror:
          tok = ParseTwinningMirror();
          break;
        case TwinningStip:
        {
          move_effect_journal_do_remove_stipulation(start);

          tok = ReadNextTokStr();
          tok = ParseStip(tok,start);
          {
            slice_index const stipulation_root_hook = input_find_stipulation(start);
            if (stipulation_root_hook==no_slice)
            {
              output_plaintext_input_error_message(NoStipulation);
              tok = 0;
            }
            else
              stipulation_modifiers_notify(start,stipulation_root_hook);
          }
          break;
        }
        case TwinningStructStip:
        {
          move_effect_journal_do_remove_stipulation(start);

          tok = ReadNextTokStr();
          tok = ParseStructuredStip(tok,start);
          {
            slice_index const stipulation_root_hook = input_find_stipulation(start);
            if (stipulation_root_hook==no_slice)
            {
              output_plaintext_input_error_message(NoStipulation);
              tok = 0;
            }
            else
              stipulation_modifiers_notify(start,stipulation_root_hook);
          }
          break;
        }
        case TwinningAdd:
          tok = ReadNextTokStr();
          tok = ParsePieces(tok);
          break;
        case TwinningCond:
        {
          fpos_t const beforeCond = InputGetPosition();
          InitCond();
          tok = ReadNextTokStr();
          tok = ParseCond(tok);
          /* we don't redo a condition twinning */
          move_effect_journal_set_effect_doers(move_effect_input_condition,
                                               &undo_input_condition,
                                               0);
          move_effect_journal_do_remember_condition(beforeCond);
          break;
        }
        case TwinningRemove:
          tok = ParseTwinningRemove();
          break;
        case TwinningPolish:
          tok = ParseTwinningPolish();
          break;
        case TwinningShift:
          tok = ParseTwinningShift();
          break;
        case TwinningSubstitute:
          tok = ParseTwinningSubstitute();
          break;
        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static square NextSquare(square sq)
{
  if (sq%onerow<nr_of_slack_files_left_of_board+nr_files_on_board-1)
    return sq+1;
  else if (sq>=square_a2 && sq<=square_h8)
    return sq - onerow - (nr_files_on_board-1);
  else
    return initsquare;
}

static char *ParseForsythPiece(char *tok,
                               unsigned int nr_chars,
                               Flags *colour_flags,
                               square *pos)
{
  if (*colour_flags==0)
    *colour_flags = BIT(islower((unsigned char)*tok) ? Black : White);

  {
    char const char1 = (char)tolower((unsigned char)*tok++);
    char const char2 = nr_chars==1 ? ' ' : (char)tolower((unsigned char)*tok++);

    piece_walk_type const walk = GetPieNamIndex(char1,char2);
    if (walk!=nr_piece_walks)
    {
      if (move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                                *pos,walk,
                                                *colour_flags,
                                                no_side))
        *pos = NextSquare(*pos);
      else
        tok = 0;
    }
    else
      ++tok;           /* error */
  }

  return tok;
}

static char *ParseForsythColour(char *tok, Flags *colour_flags)
{
  if (*tok=='=')
  {
    ++tok;
    *colour_flags |= NeutralMask;
  }
  else if (*tok=='+')
  {
    ++tok;
    *colour_flags |= BIT(White);
  }
  else if (*tok=='-')
  {
    ++tok;
    *colour_flags |= BIT(Black);
  }

  return tok;
}

static char *ParseForsythPieceAndColor(char *tok, square *pos)
{

  Flags colour_flags = 0;

  tok = ParseForsythColour(tok,&colour_flags);
  if (*tok=='.')
    tok = ParseForsythPiece(tok+1,2,&colour_flags,pos);
  else
    tok = ParseForsythPiece(tok,1,&colour_flags,pos);

  if (tok && colour_flags==0)
      ++tok;  

  return tok;
}

static boolean ParseForsyth(void)
{
  char *tok = ReadNextTokStr();

  square sq = square_a8;

  while (sq && tok && *tok)
    if (isdigit((unsigned char)*tok))
    {
      int num = *tok++ - '0';
      if (isdigit((unsigned char)*tok))
        num = 10*num + *tok++ - '0';
      for (; num && sq; num--)
        sq = NextSquare(sq);
    }
    else if (*tok=='/')
      ++tok;
    else
      tok = ParseForsythPieceAndColor(tok,&sq);

  return tok!=0;
}

static char *ReadRemark(void)
{
  if (ReadToEndOfLine())
  {
    protocol_fprintf(stdout,"%s",InputLine);
    output_plaintext_message(NewLine);
    protocol_fflush(stdout);
  }

  return ReadNextTokStr();
}

/* protocol_close returns an int, but we need a function returning void for atexit */
static void protocol_close_wrapper(void)
{
  if (protocol_close())
    perror(__func__);
}

static char *ReadTrace(void)
{
  static boolean need_to_schedule_protocol_close = true;
  if (ReadToEndOfLine())
  {
    {
      FILE * const protocol = protocol_open(InputLine);
      if (protocol==0)
        output_plaintext_input_error_message(WrOpenError);
      else
      {
        if (need_to_schedule_protocol_close)
        {
          if (atexit(&protocol_close_wrapper))
            perror(__func__);
          else
            need_to_schedule_protocol_close = false;
        }
        output_plaintext_print_version_info(protocol);
      }
    }
  }

  return ReadNextTokStr();
}

static char *ReadAuthor(void)
{
  if (ReadToEndOfLine())
  {
    strcat(ActAuthor,InputLine);
    strcat(ActAuthor,"\n");
  }

  return ReadNextTokStr();
}

static char *ReadAward(void)
{
  if (ReadToEndOfLine())
  {
    strcpy(ActAward,InputLine);
    strcat(ActAward, "\n");
  }

  return ReadNextTokStr();
}

static char *ReadOrigin(void)
{
  if (ReadToEndOfLine())
  {
    strcat(ActOrigin,InputLine);
    strcat(ActOrigin,"\n");
  }

  return ReadNextTokStr();
}

static char *ReadTitle(void)
{
  if (ReadToEndOfLine())
  {
    strcat(ActTitle,InputLine);
    strcat(ActTitle,"\n");
  }

  return ReadNextTokStr();
}

static char *ReadLaTeXToken(slice_index start)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  if (ReadToEndOfLine())
  {
    LaTeXSetup(start);
    result = ParseLaTeXPieces();
  }
  else
    result = ReadNextTokStr();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static void ReadInitialTwin(slice_index start)
{
  char *tok;
  InitialTwinToken token;
  boolean more_input = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  tok = ReadNextTokStr();

  while (more_input)
  {
    token = GetUniqIndex(InitialTwinTokenCount,InitialTwinTokenTab,tok);
    TraceValue("%u",token);TraceEOL();

    if (token>InitialTwinTokenCount)
    {
      output_plaintext_input_error_message(ComNotUniq);
      tok = ReadNextTokStr();
    }
    else if (token==InitialTwinTokenCount)
    {
      if (GetUniqIndex(ProblemTokenCount,ProblemTokenTab,tok)==ProblemTokenCount
          && GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,tok)==EndTwinTokenCount)
      {
        output_plaintext_input_error_message(OffendingItem,tok);
        tok = ReadNextTokStr();
      }
      else
        break;
    }
    else
      switch (token)
      {
        case StipToken:
        {
          tok = ReadNextTokStr();
          tok = ParseStip(tok,start);
          {
            slice_index const root_slice_hook = input_find_stipulation(start);
            if (root_slice_hook==no_slice)
              output_plaintext_input_error_message(UnrecStip);
          }
          break;
        }

        case StructStipToken:
        {
          tok = ReadNextTokStr();
          tok = ParseStructuredStip(tok,start);
          {
            slice_index const root_slice_hook = input_find_stipulation(start);
            if (root_slice_hook==no_slice)
              output_plaintext_input_error_message(UnrecStip);
          }
          break;
        }

        case Author:
          tok = ReadAuthor();
          break;

        case Award:
          tok = ReadAward();
          break;

        case Origin:
          tok = ReadOrigin();
          break;

        case TitleToken:
          tok = ReadTitle();
          break;

        case PieceToken:
        {
          unsigned int ghost_idx = nr_ghosts;

          total_invisible_number = 0;

          tok = ReadNextTokStr();
          tok = ParsePieces(tok);

          for (; ghost_idx!=nr_ghosts; ++ghost_idx)
          {
            WriteSquare(&output_plaintext_engine,stdout,underworld[ghost_idx].on);
            output_plaintext_message(OverwritePiece);
          }
          break;
        }

        case CondToken:
        {
          fpos_t const beforeCond = InputGetPosition();
          tok = ReadNextTokStr();
          tok = ParseCond(tok);
          move_effect_journal_do_remember_condition(beforeCond);
          break;
        }

        case OptToken:
          tok = ParseOpt(start);
          break;

        case RemToken:
          tok = ReadRemark();
          break;

        case TraceToken:
          tok = ReadTrace();
          break;

        case LaTeXPieces:
          tok = ParseLaTeXPieces();
          break;

        case LaTeXToken:
          tok = ReadLaTeXToken(start);
          break;

        case SepToken:
          tok = ReadNextTokStr();
          break;

        case Array:
          {
            int i;
            for (i = 0; i<nr_squares_on_board; i++)
              if (PAS[i]==Empty)
                empty_square(boardnum[i]);
              else
                occupy_square(boardnum[i],PAS[i],BIT(PAS_sides[i]));
            being_solved.king_square[White] = square_e1;
            being_solved.king_square[Black] = square_e8;
          }
          tok = ReadNextTokStr();
          break;

        case Forsyth:
          total_invisible_number = 0;
          if (!ParseForsyth())
            output_plaintext_input_error_message(ParseForsythFailure);
          tok = ReadNextTokStr();
          break;

        default:
          output_plaintext_fatal_message(InternalError);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ReadSubsequentTwin(char *tok, slice_index start, unsigned int twin_number)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",twin_number);
  TraceFunctionParamListEnd();

  tok = ParseTwinning(tok,start,twin_number);

  while (tok)
  {
    InitialTwinToken const result = GetUniqIndex(SubsequentTwinTokenCount,InitialTwinTokenTab,tok);
    if (result>SubsequentTwinTokenCount)
    {
      output_plaintext_input_error_message(ComNotUniq);
      tok = ReadNextTokStr();
    }
    else if (result==SubsequentTwinTokenCount)
      break;
    else
      switch (result)
      {
        case RemToken:
          tok = ReadRemark();
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

void solving_machinery_intro_builder_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STCreateBuilderSetupPly),
        alloc_pipe(STPieceWalkCacheInitialiser),
        alloc_pipe(STPiecesCounter),
        alloc_pipe(STRoyalsLocator),
        alloc_pipe(STPiecesFlagsInitialiser),
#if defined(DOMEASURE)
        alloc_pipe(STCountersWriter),
#endif
        alloc_pipe(STInputVerification),
        alloc_pipe(STMoveEffectsJournalReset),
        alloc_pipe(STSolversBuilder1),
        alloc_pipe(STSolversBuilder2),
        alloc_pipe(STSlackLengthAdjuster),
        alloc_pipe(STProxyResolver)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);

    if (!CondFlag[lesemajeste])
    {
      slice_index const prototype = alloc_illegal_selfcheck_writer_slice();
      slice_insertion_insert(si,&prototype,1);
    }
  }

  solving_impose_starter(si,SLICE_STARTER(si));

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void stipulation_copier_solve(slice_index si)
{
  slice_index const stipulation_prototype = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const proto = stip_deep_copy(SLICE_NEXT1(stipulation_prototype));
    slice_insertion_insert(si,&proto,1);
  }

  solving_impose_starter(si,SLICE_STARTER(stipulation_prototype));
  TraceStipulation(si);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void build_atob_solving_machinery(slice_index si)
{
  slice_index const start_of_machinery = branch_find_slice(STStartOfCurrentTwin,
                                                           si,
                                                           stip_traversal_context_intro);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(start_of_machinery!=no_slice);

  input_instrument_proof(start_of_machinery);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void build_proof_solving_machinery(slice_index si)
{
  slice_index const start_of_machinery = branch_find_slice(STStartOfCurrentTwin,
                                                           si,
                                                           stip_traversal_context_intro);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(start_of_machinery!=no_slice);

  input_instrument_proof(start_of_machinery);

  {
    slice_index const prototype = alloc_pipe(STProofgameStartPositionInitialiser);
    slice_insertion_insert(start_of_machinery,&prototype,1);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void slices_deallocator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  dealloc_slices(SLICE_NEXT1(si));
  SLICE_NEXT1(si) = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean detect_and_impose(slice_index stipulation_root_hook)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  stip_detect_starter(stipulation_root_hook);
  solving_impose_starter(stipulation_root_hook,
                         SLICE_STARTER(stipulation_root_hook));
  if (SLICE_STARTER(SLICE_NEXT1(stipulation_root_hook))==no_side)
    output_plaintext_verifie_message(CantDecideWhoIsAtTheMove);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void stipulation_starter_detector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const stipulation_root_hook = stipulation_modifier_to_be_modified(si);
    if (stipulation_root_hook==no_slice
        || detect_and_impose(stipulation_root_hook))
      pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void twin_id_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  ++twin_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *twins_handle(char *tok, slice_index si)
{
  unsigned int twin_number = twin_b;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  heartbeat_instrument_solving(si);
  pipe_solve_delegate(si);

  while (tok)
  {
    tok = ReadSubsequentTwin(tok,si,twin_number);

    if (tok)
    {
      EndTwinToken const endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,tok);

      if (endToken>EndTwinTokenCount)
        output_plaintext_input_error_message(ComNotUniq);
      else
      {
        heartbeat_instrument_solving(si);
        pipe_solve_delegate(si);

        if (endToken==EndTwinTokenCount)
          break;
        else
        {
          assert(endToken==TwinProblem);
        }
      }

      tok = ReadNextTokStr();
      ++twin_number;
    }
    else
      break;
  }

  ply_reset();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Iterate over the twins of a problem
 * @return token that ended the last twin
 */
void input_plaintext_twins_handle(slice_index si)
{
  char *tok;
  EndTwinToken endToken;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_twinning);

  endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,TokenLine);

  switch (endToken)
  {
    case TwinProblem:
      output_notify_twinning(si,twin_regular+twin_a,false);
      break;

    case ZeroPosition:
      problem_instrument_zeroposition(si);

      output_notify_twinning(si,twin_zeroposition,false);
      pipe_solve_delegate(si);

      tok = ReadNextTokStr();
      ReadSubsequentTwin(tok,si,twin_a);

      endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,TokenLine);
      break;

    default:
      break;
  }

  switch (endToken)
  {
    case TwinProblem:
      tok = ReadNextTokStr();
      tok = twins_handle(tok,si);
      break;

    case EndTwinTokenCount:
      heartbeat_instrument_solving(si);
      pipe_solve_delegate(si);
      break;

    default:
      output_plaintext_input_error_message(ComNotUniq);
      tok = ReadNextTokStr();
      break;
  }

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void input_plaintext_initial_twin_reader_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ReadInitialTwin(si);

  {
    slice_index const stipulation_root_hook = input_find_stipulation(si);
    if (stipulation_root_hook==no_slice)
      output_plaintext_input_error_message(NoStipulation);
    else
    {
      stipulation_modifiers_notify(si,stipulation_root_hook);
      pipe_solve_delegate(si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
