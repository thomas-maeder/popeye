#include "input/plaintext/twin.h"
#include "input/plaintext/token.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "input/plaintext/sstipulation.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/position.h"
#include "output/plaintext/twinning.h"
#include "output/plaintext/message.h"
#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/chameleon.h"
#include "position/underworld.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/move_inverter.h"
#include "stipulation/stipulation.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/goals/prerequisite_guards.h"
#include "solving/machinery/twin.h"
#include "utilities/table.h"
#include "platform/maxmem.h"
#include "platform/pytime.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <string.h>

static void TwinResetPosition(void)
{
  assert(nbply==ply_twinning);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
}

static char *ParseTwinningMove(int indexx)
{
  square sq1= 0, sq2= 0;
  char *tok;

  /* read the first square */
  while (sq1 == 0)
  {
    tok = ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == initsquare) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  /* read the second square */
  while (sq2 == 0)
  {
    tok = ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == initsquare) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  if (is_square_empty(sq1))
  {
    WriteSquare(sq1);
    StdString(": ");
    ErrorMsg(NothingToRemove);
    return ReadNextTokStr();
  }

  if (indexx==TwinningMove)
    move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                          sq1,sq2);
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
    IoErrorMsg(UnrecRotMirr,0);
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
    IoErrorMsg(OptNotUniq,0);
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
        IoErrorMsg(UnrecRotMirr,0);
        break;
    }
  }

  return ReadNextTokStr();
}

static char *ParseTwinningShift(void)
{
  char *tok = ReadNextTokStr();
  square const sq1 = SquareNum(tok[0],tok[1]);

  if (sq1==initsquare)
  {
    ErrorMsg(WrongSquareList);
    return tok;
  }
  else
  {
    char *tok = ReadNextTokStr();
    square const sq2 = SquareNum(tok[0],tok[1]);

    if (sq2==initsquare)
    {
      ErrorMsg(WrongSquareList);
      return tok;
    }
    else
    {
      if (twin_twinning_shift_validate(sq1,sq2))
        move_effect_journal_do_twinning_shift(sq1,sq2);
      else
        ErrorMsg(PieceOutside);

      return ReadNextTokStr();
    }
  }
}

static char *ParseTwinningRemove(void)
{
  char*tok;
  boolean WrongList;

  do {
    WrongList = false;
    tok = ReadNextTokStr();

    if (strlen(tok) % 2)
      WrongList= true;
    else
    {
      char *tok2= tok;

      while (*tok2 && !WrongList)
      {
        if (SquareNum(tok2[0],tok2[1])==initsquare)
          WrongList = true;
        tok2 += 2;
      }
    }
    if (WrongList)
      ErrorMsg(WrongSquareList);
  } while (WrongList);

  while (*tok)
  {
    square const sq = SquareNum(tok[0],tok[1]);

    if (get_walk_of_piece_on_square(sq)>=King)
      move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,sq);
    else
    {
      WriteSquare(sq);
      StdString(": ");
      Message(NothingToRemove);
    }

    tok += 2;
  }

  return ReadNextTokStr();
}

static char *ParseTwinningPolish(void)
{
  move_effect_journal_do_twinning_polish();
  return ReadNextTokStr();
}

static char *ParseTwinningSubstitute(void)
{
  char *tok = ReadNextTokStr();

  piece_walk_type p_old;
  tok = ParseSingleWalk(tok,&p_old);

  if (p_old==nr_piece_walks)
    IoErrorMsg(WrongPieceName,0);
  else
  {
    piece_walk_type p_new;
    tok = ParseSingleWalk(tok,&p_new);

    if (p_new==nr_piece_walks)
      IoErrorMsg(WrongPieceName,0);
    else
      move_effect_journal_do_twinning_substitute(p_old,p_new);
  }

  return tok;
}

static char *ParseTwinning(slice_index root_slice_hook)
{
  char  *tok = ReadNextTokStr();
  boolean TwinningRead= false;

  ++twin_number;

  while (true)
  {
    TwinningType twinning = 0;
    Token tk = StringToToken(tok);

    if (twinning>=TwinningCount
        || tk==TwinProblem
        || tk==NextProblem
        || tk==EndProblem)
      break;

    twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);
    if (twinning>=TwinningCount)
      break;
    else
      switch (twinning)
      {
        case TwinningContinued:
          if (TwinningRead)
            Message(ContinuedFirst);
          else
            twin_is_continued = true;
          tok = ReadNextTokStr();
          continue;

        default:
          break;
      }

    if (!TwinningRead)
    {
      if (!twin_is_continued)
        TwinResetPosition();
    }

    TwinningRead= true;
    switch(twinning)
    {
      case TwinningMove:
        tok = ParseTwinningMove(twinning);
        break;
      case TwinningExchange:
        tok = ParseTwinningMove(twinning);
        break;
      case TwinningRotate:
        tok = ParseTwinningRotate();
        break;
      case TwinningMirror:
        tok = ParseTwinningMirror();
        break;
      case TwinningStip:
      {
        fpos_t const beforeStip = InputGetPosition();

        slice_index const next = slices[root_slice_hook].next1;
        pipe_unlink(root_slice_hook);
        dealloc_slices(next);

        tok = ParseStip(root_slice_hook);
        move_effect_journal_do_remember_stipulation(root_slice_hook,beforeStip);
        break;
      }
      case TwinningStructStip:
      {
        fpos_t const beforeStip = InputGetPosition();

        slice_index const next = slices[root_slice_hook].next1;
        pipe_unlink(root_slice_hook);
        dealloc_slices(next);

        tok = ParseStructuredStip(root_slice_hook);
        move_effect_journal_do_remember_sstipulation(root_slice_hook,beforeStip);
        break;
      }
      case TwinningAdd:
        tok = ParsePieces(piece_addition_twinning);
        break;
      case TwinningCond:
      {
        fpos_t const beforeCond = InputGetPosition();
        InitCond();
        tok = ParseCond();
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
        /* no further action required */
        break;
    }
  }

  TraceText("ParseTwinning() returns\n");
  return tok;
} /* ParseTwinning */

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
                               Flags colour_flags,
                               square *pos)
{
  char const char1 = tolower((int)*tok++);
  char const char2 = nr_chars==1 ? ' ' : tolower((int)*tok++);

  piece_walk_type const walk = GetPieNamIndex(char1,char2);
  if (walk>=King)
  {
    occupy_square(*pos,walk,colour_flags);
    *pos = NextSquare(*pos);
  }
  else
    ++tok;           /* error */

  return tok;
}

static char *ParseForsythColour(char *tok, Flags *colour_flags)
{
  if (isalpha((int)*tok))
    *colour_flags = BIT(islower((int)*tok) ? Black : White);
  else if (*tok=='=')
  {
    ++tok;
    *colour_flags = NeutralMask;
  }
  else
    *colour_flags = 0;

  return tok;
}

static char *ParseForsythPieceAndColor(char *tok,
                                       unsigned int nr_chars,
                                       square *pos)
{
  Flags colour_flags;
  tok = ParseForsythColour(tok,&colour_flags);
  if (colour_flags==0)
    ++tok;
  else
    tok = ParseForsythPiece(tok,nr_chars,colour_flags,pos);

  return tok;
}

static char *ParseForsyth(void)
{
  char *tok = ReadNextCaseSensitiveTokStr();
  square sq = square_a8;

  sprintf(GlobalStr, "  %s  \n", tok);

  while (sq && *tok)
     if (isdigit((int)*tok))
     {
       int num = *tok++ - '0';
       if (isdigit((int)*tok))
         num = 10*num + *tok++ - '0';
       for (; num && sq; num--)
         sq = NextSquare(sq);
     }
     else if (*tok=='.')
       tok = ParseForsythPieceAndColor(tok+1,2,&sq);
     else
       tok = ParseForsythPieceAndColor(tok,1,&sq);

  return ReadNextTokStr();
}

Token ReadInitialTwin(slice_index root_slice_hook)
{
  Token result;
  char *tok;
  boolean more_input = true;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  tok = ReadNextTokStr();
  while (more_input)
  {
    result = StringToToken(tok);
    if (result>TokenCount)
    {
      IoErrorMsg(ComNotUniq,0);
      tok = ReadNextTokStr();
    }
    else
      switch (result)
      {
        case TokenCount:
          IoErrorMsg(ComNotKnown,0);
          tok = ReadNextTokStr();
          break;

        case BeginProblem:
          tok = ReadNextTokStr();
          break;

        case TwinProblem:
          more_input = false;
          break;

        case NextProblem:
        case EndProblem:
        case ZeroPosition:
          more_input = false;
          break;

        case StipToken:
          if (slices[root_slice_hook].next1==no_slice)
          {
            fpos_t const beforeCond = InputGetPosition();
            *AlphaStip='\0';
            tok = ParseStip(root_slice_hook);
            move_effect_journal_do_remember_stipulation(root_slice_hook,beforeCond);
          }
          else
          {
            IoErrorMsg(UnrecStip,0);
            tok = ReadNextTokStr();
          }
          break;

        case StructStipToken:
          *AlphaStip='\0';
          tok = ParseStructuredStip(root_slice_hook);
          break;

        case Author:
          ReadToEndOfLine();
          strcat(ActAuthor,InputLine);
          strcat(ActAuthor,"\n");
          tok = ReadNextTokStr();
          break;

        case Award:
          ReadToEndOfLine();
          strcpy(ActAward,InputLine);
          strcat(ActAward, "\n");
          tok = ReadNextTokStr();
          break;

        case Origin:
          ReadToEndOfLine();
          strcat(ActOrigin,InputLine);
          strcat(ActOrigin,"\n");
          tok = ReadNextTokStr();
          break;

        case TitleToken:
          ReadToEndOfLine();
          strcat(ActTitle,InputLine);
          strcat(ActTitle,"\n");
          tok = ReadNextTokStr();
          break;

        case PieceToken:
          tok = ParsePieces(piece_addition_initial);
          break;

        case CondToken:
        {
          fpos_t const beforeCond = InputGetPosition();
          tok = ParseCond();
          move_effect_journal_do_remember_condition(beforeCond);
          break;
        }

        case OptToken:
          tok = ParseOpt(root_slice_hook);
          break;

        case RemToken:
          ReadRemark();
          tok = ReadNextTokStr();
          break;

        case TraceToken:
          if (TraceFile!=NULL)
            fclose(TraceFile);

          ReadToEndOfLine();
          TraceFile = fopen(InputLine,open_mode);
          if (TraceFile==NULL)
            IoErrorMsg(WrOpenError,0);
          else if (!flag_regression)
          {
            fputs(versionString,TraceFile);
            fputs(maxmemString(),TraceFile);
            fflush(TraceFile);
          }
          tok = ReadNextTokStr();
          break;

        case LaTeXPieces:
          tok = ParseLaTeXPieces(ReadNextTokStr());
          break;

        case LaTeXToken:
          if (LaTeXout)
            LaTeXShutdown();

          ReadToEndOfLine();
          LaTeXout = LaTeXSetup();

          tok = ParseLaTeXPieces(ReadNextTokStr());
          break;

        case SepToken:
          tok = ReadNextTokStr();
          break;

        case Array:
          tok = ReadNextTokStr();
          {
            int i;
            for (i = 0; i<nr_squares_on_board; i++)
              occupy_square(boardnum[i],PAS[i],BIT(PAS_sides[i]));
            being_solved.king_square[White] = square_e1;
            being_solved.king_square[Black] = square_e8;
          }
          break;

        case Forsyth:
          tok = ParseForsyth();
          break;

        default:
          FtlMsg(InternalError);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

Token ReadSubsequentTwin(slice_index root_slice_hook)
{
  Token result;
  char *tok;
  boolean more_twinning = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  twin_is_continued = false;

  tok = ParseTwinning(root_slice_hook);

  while (more_twinning)
  {
    result = StringToToken(tok);
    if (result>TokenCount)
    {
      IoErrorMsg(ComNotUniq,0);
      tok = ReadNextTokStr();
    }
    else
      switch (result)
      {
        case TwinProblem:
          if (slices[root_slice_hook].next1==no_slice)
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
          }
          else
            more_twinning = false;
          break;

        case NextProblem:
        case EndProblem:
          if (root_slice_hook==no_slice)
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
          }
          else
            more_twinning = false;
          break;

        case RemToken:
          ReadRemark();
          tok = ReadNextTokStr();
          break;

        default:
          IoErrorMsg(ComNotKnown,0);
          tok = ReadNextTokStr();
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef enum
{
  whitetoplay_means_shorten,
  whitetoplay_means_change_colors
} meaning_of_whitetoplay;

static void remember_color_change(slice_index si, stip_structure_traversal *st)
{
  meaning_of_whitetoplay * const result = st->param;
  *result = whitetoplay_means_change_colors;
}

static meaning_of_whitetoplay detect_meaning_of_whitetoplay(slice_index si)
{
  stip_structure_traversal st;
  meaning_of_whitetoplay result = whitetoplay_means_shorten;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalAToBReachedTester,
                                           &remember_color_change);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Apply the option White to play
 * @return true iff the option is applicable (and was applied)
 */
static boolean apply_whitetoplay(slice_index proxy)
{
  slice_index next = slices[proxy].next1;
  boolean result = false;
  meaning_of_whitetoplay meaning;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(proxy);

  meaning = detect_meaning_of_whitetoplay(next);

  while (slices[next].type==STProxy || slices[next].type==STOutputModeSelector)
    next = slices[next].next1;

  TraceEnumerator(slice_type,slices[next].type,"\n");
  switch (slices[next].type)
  {
    case STHelpAdapter:
    {
      if (meaning==whitetoplay_means_shorten)
      {
        slice_index const prototype = alloc_move_inverter_slice();
        slice_insertion_insert(proxy,&prototype,1);
        help_branch_shorten(next);
      }
      else
      {
        stip_detect_starter(proxy);
        solving_impose_starter(proxy,advers(slices[proxy].starter));
      }
      result = true;
      break;
    }

    case STMoveInverter:
    {
      /* starting side is already inverted - just allow color change
       * by removing the inverter
       */
      if (meaning==whitetoplay_means_change_colors)
      {
        pipe_remove(next);
        result = true;
      }
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void write_position(slice_index stipulation_root_hook)
{
  if (!OptFlag[noboard])
  {
    if ((find_unique_goal(stipulation_root_hook).type)==goal_atob)
      WritePositionAtoB(slices[stipulation_root_hook].starter);
    else
      WritePositionRegular();
  }
}

static void complete_stipulation(slice_index stipulation_root_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  if (OptFlag[quodlibet] && OptFlag[goal_is_end])
    VerifieMsg(GoalIsEndAndQuodlibetIncompatible);
  else if (OptFlag[quodlibet])
  {
    if (!transform_to_quodlibet(stipulation_root_hook))
      Message(QuodlibetNotApplicable);
  }
  else if (OptFlag[goal_is_end])
  {
    if (!stip_insert_goal_is_end_testers(stipulation_root_hook))
      Message(GoalIsEndNotApplicable);
  }

  if (OptFlag[whitetoplay] && !apply_whitetoplay(stipulation_root_hook))
    Message(WhiteToPlayNotApplicable);

  if (OptFlag[postkeyplay] && !battle_branch_apply_postkeyplay(stipulation_root_hook))
    Message(PostKeyPlayNotApplicable);

  stip_detect_starter(stipulation_root_hook);

  TraceStipulation(stipulation_root_hook);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_stipulation(slice_index stipulation_root_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  if (slices[stipulation_root_hook].starter==no_side)
    complete_stipulation(stipulation_root_hook);

  if (slices[slices[stipulation_root_hook].next1].starter==no_side)
    VerifieMsg(CantDecideWhoIsAtTheMove);
  else
  {
    if (OptFlag[duplex])
    {
      twin_solve(stipulation_root_hook);
      twin_is_duplex = true;
      twin_solve_duplex(stipulation_root_hook);
      twin_is_duplex = false;
    }
    else if (OptFlag[halfduplex])
      twin_solve_duplex(stipulation_root_hook);
    else
      twin_solve(stipulation_root_hook);

    Message(NewLine);

    WRITE_COUNTER(add_to_move_generation_stack);
    WRITE_COUNTER(play_move);
    WRITE_COUNTER(is_white_king_square_attacked);
    WRITE_COUNTER(is_black_king_square_attacked);
  }

  ++twin_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static Token solve_twins(slice_index stipulation_root_hook)
{
  Token result;

  twin_number = twin_a;
  twin_is_continued = false;

  deal_with_stipulation(stipulation_root_hook);

  do
  {
    result = ReadSubsequentTwin(stipulation_root_hook);

    if (slices[stipulation_root_hook].next1==no_slice)
      IoErrorMsg(NoStipulation,0);
    else
    {
      initialise_piece_flags();
      deal_with_stipulation(stipulation_root_hook);
    }

    ply_reset();
  }
  while (result==TwinProblem);

  return result;
}

/* Iterate over the twins of a problem
 * @return token that ended the last twin
 */
Token iterate_twins(void)
{
  Token end_of_twin_token;
  slice_index stipulation_root_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stipulation_root_hook = alloc_proxy_slice();

  underworld_reset();

  ply_reset();

  end_of_twin_token = ReadInitialTwin(stipulation_root_hook);

  if (slices[stipulation_root_hook].next1==no_slice)
    IoErrorMsg(NoStipulation,0);
  else
  {
    nextply(no_side);
    assert(nbply==ply_twinning);

    StartTimer();

    initialise_piece_ids();

    initialise_piece_flags();

    /* this is only needed to be able to indicate the correct starting side
     * in a=>b stipulations: */
    complete_stipulation(stipulation_root_hook);

    write_position(stipulation_root_hook);

    if (LaTeXout)
    {
      LaTeXBeginDiagram();
      LaTexOpenSolution();
    }

    if (end_of_twin_token==ZeroPosition)
    {
      Message(NewLine);
      StdString(TokenTab[ZeroPosition]);
      Message(NewLine);
      Message(NewLine);

      end_of_twin_token = ReadSubsequentTwin(stipulation_root_hook);

      if (slices[stipulation_root_hook].next1==no_slice)
        IoErrorMsg(NoStipulation,0);
      else
      {
        initialise_piece_flags();

        end_of_twin_token = solve_twins(stipulation_root_hook);
      }
    }
    else if (end_of_twin_token==TwinProblem)
    {
      Message(NewLine);

      end_of_twin_token = solve_twins(stipulation_root_hook);
    }
    else
    {
      twin_number = twin_number_original_position_no_twins;
      twin_is_continued = false;
      deal_with_stipulation(stipulation_root_hook);
    }

    undo_move_effects();
    finply();

    dealloc_slices(stipulation_root_hook);
    assert_no_leaked_slices();

    if (LaTeXout)
    {
      LaTeXFlushSolution();
      LaTeXFlushTwinning();
      LaTeXEndDiagram();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",end_of_twin_token);
  TraceFunctionResultEnd();
  return end_of_twin_token;
}
