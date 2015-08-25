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
#include "output/plaintext/protocol.h"
#include "output/plaintext/twinning.h"
#include "output/plaintext/message.h"
#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/chameleon.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/stipulation.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/goals/prerequisite_guards.h"
#include "solving/machinery/twin.h"
#include "solving/pipe.h"
#include "utilities/table.h"
#include "platform/maxmem.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <string.h>

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
    output_plaintext_input_error_message(UnrecRotMirr,0);
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
    output_plaintext_input_error_message(OptNotUniq,0);
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
        output_plaintext_input_error_message(UnrecRotMirr,0);
        break;
    }
  }

  return ReadNextTokStr();
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
        move_effect_journal_do_twinning_shift(sq1,sq2);
      else
        output_plaintext_error_message(PieceOutside);

      return ReadNextTokStr();
    }
  }
}

static void HandleRemovalSquare(square s, void *dummy)
{
  if (get_walk_of_piece_on_square(s)>=King)
    move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,s);
  else
  {
    WriteSquare(&output_plaintext_engine,stderr,s);
    output_plaintext_error_message(NothingToRemove);
  }
}

static char *ParseTwinningRemove(void)
{
  char * const squares_tok = ReadNextTokStr();
  char *tok = ParseSquareList(squares_tok,HandleRemovalSquare,0);
  if (tok==squares_tok)
    output_plaintext_input_error_message(MissngSquareList,0);
  else if (*tok!=0)
    output_plaintext_error_message(WrongSquareList);

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
    output_plaintext_input_error_message(WrongPieceName,0);
  else
  {
    piece_walk_type p_new;
    tok = ParseSingleWalk(tok,&p_new);

    if (p_new==nr_piece_walks)
      output_plaintext_input_error_message(WrongPieceName,0);
    else
      move_effect_journal_do_twinning_substitute(p_old,p_new);
  }

  return tok;
}

static char *ParseTwinning(char *tok, slice_index root_slice_hook)
{
  TwinningType initial_twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  ++twin_number;

  if (initial_twinning==TwinningContinued)
  {
    twin_is_continued = true;
    tok = ReadNextTokStr();
  }
  else if (initial_twinning<TwinningCount)
  {
    twin_is_continued = false;
    TwinResetPosition();
  }

  while (true)
  {
    TwinningType const twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);

    if (twinning>TwinningCount)
    {
      output_plaintext_input_error_message(ComNotUniq,0);
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
          fpos_t const beforeStip = InputGetPosition();

          slice_index const next = SLICE_NEXT1(root_slice_hook);
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);

          tok = ReadNextTokStr();
          tok = ParseStip(tok,root_slice_hook);
          move_effect_journal_do_remember_stipulation(root_slice_hook,beforeStip);
          break;
        }
        case TwinningStructStip:
        {
          fpos_t const beforeStip = InputGetPosition();

          slice_index const next = SLICE_NEXT1(root_slice_hook);
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);

          tok = ReadNextTokStr();
          tok = ParseStructuredStip(tok,root_slice_hook);
          move_effect_journal_do_remember_sstipulation(root_slice_hook,beforeStip);
          break;
        }
        case TwinningAdd:
          tok = ReadNextTokStr();
          tok = ParsePieces(tok,piece_addition_twinning);
          break;
        case TwinningCond:
        {
          fpos_t const beforeCond = InputGetPosition();
          InitCond();
          tok = ReadNextTokStr();
          tok = ParseCond(tok);
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

static char *ParseForsyth(char *tok)
{
  square sq = square_a8;

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

static char *ReadTrace(void)
{
  if (ReadToEndOfLine())
  {
    {
      FILE * const protocol = protocol_open(InputLine);
      if (protocol==0)
        output_plaintext_input_error_message(WrOpenError,0);
      else
        output_plaintext_print_version_info(protocol);
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

static char *ReadLaTeXToken(void)
{
  if (ReadToEndOfLine())
  {
    LaTeXShutdown();
    LaTeXSetup();

    return ParseLaTeXPieces(ReadNextTokStr());
  }
  else
    return ReadNextTokStr();
}

char *ReadInitialTwin(char *tok, slice_index root_slice_hook)
{
  InitialTwinToken result;
  boolean more_input = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  while (more_input)
  {
    result = GetUniqIndex(InitialTwinTokenCount,InitialTwinTokenTab,tok);
    if (result>InitialTwinTokenCount)
    {
      output_plaintext_input_error_message(ComNotUniq,0);
      tok = ReadNextTokStr();
    }
    else if (result==InitialTwinTokenCount)
      break;
    else
      switch (result)
      {
        case StipToken:
          if (SLICE_NEXT1(root_slice_hook)==no_slice)
          {
            fpos_t const beforeCond = InputGetPosition();
            *AlphaStip='\0';
            tok = ReadNextTokStr();
            tok = ParseStip(tok,root_slice_hook);
            move_effect_journal_do_remember_stipulation(root_slice_hook,beforeCond);
          }
          else
          {
            output_plaintext_input_error_message(UnrecStip,0);
            tok = ReadNextTokStr();
          }
          break;

        case StructStipToken:
          AlphaStip[0] ='\0';
          tok = ReadNextTokStr();
          tok = ParseStructuredStip(tok,root_slice_hook);
          break;

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
          tok = ReadNextTokStr();
          tok = ParsePieces(tok,piece_addition_initial);
          break;

        case CondToken:
        {
          fpos_t const beforeCond = InputGetPosition();
          tok = ReadNextTokStr();
          tok = ParseCond(tok);
          move_effect_journal_do_remember_condition(beforeCond);
          break;
        }

        case OptToken:
          tok = ReadNextTokStr();
          tok = ParseOpt(tok,root_slice_hook);
          break;

        case RemToken:
          tok = ReadRemark();
          break;

        case TraceToken:
          tok = ReadTrace();
          break;

        case LaTeXPieces:
          tok = ReadNextTokStr();
          tok = ParseLaTeXPieces(tok);
          break;

        case LaTeXToken:
          tok = ReadLaTeXToken();
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
          tok = ReadNextTokStr();
          tok = ParseForsyth(tok);
          break;

        default:
          output_plaintext_fatal_message(InternalError);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ReadSubsequentTwin(char *tok, slice_index root_slice_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  tok = ParseTwinning(tok,root_slice_hook);

  while (true)
  {
    InitialTwinToken const result = GetUniqIndex(SubsequentTwinTokenCount,InitialTwinTokenTab,tok);
    if (result>SubsequentTwinTokenCount)
    {
      output_plaintext_input_error_message(ComNotUniq,0);
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
  slice_index next = SLICE_NEXT1(proxy);
  boolean result = false;
  meaning_of_whitetoplay meaning;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(proxy);

  meaning = detect_meaning_of_whitetoplay(next);

  while (SLICE_TYPE(next)==STProxy || SLICE_TYPE(next)==STOutputModeSelector)
    next = SLICE_NEXT1(next);

  TraceEnumerator(slice_type,SLICE_TYPE(next),"\n");
  switch (SLICE_TYPE(next))
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
        solving_impose_starter(proxy,advers(SLICE_STARTER(proxy)));
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

static void complete_stipulation(slice_index stipulation_root_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  if (OptFlag[quodlibet] && OptFlag[goal_is_end])
    output_plaintext_verifie_message(GoalIsEndAndQuodlibetIncompatible);
  else if (OptFlag[quodlibet])
  {
    if (!transform_to_quodlibet(stipulation_root_hook))
      output_plaintext_message(QuodlibetNotApplicable);
  }
  else if (OptFlag[goal_is_end])
  {
    if (!stip_insert_goal_is_end_testers(stipulation_root_hook))
      output_plaintext_message(GoalIsEndNotApplicable);
  }

  if (OptFlag[whitetoplay] && !apply_whitetoplay(stipulation_root_hook))
    output_plaintext_message(WhiteToPlayNotApplicable);

  if (OptFlag[postkeyplay] && !battle_branch_apply_postkeyplay(stipulation_root_hook))
    output_plaintext_message(PostKeyPlayNotApplicable);

  stip_detect_starter(stipulation_root_hook);

  TraceStipulation(stipulation_root_hook);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index build(slice_index stipulation_root_hook)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STStartOfSolvingMachinery);
  pipe_link(result,stip_deep_copy(SLICE_NEXT1(stipulation_root_hook)));

  {
    slice_index const prototypes[] = {
        alloc_pipe(STCreateBuilderSetupPly),
        alloc_pipe(STPieceWalkCacheInitialiser),
        alloc_pipe(STPiecesCounter),
        alloc_pipe(STRoyalsLocator),
        alloc_pipe(STPiecesFlagsInitialiser),
        alloc_pipe(STInputVerification),
        alloc_pipe(STMoveEffectsJournalReset),
        alloc_pipe(STSolversBuilder),
        alloc_pipe(STProxyResolver),
        alloc_pipe(STSlackLengthAdjuster),
        alloc_pipe(STStartOfStipulation)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(result,prototypes,nr_prototypes);
  }

  if (stip_ends_in(SLICE_NEXT1(result),goal_proofgame))
  {
    slice_index const prototypes[] = {
        alloc_pipe(STProofgameInitialiser),
        alloc_pipe(STPiecesCounter),
        alloc_pipe(STRoyalsLocator),
        alloc_pipe(STPiecesFlagsInitialiser)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(result,prototypes,nr_prototypes);
  }
  if (stip_ends_in(SLICE_NEXT1(result),goal_atob))
  {
    slice_index const prototypes[] = {
        alloc_pipe(STAToBInitialiser),
        alloc_pipe(STPiecesCounter),
        alloc_pipe(STRoyalsLocator),
        alloc_pipe(STPiecesFlagsInitialiser)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(result,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void build_solving_machinery(slice_index si)
{
  slice_index const stipulation_prototype = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_link(si,build(stipulation_prototype));
  solving_impose_starter(si,SLICE_STARTER(stipulation_prototype));
  TraceStipulation(si);

  pipe_solve_delegate(si);

  dealloc_slices(SLICE_NEXT1(si));
  SLICE_NEXT1(si) = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void swap_side(slice_index si, stip_structure_traversal *st)
{
  slice_index const stpiulation_root = SLICE_NEXT2(si);
  Side const regular_starter = SLICE_STARTER(stpiulation_root);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_impose_starter(stpiulation_root,advers(regular_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void half_duplex_solve(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STSolvingMachineryBuilder,&swap_side);
  stip_traverse_structure(si,&st);

  pipe_solve_delegate(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STSolvingMachineryBuilder,&swap_side);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void duplex_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  twin_duplex_type = twin_has_duplex;

  pipe_solve_delegate(si);

  twin_duplex_type = twin_is_duplex;

  half_duplex_solve(si);

  twin_duplex_type = twin_no_duplex;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_stipulation(slice_index stipulation_root_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(stipulation_root_hook)==no_side)
    complete_stipulation(stipulation_root_hook);

  if (SLICE_STARTER(SLICE_NEXT1(stipulation_root_hook))==no_side)
    output_plaintext_verifie_message(CantDecideWhoIsAtTheMove);
  else
  {
    slice_index const environment = alloc_pipe(STStartOfSolvingEnvironment);
    slice_index const writer = alloc_pipe(STOutputPlainTextEndOfTwinWriter);
    slice_index const builder = alloc_pipe(STSolvingMachineryBuilder);
    slices[builder].next2 = stipulation_root_hook;
    pipe_link(environment,writer);
    pipe_link(writer,builder);

    if (OptFlag[duplex])
    {
      slice_index const prototypes[] = {
          alloc_pipe(STDuplexSolver)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(environment,prototypes,nr_prototypes);
    }
    else if (OptFlag[halfduplex])
    {
      slice_index const prototypes[] = {
          alloc_pipe(STHalfDuplexSolver)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(environment,prototypes,nr_prototypes);
    }

    solve(environment);

    dealloc_slices(environment);

    WRITE_COUNTER(add_to_move_generation_stack);
    WRITE_COUNTER(play_move);
    WRITE_COUNTER(is_white_king_square_attacked);
    WRITE_COUNTER(is_black_king_square_attacked);
  }

  ++twin_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *twins_handle(char *tok, slice_index stipulation_root_hook)
{
  twin_number = twin_a;
  twin_is_continued = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  deal_with_stipulation(stipulation_root_hook);

  while (true)
  {
    tok = ReadSubsequentTwin(tok,stipulation_root_hook);

    if (SLICE_NEXT1(stipulation_root_hook)==no_slice)
    {
      output_plaintext_input_error_message(NoStipulation,0);
      break;
    }
    else
    {
      EndTwinToken const endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,tok);

      if (endToken>EndTwinTokenCount)
        output_plaintext_input_error_message(ComNotUniq,0);
      else if (endToken==EndTwinTokenCount)
      {
        twin_stage = twin_last;
        deal_with_stipulation(stipulation_root_hook);
        break;
      }
      else if (endToken==TwinProblem)
      {
        twin_stage = twin_regular;
        deal_with_stipulation(stipulation_root_hook);
      }
      else
      {
        assert(0);
      }

      tok = ReadNextTokStr();
    }
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
char *input_plaintext_twins_iterate(char *tok, slice_index stipulation_root_hook)
{
  EndTwinToken endToken;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_twinning);

  endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,tok);

  if (endToken==ZeroPosition)
  {
    twin_stage = twin_zeroposition;
    deal_with_stipulation(stipulation_root_hook);

    tok = ReadNextTokStr();
    tok = ReadSubsequentTwin(tok,stipulation_root_hook);

    endToken = GetUniqIndex(EndTwinTokenCount,EndTwinTokenTab,tok);

    if (endToken!=TwinProblem)
      output_plaintext_input_error_message(ZeroPositionNoTwin,0);
    else if (SLICE_NEXT1(stipulation_root_hook)==no_slice)
      output_plaintext_input_error_message(NoStipulation,0);
    else
    {
      twin_stage = twin_regular;
      tok = ReadNextTokStr();
      tok = twins_handle(tok,stipulation_root_hook);
    }
  }
  else if (endToken==TwinProblem)
  {
    twin_stage = twin_initial;
    tok = ReadNextTokStr();
    tok = twins_handle(tok,stipulation_root_hook);
  }
  else
  {
    twin_is_continued = false;
    twin_stage = twin_original_position_no_twins;
    deal_with_stipulation(stipulation_root_hook);
  }

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
