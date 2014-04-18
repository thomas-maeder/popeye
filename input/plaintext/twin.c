#include "input/plaintext/twin.h"
#include "input/plaintext/token.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "input/plaintext/sstipulation.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/input_stack.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/position.h"
#include "output/plaintext/proofgame.h"
#include "output/plaintext/twinning.h"
#include "output/plaintext/message.h"
#include "output/latex/latex.h"
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

static unsigned int TwinNumber;
static piece_walk_type twin_e[nr_squares_on_board];
static Flags  twin_spec[nr_squares_on_board];
static square twin_rb, twin_rn;

static void TwinStorePosition(void)
{
  int i;

  twin_rb= king_square[White];
  twin_rn= king_square[Black];
  for (i= 0; i < nr_squares_on_board; i++)
  {
    twin_e[i] = get_walk_of_piece_on_square(boardnum[i]);
    twin_spec[i] = spec[boardnum[i]];
  }
}

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

  /* issue the twinning */
  WriteSpec(spec[sq1],get_walk_of_piece_on_square(sq1),!is_square_empty(sq1));
  WritePiece(get_walk_of_piece_on_square(sq1));
  WriteSquare(sq1);
  if (indexx == TwinningExchange) {
    StdString("<-->");
    WriteSpec(spec[sq2], get_walk_of_piece_on_square(sq2),!is_square_empty(sq2));
    WritePiece(get_walk_of_piece_on_square(sq2));
    WriteSquare(sq2);
    if (LaTeXout)
      LaTeXEchoExchangedPiece(spec[sq1],get_walk_of_piece_on_square(sq1),sq1,
                              spec[sq2],get_walk_of_piece_on_square(sq2),sq2);
  }
  else
  {
    StdString("-->");
    WriteSquare(sq2);
    if (LaTeXout)
      LaTeXEchoMovedPiece(spec[sq1],get_walk_of_piece_on_square(sq1),sq1,sq2);
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
  {
    move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                rotation);

    if (LaTeXout)
      LaTeXTwinningRotate(tok);

    StdString(TwinningTab[TwinningRotate]);
    StdString(" ");
    StdString(tok);
  }

  return ReadNextTokStr();
}

static char *ParseTwinningMirror(void)
{
  char *tok = ReadNextTokStr();
  TwinningType indexx= GetUniqIndex(TwinningCount,TwinningTab,tok);

  if (indexx>TwinningCount)
    IoErrorMsg(OptNotUniq,0);
  else
  {
    switch (indexx)
    {
      case TwinningMirra1h1:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1h1);
        break;

      case TwinningMirra1a8:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1a8);
        break;

      case TwinningMirra1h8:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra1h8);
        break;

      case TwinningMirra8h1:
        move_effect_journal_do_board_transformation(move_effect_reason_diagram_setup,
                                                    mirra8h1);
        break;

      default:
        IoErrorMsg(UnrecRotMirr,0);
        break;
    }

    StdString(TwinningTab[TwinningMirror]);
    StdString(" ");
    StdString(TwinningTab[indexx]);
  }

  return ReadNextTokStr();
}

static void WriteConditionTwinning(char const CondLine[], boolean is_first)
{
  if (is_first)
  {
    StdString(CondLine);
    if (LaTeXout)
      LaTeXTwinningFirstCondition(CondLine);
  }
  else
  {
    StdString("\n   ");
    StdString(CondLine);
    if (LaTeXout)
      LaTeXTwinningNextCondition(CondLine);
  }
}

static char *ParseTwinningShift(void)
{
  square sq1= 0, sq2= 0;
  square const *bnp;
  char *tok;
  int diffrank, diffcol, minrank, maxrank, mincol, maxcol, r, c;

  /* read the first square */
  while (sq1 == 0) {
    tok = ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == initsquare) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* read the second square */
  while (sq2 == 0)
  {
    tok = ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == initsquare) {
      ErrorMsg(WrongSquareList);
    }
  }

  if (LaTeXout)
    LaTeXTwinningShift(sq1,sq2);

  StdString(TwinningTab[TwinningShift]);
  StdString(" ");
  WriteSquare(sq1);
  StdString(" ==> ");
  WriteSquare(sq2);

  diffrank= sq2/onerow-sq1/onerow;
  diffcol= sq2%onerow-sq1%onerow;

  minrank= 2*nr_of_slack_rows_below_board + nr_rows_on_board - 1;
  maxrank= 0;
  mincol= onerow-1;
  maxcol= 0;

  for (bnp= boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp))
    {
      if (*bnp/onerow < minrank)
        minrank= *bnp/onerow;
      if (*bnp/onerow > maxrank)
        maxrank= *bnp/onerow;
      if (*bnp%onerow < mincol)
        mincol= *bnp%onerow;
      if (*bnp%onerow > maxcol)
        maxcol= *bnp%onerow;
    }

  if ( maxcol+diffcol > 15
       || mincol+diffcol <  8
       || maxrank+diffrank > 15
       || minrank+diffrank <  8)
    ErrorMsg(PieceOutside);
  else
  {
    /* move along columns */
    if (diffrank > 0)
    {
      for (c= 8; c <= 15; c++)
        for (r= maxrank; r >= minrank; r--)
          if (!is_square_empty(onerow*r+c))
            move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                                  onerow*r+c, onerow*(r+diffrank)+c);
    }
    else if (diffrank < 0)
    {
      for (c= 8; c <= 15; c++)
        for (r= minrank; r <= maxrank; r++)
          if (!is_square_empty(onerow*r+c))
            move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                                  onerow*r+c, onerow*(r+diffrank)+c);
    }

    /* move along ranks */
    if (diffcol > 0)
    {
      for (c= maxcol; c >= mincol; c--)
        for (r= 8; r <= 15; r++)
          if (!is_square_empty(onerow*r+c))
            move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                                  onerow*r+c, onerow*r+c+diffcol);
    }
    else if (diffcol < 0)
    {
      for (c= mincol; c <= maxcol; c++)
        for (r= 8; r <= 15; r++)
          if (!is_square_empty(onerow*r+c))
            move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                                  onerow*r+c, onerow*r+c+diffcol);
    }
  }

  /* read next1 token */
  return ReadNextTokStr();
} /* ParseTwinningShift */

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
    {
      if (LaTeXout)
        LaTeXEchoRemovedPiece(spec[sq],get_walk_of_piece_on_square(sq),sq);

      StdString(" -");
      WriteSpec(spec[sq], get_walk_of_piece_on_square(sq),true);
      WritePiece(get_walk_of_piece_on_square(sq));
      WriteSquare(sq);

      move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,sq);
    }
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
  {
    square const king_square_white = king_square[White];
    king_square[White] = king_square[Black];
    king_square[Black] = king_square_white;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (!is_piece_neutral(spec[*bnp]) && !is_square_empty(*bnp))
        move_effect_journal_do_side_change(move_effect_reason_diagram_setup,*bnp);
  }

  StdString(TwinningTab[TwinningPolish]);

  if (LaTeXout)
    LaTeXTwinningPolish();

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
    {
      if (LaTeXout)
        LaTeXEchoSubstitutedPiece(p_old,p_new);

      WritePiece(p_old);
      StdString(" ==> ");
      WritePiece(p_new);

      {
        square const *bnp;
        for (bnp = boardnum; *bnp; bnp++)
          if (get_walk_of_piece_on_square(*bnp)==p_old)
            move_effect_journal_do_walk_change(move_effect_reason_diagram_setup,
                                               *bnp,p_new);
      }
    }
  }

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
                               square *pos,
                               twin_context_type context)
{
  char const char1 = tolower((int)*tok++);
  char const char2 = nr_chars==1 ? ' ' : tolower((int)*tok++);

  piece_walk_type const walk = GetPieNamIndex(char1,char2);
  if (walk>=King)
  {
    if (context==twin_initial)
      occupy_square(*pos,walk,colour_flags);
    else
      move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                            *pos,walk,colour_flags);

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
    SETFLAGMASK(some_pieces_flags,NeutralMask);
  }
  else
    *colour_flags = 0;

  return tok;
}

static char *ParseForsythPieceAndColor(char *tok,
                                       unsigned int nr_chars,
                                       square *pos,
                                       twin_context_type context)
{
  Flags colour_flags;
  tok = ParseForsythColour(tok,&colour_flags);
  if (colour_flags==0)
    ++tok;
  else
    tok = ParseForsythPiece(tok,nr_chars,colour_flags,pos,context);

  return tok;
}

static char *ParseForsyth(twin_context_type context)
{
  square sq = square_a8;
  char *tok = ReadNextCaseSensitiveTokStr();

  sprintf(GlobalStr, "  %s  \n", tok);
  if (context==twin_subsequent)
    StdString(tok);

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
      tok = ParseForsythPieceAndColor(tok+1,2,&sq,context);
    else
      tok = ParseForsythPieceAndColor(tok,1,&sq,context);

  return ReadNextTokStr();
}

static char *ParseTwinning(slice_index root_slice_hook)
{
  char  *tok = ReadNextTokStr();
  boolean continued= false;
  boolean TwinningRead= false;

  ++TwinNumber;
  OptFlag[noboard]= true;

  while (true)
  {
    TwinningType twinning = 0;
    Token tk = StringToToken(tok);

    if (twinning>=TwinningCount
        || tk==TwinProblem
        || tk==NextProblem
        || tk==EndProblem)
    {
      Message(NewLine);
      if (LaTeXout)
        LaTeXEndTwinning();
      break;
    }

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
            continued = true;
          tok = ReadNextTokStr();
          continue;

        default:
          break;
      }

    if (TwinningRead)
    {
      StdString("  ");
      if (LaTeXout)
        LaTeXNextTwinning();
    }
    else
    {
      if (continued)
      {
        StdChar('+');
        if (LaTeXout)
          LaTeXContinuedTwinning();
      }
      else
        TwinResetPosition();

      WriteTwinNumber(TwinNumber);
      if (LaTeXout)
        LaTeXBeginTwinning(TwinNumber);
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
          slice_index const next = slices[root_slice_hook].next1;
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);
        }
        tok = ParseStip(root_slice_hook);

        /* issue the twinning */
        StdString(AlphaStip);
        if (LaTeXout)
          LaTeXTwinningStipulation(AlphaStip);
        break;
      case TwinningStructStip:
        {
          slice_index const next = slices[root_slice_hook].next1;
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);
        }
        tok = ParseStructuredStip(root_slice_hook);

        /* issue the twinning */
        StdString(AlphaStip);
        if (LaTeXout)
          LaTeXTwinningStipulation(AlphaStip);
        break;
      case TwinningAdd:
        tok = ParsePieces(piece_addition_twinning);
        break;
      case TwinningCond:
        InitCond();
        tok = ParseCond();
        WriteConditions(&WriteConditionTwinning);
        break;
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
      case TwinningForsyth:
        {
          square const *bnp;
          for (bnp = boardnum; *bnp; bnp++)
            if (!is_square_empty(*bnp))
              move_effect_journal_do_piece_removal(move_effect_reason_diagram_setup,
                                                   *bnp);
          tok = ParseForsyth(twin_subsequent);
        }
        break;
      default:
        /* no further action required */
        break;
    }
  }

  TraceText("ParseTwinning() returns\n");
  return tok;
} /* ParseTwinning */

Token ReadInitialTwin(slice_index root_slice_hook)
{
  char *tok;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  TraceText("ReadInitialTwin()\n");
  tok = ReadNextTokStr();
  TwinNumber= 1;
  while (true)
  {
    Token const result = StringToToken(tok);
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
          if (TwinNumber==1)
            TwinStorePosition();

          if (slices[root_slice_hook].next1!=no_slice)
            return result;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        case NextProblem:
        case EndProblem:
          if (slices[root_slice_hook].next1!=no_slice)
            return result;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        case ZeroPosition:
          return result;

        case StipToken:
          if (slices[root_slice_hook].next1==no_slice)
          {
            *AlphaStip='\0';
            tok = ParseStip(root_slice_hook);
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
          tok = ParseCond();
          break;

        case OptToken:
          tok = ParseOpt(root_slice_hook);
          break;

        case RemToken:
          ReadRemark();
          tok = ReadNextTokStr();
          break;

        case InputToken:
          ReadToEndOfLine();
          PushInput(InputLine);
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
            king_square[White] = square_e1;
            king_square[Black] = square_e8;
          }
          break;

        case Forsyth:
          tok = ParseForsyth(twin_initial);
          break;

        default:
          FtlMsg(InternalError);
          break;
      }
  }
}

Token ReadSubsequentTwin(slice_index root_slice_hook)
{
  Token result;
  char *tok;
  boolean more_twinning = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

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
          if (slices[root_slice_hook].next1!=no_slice)
            more_twinning = false;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
          }
          break;

        case NextProblem:
        case EndProblem:
          if (root_slice_hook!=no_slice)
            more_twinning = false;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
          }
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

static void initialise_twin(void)
{
  ++twin_number;

  ply_reset();
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
        branch_insert_slices(proxy,&prototype,1);
        help_branch_shorten(next);
      }
      else
      {
        stip_detect_starter(proxy);
        stip_impose_starter(proxy,advers(slices[proxy].starter));
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_stipulation(slice_index stipulation_root_hook,
                                  twin_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  if (slices[stipulation_root_hook].starter==no_side)
    complete_stipulation(stipulation_root_hook);

  TraceStipulation(stipulation_root_hook);

  if (slices[slices[stipulation_root_hook].next1].starter==no_side)
    VerifieMsg(CantDecideWhoIsAtTheMove);
  else
  {
    Side const regular_starter = slices[stipulation_root_hook].starter;

    if (!OptFlag[halfduplex])
      twin_solve_stipulation(stipulation_root_hook,context);

    if (OptFlag[halfduplex] || OptFlag[duplex])
    {
      stip_impose_starter(stipulation_root_hook,advers(regular_starter));
      twin_solve_stipulation(stipulation_root_hook,
                             OptFlag[duplex] ? twin_subsequent : context);
      stip_impose_starter(stipulation_root_hook,regular_starter);
    }

    Message(NewLine);

    WRITE_COUNTER(add_to_move_generation_stack);
    WRITE_COUNTER(play_move);
    WRITE_COUNTER(is_white_king_square_attacked);
    WRITE_COUNTER(is_black_king_square_attacked);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deal with the problem having a zeroposition
 * @return token that ended the first real twin
 */
static Token zeroposition(slice_index stipulation_root_hook)
{
  Token result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  StdString("\n");
  StdString(TokenTab[ZeroPosition]);
  StdString("\n\n");
  TwinNumber= 0;
  TwinStorePosition();

  result = ReadSubsequentTwin(stipulation_root_hook);

  initialise_piece_flags_from_conditions();
  deal_with_stipulation(stipulation_root_hook,twin_subsequent);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Deal with a "real twin" of a problem
 * @return token that ended the current twin
 */
static Token subsequent_twin(slice_index stipulation_root_hook)
{
  Token result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  initialise_twin();

  result = ReadSubsequentTwin(stipulation_root_hook);

  initialise_piece_flags_from_conditions();

  TraceSquare(king_square[White]);
  TraceSquare(king_square[Black]);
  TraceEOL();

  deal_with_stipulation(stipulation_root_hook,twin_subsequent);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Iterate over the twins of a problem
 * @return token that ended the last twin
 */
Token iterate_twins(void)
{
  Token result;
  slice_index stipulation_root_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stipulation_root_hook = alloc_proxy_slice();

  underworld_reset();

  initialise_twin();

  result = ReadInitialTwin(stipulation_root_hook);

  nextply(no_side);
  assert(nbply==ply_twinning);

  initialise_piece_ids();
  initialise_piece_flags_from_conditions();

  if (!OptFlag[noboard])
    WritePosition();

  if (LaTeXout)
    LaTeXBeginDiagram();

  if (result==TwinProblem)
  {
    Message(NewLine);
    WriteTwinNumber(TwinNumber);
    Message(NewLine);
  }

  StartTimer();

  if (result==ZeroPosition)
    result = zeroposition(stipulation_root_hook);
  else
    deal_with_stipulation(stipulation_root_hook,twin_initial);

  while (result==TwinProblem)
    result = subsequent_twin(stipulation_root_hook);

  undo_move_effects();
  finply();

  dealloc_slices(stipulation_root_hook);
  assert_no_leaked_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
