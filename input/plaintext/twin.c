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
#include "output/latex/latex.h"
#include "conditions/imitator.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/walks/orphan.h"
#include "pieces/walks/classification.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/magic.h"
#include "conditions/alphabetic.h"
#include "conditions/blackchecks.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/chameleon.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/duellists.h"
#include "conditions/exclusive.h"
#include "conditions/immune.h"
#include "conditions/mummer.h"
#include "conditions/sentinelles.h"
#include "conditions/follow_my_leader.h"
#include "conditions/football.h"
#include "conditions/forced_squares.h"
#include "conditions/isardam.h"
#include "conditions/must_capture.h"
#include "conditions/oscillating_kings.h"
#include "conditions/republican.h"
#include "conditions/singlebox/type1.h"
#include "conditions/synchronous.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/vaulting_kings.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/king_capture_avoider.h"
#include "solving/castling.h"
#include "solving/check.h"
#include "solving/solvers.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/move_inverter.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "stipulation/moves_traversal.h"
#include "utilities/table.h"
#include "platform/maxmem.h"
#include "platform/pytime.h"
#include "solving/proofgames.h"
#include "pymsg.h"

#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

static unsigned int TwinNumber;
static PieNam twin_e[nr_squares_on_board];
static Flags  twin_spec[nr_squares_on_board];
static square twin_rb, twin_rn;
static imarr  twin_isquare;

static void ReadBeginSpec(void)
{
  while (true)
  {
    char *tok = ReadNextTokStr();
    TokenTab = TokenString[0];
    for (UserLanguage= 0; UserLanguage<LanguageCount; UserLanguage++)
    {
      TokenTab= &(TokenString[UserLanguage][0]);
      if (GetUniqIndex(TokenCount,TokenTab,tok)==BeginProblem)
      {
        OptTab= &OptString[UserLanguage][0];
        CondTab= &CondString[UserLanguage][0];
        TwinningTab= &TwinningString[UserLanguage][0];
        VariantTypeTab= &VariantTypeString[UserLanguage][0];
        ExtraCondTab= &ExtraCondString[UserLanguage][0];
        mummer_strictness_tab = &mummer_strictness_string[UserLanguage][0];
        PieceTab= PieNamString[UserLanguage];
        PieSpTab= PieSpString[UserLanguage];
        ColorTab= ColorString[UserLanguage];
        InitMsgTab(UserLanguage);
        return;
      }
    }

    IoErrorMsg(NoBegOfProblem, 0);
  }
}

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

  for (i= 0; i < maxinum; i++)
    twin_isquare[i]= isquare[i];
}

static void TwinResetPosition(void)
{
  int i;

  king_square[White]= twin_rb;
  king_square[Black]= twin_rn;

  for (i= 0; i < nr_squares_on_board; i++)
    switch (twin_e[i])
    {
      case Empty:
        empty_square(boardnum[i]);
        break;

      case Invalid:
        block_square(boardnum[i]);
        break;

      default:
        occupy_square(boardnum[i],twin_e[i],twin_spec[i]);
        break;
    }

  for (i= 0; i < maxinum; i++)
    isquare[i]= twin_isquare[i];
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

  {
    PieNam const p = get_walk_of_piece_on_square(sq2);
    Flags const sp = spec[sq2];

    occupy_square(sq2,get_walk_of_piece_on_square(sq1),spec[sq1]);

    if (indexx==TwinningMove)
    {
      empty_square(sq1);

      if (sq1 == king_square[White])
        king_square[White]= sq2;
      else if (sq2 == king_square[White])
        king_square[White] = initsquare;

      if (sq1 == king_square[Black])
        king_square[Black]= sq2;
      else if (sq2 == king_square[Black])
        king_square[Black]= initsquare;
    }
    else
    {
      occupy_square(sq1,p,sp);

      if (sq1 == king_square[White])
        king_square[White]= sq2;
      else if (sq2 == king_square[White])
        king_square[White]= sq1;

      if (sq1 == king_square[Black])
        king_square[Black]= sq2;
      else if (sq2 == king_square[Black])
        king_square[Black]= sq1;
    }
  }

  return ReadNextTokStr();
}

static void transformPosition(SquareTransformation transformation)
{
  PieNam t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  square t_rb, t_rn, sq1, sq2;
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  t_rb = king_square[White];
  t_rn = king_square[Black];
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = get_walk_of_piece_on_square(boardnum[i]);
    t_spec[i] = spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = isquare[i];

  /* now rotate/mirror */
  /* pieces */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    sq1 = boardnum[i];
    sq2 = transformSquare(sq1,transformation);

    switch (t_e[i])
    {
      case Empty:
        empty_square(sq2);
        break;

      case Invalid:
        block_square(sq2);
        break;

      default:
        occupy_square(sq2,t_e[i],t_spec[i]);
        break;
    }

    if (sq1==t_rb)
      king_square[White] = sq2;
    if (sq1==t_rn)
      king_square[Black] = sq2;
  }

  /* imitators */
  for (i= 0; i<maxinum; i++)
  {
    sq1 = t_isquare[i];
    sq2 = transformSquare(sq1, transformation);
    isquare[i]= sq2;
  }
} /* transformPosition */

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
    transformPosition(rotation);

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
        transformPosition(mirra1h1);
        break;

      case TwinningMirra1a8:
        transformPosition(mirra1a8);
        break;

      case TwinningMirra1h8:
        transformPosition(mirra1h8);
        break;

      case TwinningMirra8h1:
        transformPosition(mirra8h1);
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

static void MovePieceFromTo(square from, square to)
{
  PieNam const piece = get_walk_of_piece_on_square(from);

  switch (piece)
  {
    case Empty:
      empty_square(to);
      break;

    case Invalid:
      block_square(to);
      empty_square(from);
      break;

    default:
      occupy_square(to,piece,spec[from]);
      empty_square(from);
      if (from == king_square[White])
        king_square[White]= to;
      if (from == king_square[Black])
        king_square[Black]= to;
      break;
  }
} /* MovePieceFromTo */

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
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
    }
    else if (diffrank < 0)
    {
      for (c= 8; c <= 15; c++)
        for (r= minrank; r <= maxrank; r++)
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
    }

    /* move along ranks */
    if (diffcol > 0)
    {
      for (c= maxcol; c >= mincol; c--)
        for (r= 8; r <= 15; r++)
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
    }
    else if (diffcol < 0)
    {
      for (c= mincol; c <= maxcol; c++)
        for (r= 8; r <= 15; r++)
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
    }
  }

  /* read next1 token */
  return ReadNextTokStr();

} /* ParseTwinningShift */

static char *ParseTwinningRemove(void) {
  square    sq;
  char  *tok;
  boolean   WrongList;

  do {
    WrongList= false;
    tok = ReadNextTokStr();

    if (strlen(tok) % 2) {
      WrongList= true;
    }
    else {
      char *tok2= tok;

      while (*tok2 && !WrongList) {
        if (SquareNum(tok2[0], tok2[1]) == initsquare) {
          WrongList= true;
        }
        tok2 += 2;
      }
    }
    if (WrongList) {
      ErrorMsg(WrongSquareList);
    }
  } while (WrongList);

  while (*tok) {
    sq= SquareNum(tok[0], tok[1]);

    if (get_walk_of_piece_on_square(sq) < King) {
      WriteSquare(sq);
      StdString(": ");
      Message(NothingToRemove);
    }
    else {
      if (LaTeXout)
        LaTeXEchoRemovedPiece(spec[sq],get_walk_of_piece_on_square(sq),sq);

      StdString(" -");
      WriteSpec(spec[sq], get_walk_of_piece_on_square(sq),!is_square_empty(sq));
      WritePiece(get_walk_of_piece_on_square(sq));
      WriteSquare(sq);
      empty_square(sq);
      if (sq == king_square[White])
        king_square[White]= initsquare;
      if (sq == king_square[Black])
        king_square[Black]= initsquare;
    }
    tok += 2;
  }

  return ReadNextTokStr();
} /* ParseTwinningRemove */

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
      {
        Flags flags = spec[*bnp];
        piece_change_side(&flags);
        occupy_square(*bnp,get_walk_of_piece_on_square(*bnp),flags);
      }
  }

  StdString(TwinningTab[TwinningPolish]);

  if (LaTeXout)
    LaTeXTwinningPolish();

  return ReadNextTokStr();
}

static char *ParseTwinningSubstitute(void)
{
  PieNam p_old, p_new;
  char  *tok;

  tok = ReadNextTokStr();
  switch (strlen(tok))
  {
  case 1:
    p_old= GetPieNamIndex(*tok,' ');
    break;
  case 2:
    p_old= GetPieNamIndex(*tok,tok[1]);
    break;
  default:
    IoErrorMsg(WrongPieceName,0);
    return tok;
  }

  tok = ReadNextTokStr();
  switch (strlen(tok))
  {
  case 1:
    p_new= GetPieNamIndex(*tok,' ');
    break;
  case 2:
    p_new= GetPieNamIndex(*tok,tok[1]);
    break;
  default:
    IoErrorMsg(WrongPieceName,0);
    return tok;
  }

  if (LaTeXout)
    LaTeXEchoSubstitutedPiece(p_old,p_new);

  WritePiece(p_old);
  StdString(" ==> ");
  WritePiece(p_new);

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (get_walk_of_piece_on_square(*bnp)==p_old)
        replace_piece(*bnp,p_new);
  }

  return ReadNextTokStr();
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

static void SetSquare(square sq, PieNam p, boolean bw, boolean neut)
{
  if (neut)
  {
    occupy_square(sq,p,NeutralMask);
    SETFLAGMASK(some_pieces_flags,NeutralMask);
  }
  else
    occupy_square(sq, p, bw ? BIT(Black) : BIT(White));
}

static char *ParseForsyth(boolean output)
{
  int num;
  square sq = square_a8;
  square const *bnp;
  boolean NeutralFlag= false;
  char *tok = ReadNextCaseSensitiveTokStr();

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  sprintf(GlobalStr, "  %s  \n", tok);
  if (output)
    StdString(tok);

  while (sq && *tok)
  {
    if (isdigit((int)*tok))
    {
      num= (*tok++) - '0';
      if (isdigit((int)*tok))
        num += num*9 + (*tok++) - '0';
      for (;num && sq;num--)
        sq = NextSquare(sq);
      NeutralFlag= false;
    }
    else if (isalpha((int)*tok))
    {
      PieNam const pc= GetPieNamIndex(tolower(*tok),' ');
      if (pc>=King)
      {
        SetSquare(sq,
                  pc,
                  islower((int)InputLine[(tok++)-TokenLine]),
                  NeutralFlag);
        NeutralFlag = false;
        sq = NextSquare(sq);
      }
      else
        tok++;           /* error */
    }
    else if (*tok == '.')
    {
      if (*(tok+1) == '=')
      {
        NeutralFlag= true;
        tok++;
      }
      {
        PieNam const pc= GetPieNamIndex(tolower(*(tok+1)), tolower(*(tok+2)));
        if (pc>=King)
        {
          SetSquare(sq,
                    pc,
                    islower((int)InputLine[(tok+1-TokenLine)]),
                    NeutralFlag);
          NeutralFlag = false;
          sq = NextSquare(sq);
        }
        tok += 3;
      }
    }
    else if (*tok == '=')
    {
      NeutralFlag= true;
      tok++;
    }
    else
      tok++;
  }

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
      return tok;
    }

    twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);
    if (twinning>=TwinningCount)
      return tok;
    else
      switch (twinning)
      {
        case TwinningContinued:
          if (TwinningRead == true)
            Message(ContinuedFirst);
          else
            continued= true;
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
    } /* !TwinningRead */

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
        tok = ParseForsyth(true);
        break;
      default:
        /* no further action required */
        break;
    }
  }

  TraceText("ParseTwinning() returns\n");
} /* ParseTwinning */

Token ReadTwin(Token tk, slice_index root_slice_hook)
{
  char *tok;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  TraceValue("ReadTwin() - %u\n",tk);

  if (tk==BeginProblem)
  {
    LastChar= ' ';
    ReadBeginSpec();
  }

  if (tk == TwinProblem || tk == ZeroPosition)
  {
    if (tk==ZeroPosition)
    {
      StdString("\n");
      StdString(TokenTab[ZeroPosition]);
      StdString("\n\n");
      TwinNumber= 0;
      TwinStorePosition();
    }
    TraceText("-> ParseTwinning()\n");
    tok = ParseTwinning(root_slice_hook);
    TraceValue("ParseTwinning() -> %s\n",tok);

    while (true)
    {
      tk = StringToToken(tok);
      if (tk>TokenCount)
      {
        IoErrorMsg(ComNotUniq,0);
        tok = ReadNextTokStr();
      }
      else
        switch (tk)
        {
          case TwinProblem:
            if (slices[root_slice_hook].next1!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case NextProblem:
          case EndProblem:
            if (root_slice_hook!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

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
  }
  else
  {
    tok = ReadNextTokStr();
    TwinNumber= 1;
    while (true)
    {
      tk = StringToToken(tok);
      if (tk>TokenCount)
      {
        IoErrorMsg(ComNotUniq,0);
        tok = ReadNextTokStr();
      }
      else
        switch (tk)
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
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case NextProblem:
          case EndProblem:
            if (slices[root_slice_hook].next1!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case ZeroPosition:
            return tk;

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
            tok = ParseForsyth(false);
            break;

          default:
            FtlMsg(InternalError);
            break;
        }
    } /* while */
  }
}

static void InitAlways(void)
{
  ply i;

  ply_reset();

  flagfee = false;

  for (i= maxply; i > 0; i--)
  {
    killer_moves[i].departure = initsquare;
    killer_moves[i].arrival = initsquare;
    current_circe_rebirth_square[i] = initsquare;
    trait[i] = White;
    anticirce_current_rebirth_square[i] = initsquare;
  }

  reset_tables();

  reset_max_nr_solutions_per_target_position();

  king_capture_avoiders_reset();

  check_reset_no_king_knowledge();
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

  stip_insert_goal_prerequisite_guards(stipulation_root_hook);

  if (OptFlag[postkeyplay] && !battle_branch_apply_postkeyplay(stipulation_root_hook))
    Message(PostKeyPlayNotApplicable);

  stip_detect_starter(stipulation_root_hook);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void initPieces(void)
{
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initalise_standard_walks();

  for (p = Empty; p<PieceCount; ++p)
    may_exist[p] = false;

  for (p = Empty; p<=Bishop; p++)
    may_exist[standard_walks[p]] = true;

  if (CondFlag[sentinelles])
    may_exist[sentinelle] = true;

  if (CondFlag[chinoises])
    for (p = Leo; p<=Vao; ++p)
      may_exist[p] = true;

  for (p = (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction]
            ? King
            : Queen);
       p < PieceCount;
       ++p)
    if (promonly[p] || is_football_substitute[p])
      may_exist[p] = true;

  if (CondFlag[protean])
    may_exist[ReversePawn] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void countPieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    PieNam p;
    for (p = Empty; p<PieceCount; ++p)
      exist[p] = false;
  }

  {
    PieNam p;
    for (p = King; p<PieceCount; ++p)
    {
      number_of_pieces[White][p] = 0;
      number_of_pieces[Black][p] = 0;
    }
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp))
      {
        PieNam const p = get_walk_of_piece_on_square(*bnp);
        exist[p] = true;
        may_exist[p] = true;
        if (TSTFLAG(spec[*bnp],White))
          ++number_of_pieces[White][p];
        if (TSTFLAG(spec[*bnp],Black))
          ++number_of_pieces[Black][p];
      }
  }

  if (exist[MarinePawn]>0)
  {
    may_exist[Sirene] = true;
    may_exist[Triton] = true;
    may_exist[Nereide] = true;
    may_exist[MarineKnight] = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean initialise_piece_flags(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[volage])
  {
    SETFLAG(some_pieces_flags,Volage);
    SETFLAG(all_pieces_flags,Volage);
  }

  if (CondFlag[patrouille])
  {
    SETFLAG(some_pieces_flags,Patrol);
    SETFLAG(all_pieces_flags,Patrol);
    SETFLAG(all_royals_flags,Patrol);
  }

  if (CondFlag[beamten])
  {
    SETFLAG(some_pieces_flags,Beamtet);
    SETFLAG(all_pieces_flags,Beamtet);
    SETFLAG(all_royals_flags,Beamtet);
  }

  {
    square const *bnp;
    currPieceId = MinPieceId;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp))
      {
        PieNam const p = get_walk_of_piece_on_square(*bnp);
        SETFLAGMASK(spec[*bnp],all_pieces_flags);

        assert(currPieceId<=MaxPieceId);
        SetPieceId(spec[*bnp],currPieceId++);
        SavePositionInDiagram(spec[*bnp],*bnp);

        if (TSTFLAG(spec[*bnp],ColourChange)
            && !is_simplehopper(p))
        {
          /* relies on imitators already having been implemented */
          CLRFLAG(spec[*bnp],ColourChange);
          ErrorMsg(ColourChangeRestricted);
        }
      }

    if (OptFlag[lastcapture] && move_effect_journal[3].type==move_effect_piece_removal)
      SetPieceId(move_effect_journal[3].u.piece_removal.removedspec,currPieceId++);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return true;
}

static boolean locate_unique_royal(Side side)
{
  unsigned int nr_royals = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (royal_square[side]!=initsquare)
  {
    king_square[side] = royal_square[side];
    ++nr_royals;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
    {
      square const s = *bnp;
      if (get_walk_of_piece_on_square(s)==King || get_walk_of_piece_on_square(s)==Poseidon)
      {
        if (TSTFLAG(spec[s],side))
        {
          king_square[side] = s;
          ++nr_royals;
        }
        CLRFLAGMASK(spec[s],all_pieces_flags);
        SETFLAGMASK(spec[s],all_royals_flags|BIT(Royal));
      }
      else if (TSTFLAG(spec[s],Royal))
      {
        if (TSTFLAG(spec[s],side))
        {
          king_square[side] = s;
          ++nr_royals;
        }
        CLRFLAGMASK(spec[s],all_pieces_flags);
        SETFLAGMASK(spec[s],all_royals_flags);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",nr_royals<=1);
  TraceFunctionResultEnd();
  return nr_royals<=1;
}

static boolean locate_royals(void)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  if (CondFlag[dynasty])
  {
    assert(royal_square[White]==initsquare);
    assert(royal_square[Black]==initsquare);

    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
      {
        square const s = *bnp;
        assert(!TSTFLAG(spec[s],Royal));
        if (get_walk_of_piece_on_square(s)==King)
        {
          Side const king_side = TSTFLAG(spec[s],White) ? White : Black;
          CLRFLAGMASK(spec[s],all_pieces_flags);
          SETFLAGMASK(spec[s],all_royals_flags);
          if (number_of_pieces[king_side][King]==1)
          {
            king_square[king_side] = s;
            SETFLAG(spec[s],Royal);
          }
        }
      }
    }
  }
  else if (CondFlag[losingchess] || CondFlag[extinction])
  {
    assert(royal_square[White]==initsquare);
    assert(royal_square[Black]==initsquare);

    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
      {
        assert(!TSTFLAG(spec[*bnp],Royal));
      }
    }
  }
  else if (!locate_unique_royal(White) || !locate_unique_royal(Black))
  {
    VerifieMsg(OneKing);
    result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_binary(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;
  stip_length_type const save_result = *result;
  stip_length_type result1;
  stip_length_type result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_binary_operand1(si,st);
  result1 = *result;
  TraceValue("%u\n",result1);

  *result = save_result;
  stip_traverse_moves_binary_operand2(si,st);
  result2 = *result;
  TraceValue("%u\n",result2);

  if (result1>result2)
    *result = result1;
  TraceValue("%u\n",*result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_move(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  TraceValue("%u\n",*result);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
static stip_length_type get_max_nr_moves(slice_index si)
{
  stip_moves_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  stip_moves_traversal_init(&st,&result);
  stip_moves_traversal_override_by_function(&st,
                                            slice_function_binary,
                                            &get_max_nr_moves_binary);
  stip_moves_traversal_override_single(&st,STMove,&get_max_nr_moves_move);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef boolean is_restricted_type[nr_sides];

static void find_restricted_side_attack(slice_index si,
                                        stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*is_restricted)[advers(slices[si].starter)] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_restricted_side_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*is_restricted)[slices[si].starter] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_restricted_side_help(slice_index si,
                                      stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  Side const starter = slices[si].starter;
  Side const restricted_side = ((length-slack_length)%2==1
                                ? advers(starter)
                                : starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,restricted_side,"\n");
  (*is_restricted)[restricted_side] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor restricted_side_finders[] =
{
  { STAttackAdapter,   &find_restricted_side_attack  },
  { STDefenseAdapter , &find_restricted_side_defense },
  { STReadyForDefense, &find_restricted_side_defense },
  { STHelpAdapter,     &find_restricted_side_help    }
};

enum
{
  nr_restricted_side_finders = (sizeof restricted_side_finders
                                / sizeof restricted_side_finders[0])
};

/* Find the side restricted by some condition (e.g. maximummer without
 * indication of the side)
 * @param si identifies the slice where to start looking for the
 *           restricted side
 * @return the restricted side; no_side if the restricted side can't
 *         be determined
 */
static Side findRestrictedSide(slice_index si)
{
  stip_structure_traversal st;
  is_restricted_type is_restricted = { false, false };
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&is_restricted);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    restricted_side_finders,
                                    nr_restricted_side_finders);
  stip_traverse_structure(si,&st);

  if (is_restricted[White] && !is_restricted[Black])
    result = White;
  else if (is_restricted[Black] && !is_restricted[White])
    result = Black;
  else
    result = no_side;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Verify the user input and our interpretation of it
 * @param si identifies the root slice of the representation of the
 *           stipulation
 * @return true iff the verification was successful
 */
static boolean verify_position(slice_index si)
{
  boolean flagveryfairy = false;
  boolean flagsymmetricfairy = false;
  boolean flagsimplehoppers = false;

  reset_killer_move_optimisation();
  reset_orthodox_mating_move_optimisation();

  reset_countnropponentmoves_defense_move_optimisation();

  if (CondFlag[glasgow] && CondFlag[circemalefique])
    anycirprom = true;

  /* initialize promotion squares */
  if (!CondFlag[einstein])
  {
    square i;
    if (!CondFlag[whprom_sq])
      for (i = 0; i<nr_files_on_board; ++i)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);

    if (!CondFlag[blprom_sq])
      for (i = 0; i<nr_files_on_board; ++i)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);
  }

  {
    unsigned int i;
    for (i = 0; i<nr_files_on_board; i++)
    {
      SETFLAG(sq_spec[square_a1+i*dir_right],WhBaseSq);
      SETFLAG(sq_spec[square_a2+i*dir_right],WhPawnDoublestepSq);
      SETFLAG(sq_spec[square_a7+i*dir_right],BlPawnDoublestepSq);
      SETFLAG(sq_spec[square_a8+i*dir_right],BlBaseSq);

      SETFLAG(sq_spec[square_a3+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a4+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a5+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a6+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a7+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a8+i*dir_right],CapturableByWhPawnSq);

      SETFLAG(sq_spec[square_a1+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a2+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a3+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a4+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a5+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a6+i*dir_right],CapturableByBlPawnSq);

      if (anyparrain
          || CondFlag[normalp]
          || CondFlag[einstein]
          || CondFlag[circecage]
          || CondFlag[wormholes])
      {
        SETFLAG(sq_spec[square_a2+i*dir_right],CapturableByWhPawnSq);
        SETFLAG(sq_spec[square_a7+i*dir_right],CapturableByBlPawnSq);
      }
    }
  }

  im0 = isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom] = true;

  if (get_max_nr_moves(si) >= maxply-2)
  {
    VerifieMsg(BigNumMoves);
    return false;
  }

  if (anyparrain && stip_ends_in(si,goal_steingewinn))
  {
    VerifieMsg(PercentAndParrain);
    return false;
  }

  if (TSTFLAG(some_pieces_flags, HalfNeutral))
    SETFLAGMASK(some_pieces_flags,NeutralMask);

  if (CondFlag[backhome])
    SETFLAGMASK(some_pieces_flags,PieceIdMask);
  if (CondFlag[circediagramm])
    SETFLAGMASK(some_pieces_flags,PieceIdMask);

  if (CondFlag[republican] && !republican_verifie_position(si))
    return false;

  if ((royal_square[Black]!=initsquare || royal_square[White]!=initsquare
       || CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]
       || circe_is_rex_inclusive
       || immune_is_rex_inclusive
       || TSTFLAG(some_pieces_flags,Royal))
      && (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction]))
  {
    VerifieMsg(IncompatibleRoyalSettings);
    return false;
  }

  if (CondFlag[takemake])
  {
    if (CondFlag[sentinelles]
        || CondFlag[nocapture]
        || anyanticirce)
    {
      VerifieMsg(TakeMakeAndFairy);
      return false;
    }
  }

  if (CondFlag[immuncage] && !CondFlag[circecage])
  {
    VerifieMsg(NoCageImmuneWithoutCage);
    CondFlag[immuncage] = false;
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_init();
#endif
  {
    PieNam p;
    for (p = Bishop+1; p<PieceCount; ++p)
    {
      if (may_exist[p] || promonly[p] || is_football_substitute[p])
      {
        flagfee = true;
        if (is_rider(p)) {}
        else if (is_leaper(p)) {}
        else if (is_simplehopper(p))
          flagsimplehoppers = true;
        else if (is_simpledecomposedleaper(p)) {}
        else if (is_symmetricfairy(p))
          flagsymmetricfairy = true;
        else
          flagveryfairy = true;

        if (TSTFLAG(some_pieces_flags,Magic) && !magic_is_piece_supported(p))
        {
          VerifieMsg(MagicAndFairyPieces);
          return false;
        }
        if (CondFlag[einstein])
        {
          VerifieMsg(EinsteinAndFairyPieces);
          return false;
        }
      }
    }
  }

  if (CondFlag[sting])
  {
    disable_orthodox_mating_move_optimisation(nr_sides);
    flagfee = true;
    flagsimplehoppers = true;
  }

  if (CondFlag[imitators])
  {
    if (flagveryfairy
        || flagsymmetricfairy
        || TSTFLAG(some_pieces_flags, Jigger)
        || CondFlag[annan]
        || CondFlag[newkoeko]
        || CondFlag[gridchess] || CondFlag[koeko] || CondFlag[antikoeko]
        || CondFlag[blackedge] || CondFlag[whiteedge]
        || CondFlag[geneva]
        || CondFlag[chinoises]
        || anyimmun
        || CondFlag[haanerchess]
        || CondFlag[sting]
        || anycirce           /* rebirth square may coincide with I */
        || anyanticirce       /* rebirth square may coincide with I */
        || anyparrain) /* verkraftet nicht 2 IUW in einem Zug !!! */
    {
      VerifieMsg(ImitWFairy);
      return false;
    }
    disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[leofamily])
  {
    PieNam p;
    for (p = Queen; p<=Bishop; p++)
      if (number_of_pieces[White][p]+number_of_pieces[Black][p]!=0)
      {
        VerifieMsg(LeoFamAndOrtho);
        return false;
      }
  }

  if (CondFlag[chinoises])
    flagfee = true;

  if (anycirce)
  {
    if (may_exist[Dummy])
    {
      VerifieMsg(CirceAndDummy);
      return false;
    }
    if (is_piece_neutral(some_pieces_flags)
        || CondFlag[volage] || TSTFLAG(some_pieces_flags,Volage))
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  mummer_reset_length_measurers();

  if (ExtraCondFlag[maxi])
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
    }
    else
    {
      mummer_strictness[restricted_side] = mummer_strictness_default_side;
      CondFlag[blmax] = restricted_side==Black;
      CondFlag[whmax] = restricted_side==White;
    }
  }

  if (CondFlag[alphabetic])
  {
    CondFlag[blackalphabetic] = true;
    CondFlag[whitealphabetic] = true;
  }

  if (CondFlag[blmax] && !mummer_set_length_measurer(Black,&maximummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whmax] && !mummer_set_length_measurer(White,&maximummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blmin] && !mummer_set_length_measurer(Black,&minimummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whmin] && !mummer_set_length_measurer(White,&minimummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blcapt] && !mummer_set_length_measurer(Black,&must_capture_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whcapt] && !mummer_set_length_measurer(White,&must_capture_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blfollow] && !mummer_set_length_measurer(Black,&follow_my_leader_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whfollow] && !mummer_set_length_measurer(White,&follow_my_leader_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[duellist]
      && !(mummer_set_length_measurer(Black,&duellists_measure_length)
           && mummer_set_length_measurer(White,&duellists_measure_length)))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blackalphabetic]
      && !mummer_set_length_measurer(Black,&alphabetic_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whitealphabetic]
      && !mummer_set_length_measurer(White,&alphabetic_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blacksynchron]
      && !mummer_set_length_measurer(Black,&synchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whitesynchron]
      && !mummer_set_length_measurer(White,&synchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blackantisynchron]
      && !mummer_set_length_measurer(Black,&antisynchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whiteantisynchron]
      && !mummer_set_length_measurer(White,&antisynchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  /* the mummer logic is (ab)used to priorise transmuting king moves */
  if (CondFlag[blsupertrans_king]
      && !mummer_set_length_measurer(Black,&len_supertransmuting_kings))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whsupertrans_king]
      && !mummer_set_length_measurer(White,&len_supertransmuting_kings))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
    return false;
  }
  if (CondFlag[whforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blconforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whconforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[schwarzschacher]
      && !mummer_set_length_measurer(Black,&blackchecks_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }


  if (ExtraCondFlag[ultraschachzwang])
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
    }
    else
    {
      CondFlag[blackultraschachzwang] = restricted_side==Black;
      CondFlag[whiteultraschachzwang] = restricted_side==White;

      disable_orthodox_mating_move_optimisation(nr_sides);
    }
  }

  if (CondFlag[cavaliermajeur])
  {
    if (number_of_pieces[White][Knight] + number_of_pieces[Black][Knight] > 0)
    {
      VerifieMsg(CavMajAndKnight);
      return false;
    }
    flagfee = true;
  }

  if (OptFlag[sansrb] && king_square[White]!=initsquare)
    OptFlag[sansrb] = false;

  if (OptFlag[sansrn] && king_square[Black]!=initsquare)
    OptFlag[sansrn] = false;

  if (king_square[White]==initsquare && number_of_pieces[White][King]==0 && !OptFlag[sansrb])
    ErrorMsg(MissingKing);

  if (king_square[Black]==initsquare && number_of_pieces[Black][King]==0 && !OptFlag[sansrn])
    ErrorMsg(MissingKing);

  if (circe_is_rex_inclusive)
  {
    if (CondFlag[circeequipollents]
        || CondFlag[circeclone]
        || CondFlag[couscous]
        || CondFlag[circeclonemalefique])
    {
      /* disallowed because of the call to (*circerenai) in echecc would require
       * knowledge of the departure square. Other forms now allowed
       */
      if ((!OptFlag[sansrb]
           && king_square[White]!=initsquare
           && get_walk_of_piece_on_square(king_square[White])!=King)
          || (!OptFlag[sansrn]
              && king_square[Black]!=initsquare
              && get_walk_of_piece_on_square(king_square[Black])!=King))
      {
        VerifieMsg(RoyalPWCRexCirce);
        return false;
      }
    }
  }

  if (CondFlag[bicolores]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || CondFlag[volage]
      || CondFlag[masand]
      || CondFlag[dynasty]
      || TSTFLAG(some_pieces_flags,Magic))
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if (circe_is_rex_inclusive
      || CondFlag[bicolores])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[monochro] && CondFlag[bichro])
  {
    VerifieMsg(MonoAndBiChrom);
    return false;
  }

  if (CondFlag[bichro] || CondFlag[monochro])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if ((CondFlag[koeko]
       || CondFlag[newkoeko]
       || CondFlag[antikoeko]
       || TSTFLAG(some_pieces_flags, Jigger))
      && anycirce
      && is_piece_neutral(some_pieces_flags))
  {
    VerifieMsg(TooFairyForNeutral);
    return false;
  }

  if (TSTFLAG(some_pieces_flags, Kamikaze))
  {
    disable_orthodox_mating_move_optimisation(nr_sides);
    if (CondFlag[haanerchess])
    {
      VerifieMsg(KamikazeAndHaaner);
      return false;
    }
    if (anycirce) {
      /* No Kamikaze and Circe with fairy pieces; taking and
         taken piece could be reborn on the same square! */
      if (flagfee || CondFlag[volage])
      {
        VerifieMsg(KamikazeAndSomeCond);
        return false;
      }
    }
    king_capture_avoiders_avoid_own();
  }

  if ((CondFlag[supercirce] || CondFlag[april] || CondFlag[circecage])
      && (CondFlag[koeko] || CondFlag[newkoeko] || CondFlag[antikoeko]))
  {
    VerifieMsg(SuperCirceAndOthers);
    return false;
  }

  {
    int numsuper=0;
    if (CondFlag[supercirce]) numsuper++;
    if (CondFlag[circecage]) numsuper++;
    if (CondFlag[april]) numsuper++;
    if (CondFlag[antisuper]) numsuper++;
    if (numsuper>1)
    {
      VerifieMsg(SuperCirceAndOthers);
      return false;
    }
  }

  if (CondFlag[nowhcapture] && CondFlag[noblcapture])
  {
    CondFlag[nocapture] = true;
    if (CondFlag[nocapture])
    {
      CondFlag[nowhcapture] = false;
      CondFlag[noblcapture] = false;
    }
  }

  if (CondFlag[isardam]+CondFlag[madras]+CondFlag[eiffel]>1)
  {
    VerifieMsg(IsardamAndMadrasi);
    return false;
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs])
  {
    if (king_square[White]==initsquare || king_square[Black]==initsquare)
    {
      CondFlag[black_oscillatingKs] = false;
      CondFlag[white_oscillatingKs] = false;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }
  if (CondFlag[black_oscillatingKs] && OscillatingKingsTypeC[White]
      && CondFlag[white_oscillatingKs] && OscillatingKingsTypeC[White])
    CondFlag[swappingkings] = true;

  if (anymars || anyantimars || CondFlag[phantom])
  {
    if (anymars+anyantimars+CondFlag[phantom]>1)
    {
      VerifieMsg(MarsCirceAndOthers);
      return false;
    }
    else if ((CondFlag[whvault_king] && vaulting_kings_transmuting[White])
             || (CondFlag[blvault_king] && vaulting_kings_transmuting[Black])
             || calc_reflective_king[White]
             || calc_reflective_king[Black]
             || CondFlag[bicolores]
             || CondFlag[sting]
             || flagsimplehoppers)
    {
      VerifieMsg(MarsCirceAndOthers);
      return false;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[madras] || CondFlag[eiffel] || CondFlag[isardam])
  {
    if ( CondFlag[imitators]|| TSTFLAG(some_pieces_flags,Paralysing))
    {
      VerifieMsg(MadrasiParaAndOthers);
      return false;
    }
  }

  if (CondFlag[circeassassin]) {
    if (is_piece_neutral(some_pieces_flags) /* Neutrals not implemented */
        || CondFlag[bicolores])             /* others? */
    {
      VerifieMsg(AssassinandOthers);
      return false;
    }
  }

  if (circe_is_rex_inclusive && immune_is_rex_inclusive)
  {
    VerifieMsg(RexCirceImmun);
    return false;
  }

  if (immune_is_rex_inclusive && anyanticirce)
  {
    VerifieMsg(SomeCondAndAntiCirce);
    return false;
  }

  if (anyanticirce) {
    if (CondFlag[couscous]
        || CondFlag[koeko]
        || CondFlag[newkoeko]
        || CondFlag[antikoeko]
        || (CondFlag[singlebox] && SingleBoxType==singlebox_type1)
        || CondFlag[geneva]
        || TSTFLAG(some_pieces_flags, Kamikaze))
    {
      VerifieMsg(SomeCondAndAntiCirce);
      return false;
    }
  }

  if ((CondFlag[singlebox]  && SingleBoxType==singlebox_type1))
  {
    if (flagfee)
    {
      VerifieMsg(SingleBoxAndFairyPieces);
      return false;
    }
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      && (OptFlag[sansrb] || OptFlag[sansrn]))
  {
    VerifieMsg(MissingKing);
    return false;
  }

  if (mummer_strictness[White]==mummer_strictness_ultra && !CondFlag[whcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if (mummer_strictness[Black]==mummer_strictness_ultra && !CondFlag[blcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if ((CondFlag[heffalumps] || CondFlag[biheffalumps])
      && (may_exist[Rose]
          || may_exist[SpiralSpringer]
          || may_exist[UbiUbi]
          || may_exist[Hamster]
          || may_exist[Elk]
          || may_exist[Eagle]
          || may_exist[Sparrow]
          || may_exist[Archbishop]
          || may_exist[ReflectBishop]
          || may_exist[Cardinal]
          || may_exist[BoyScout]
          || may_exist[GirlScout]
          || may_exist[DiagonalSpiralSpringer]
          || may_exist[BouncyKnight]
          || may_exist[BouncyNightrider]
          || may_exist[CAT]
          || may_exist[RoseHopper]
          || may_exist[RoseLion]
          || may_exist[Rao]
          || may_exist[RookMoose]
          || may_exist[RookEagle]
          || may_exist[RookSparrow]
          || may_exist[BishopMoose]
          || may_exist[BishopEagle]
          || may_exist[BishopSparrow]
          || may_exist[DoubleGras]
          || may_exist[DoubleRookHopper]
          || may_exist[DoubleBishopper]))
  {
    VerifieMsg(SomePiecesAndHeffa);
    return false;
  }

  if (CondFlag[ghostchess] || CondFlag[hauntedchess])
  {
    if (anycirce || anyanticirce
        || CondFlag[haanerchess]
        || TSTFLAG(some_pieces_flags,Kamikaze)
        || (CondFlag[ghostchess] && CondFlag[hauntedchess]))
    {
      VerifieMsg(GhostHauntedChessAndCirceKamikazeHaanIncompatible);
      return false;
    }
    else
      SETFLAG(some_pieces_flags,Uncapturable);
  }

  change_moving_piece=
      TSTFLAG(some_pieces_flags, Kamikaze)
      || TSTFLAG(some_pieces_flags, Protean)
      || CondFlag[tibet]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || TSTFLAG(some_pieces_flags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[volage]
      || TSTFLAG(some_pieces_flags, Volage)
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[traitor]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[protean]
      || CondFlag[champursue];

  if (CondFlag[vogt]
      || CondFlag[antikings]
      || CondFlag[SAT]
      || CondFlag[strictSAT])
    king_capture_avoiders_avoid_opponent();

  if (TSTFLAG(some_pieces_flags, Jigger)
      || CondFlag[newkoeko]
      || CondFlag[koeko]
      || CondFlag[antikoeko]
      || anyparrain
      || anyanticirce
      || mummer_strictness[White]!=mummer_strictness_none
      || mummer_strictness[Black]!=mummer_strictness_none
      || CondFlag[vogt]
      || CondFlag[central]
      || TSTFLAG(some_pieces_flags,Beamtet)
      || TSTFLAG(some_pieces_flags,Patrol)
      || CondFlag[provocateurs]
      || CondFlag[ultrapatrouille]
      || CondFlag[lortap]
      || CondFlag[shieldedkings]
      || TSTFLAG(some_pieces_flags,Paralysing)
      || CondFlag[madras] || CondFlag[eiffel]
      || CondFlag[brunner]
      || (king_square[White] != initsquare && get_walk_of_piece_on_square(king_square[White]) != King)
      || (king_square[Black] != initsquare && get_walk_of_piece_on_square(king_square[Black]) != King)
      || TSTFLAG(some_pieces_flags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[messigny]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[antikings]
      || TSTFLAG(some_pieces_flags, HalfNeutral)
      || CondFlag[geneva]
      || CondFlag[disparate]
      || CondFlag[BGL]
      || CondFlag[dynasty] /* TODO why? */
      || TSTFLAG(some_pieces_flags,Magic)
      || CondFlag[woozles] || CondFlag[biwoozles]
      || CondFlag[heffalumps] || CondFlag[biheffalumps]
      || (CondFlag[singlebox]
          && (SingleBoxType==singlebox_type1 || SingleBoxType==singlebox_type3))
      || CondFlag[football]
      || CondFlag[wormholes])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[superguards])
    disable_orthodox_mating_move_optimisation(nr_sides);

  pieces_pawns_init_promotion_pieces();

  if (CondFlag[football])
    init_football_substitutes();

  {
    PieNam p;
    PieNam firstprompiece;
    unsigned int check_piece_index = 0;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
      firstprompiece = King;
    else if ((CondFlag[singlebox] && SingleBoxType!=singlebox_type1) || CondFlag[football])
      firstprompiece = Pawn;
    else
      firstprompiece = Queen;

    for (p = firstprompiece; p<PieceCount; ++p)
      if (may_exist[p])
      {
        if (p>Bishop && p!=Dummy)
        {
          /* only fairy pieces until now ! */
          disable_orthodox_mating_move_optimisation(nr_sides);
          if (p!=Hamster)
          {
            checkpieces[check_piece_index] = p;
            check_piece_index++;
          }
        }
      }

    checkpieces[check_piece_index] = Empty;
  }

  {
    unsigned int op = 0;
    PieNam p;
    for (p = King; p<PieceCount; ++p) {
      if (may_exist[p] && p!=Dummy && p!=Hamster)
      {
        if (p!=Orphan && p!=Friend
            && (may_exist[Orphan] || may_exist[Friend]))
          orphanpieces[op++] = p;
      }
    }
    orphanpieces[op] = Empty;
  }

  if (CondFlag[whtrans_king] || CondFlag[whsupertrans_king] || CondFlag[whrefl_king])
    init_transmuters_sequence(White);
  if (CondFlag[bltrans_king] || CondFlag[blsupertrans_king] || CondFlag[blrefl_king])
    init_transmuters_sequence(Black);

  if (calc_reflective_king[White] || calc_reflective_king[Black])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if ((calc_reflective_king[White]
       && king_square[White] != initsquare
       && (get_walk_of_piece_on_square(king_square[White])!=King
           || CondFlag[sting]))
      || (calc_reflective_king[Black]
          && king_square[Black] != initsquare
          && (get_walk_of_piece_on_square(king_square[Black])!=King
              || CondFlag[sting])))
  {
    VerifieMsg(TransmRoyalPieces);
    return false;
  }

  if ((may_exist[Orphan]
       || may_exist[Friend]
       || calc_reflective_king[White]
       || calc_reflective_king[Black])
      && is_piece_neutral(some_pieces_flags))
  {
    VerifieMsg(TooFairyForNeutral);
    return false;
  }

  if (((CondFlag[isardam] && !IsardamB) || CondFlag[brunner])
      && CondFlag[vogt])
  {
    VerifieMsg(VogtlanderandIsardam);
    return false;
  }

  if ((CondFlag[chamchess] || CondFlag[linechamchess])
      && TSTFLAG(some_pieces_flags, Chameleon))
  {
    VerifieMsg(ChameleonPiecesAndChess);
    return false;
  }

  if (CondFlag[platzwechselrochade] && CondFlag[haanerchess])
  {
    VerifieMsg(NonsenseCombination);
    return false;
  }

  if (TSTFLAG(some_pieces_flags, ColourChange))
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[sentinelles])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[annan])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[losingchess])
  {
    if (stip_ends_in(si,goal_mate)
        || stip_ends_in(si,goal_check)
        || stip_ends_in(si,goal_mate_or_stale))
    {
      VerifieMsg(LosingChessNotInCheckOrMateStipulations);
      return false;
    }

    /* capturing moves are "longer" than non-capturing moves */
    if (!(mummer_set_length_measurer(Black,&must_capture_measure_length)
          && mummer_set_length_measurer(White,&must_capture_measure_length)))
    {
      VerifieMsg(TwoMummerCond);
      return false;
    }
  }

  /* check castling possibilities */
  CLEARFL(castling_flag);

  if ((get_walk_of_piece_on_square(square_e1)== standard_walks[King]) && TSTFLAG(spec[square_e1], White)
      && (!CondFlag[dynasty] || number_of_pieces[White][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(White,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h1)== standard_walks[Rook]) && TSTFLAG(spec[square_h1], White))
    SETCASTLINGFLAGMASK(White,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a1)== standard_walks[Rook]) && TSTFLAG(spec[square_a1], White))
    SETCASTLINGFLAGMASK(White,ra_cancastle);
  if ((get_walk_of_piece_on_square(square_e8)== standard_walks[King]) && TSTFLAG(spec[square_e8], Black)
      && (!CondFlag[dynasty] || number_of_pieces[Black][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(Black,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h8)== standard_walks[Rook]) && TSTFLAG(spec[square_h8], Black))
    SETCASTLINGFLAGMASK(Black,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a8)== standard_walks[Rook]) && TSTFLAG(spec[square_a8], Black))
    SETCASTLINGFLAGMASK(Black,ra_cancastle);

  castling_flag &= castling_flags_no_castling;

  /* a small hack to enable ep keys */
  trait[1] = no_side;

  if (CondFlag[exclusive] && !exclusive_verifie_position(si))
    return false;

  if (CondFlag[isardam]
      || CondFlag[ohneschach])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[extinction] || CondFlag[circeassassin])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[actrevolving] || CondFlag[arc])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[circeturncoats] || CondFlag[circedoubleagents])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[kobulkings])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[chamcirce])
    chameleon_circe_init_implicit();

  if (CondFlag[SAT] || CondFlag[strictSAT])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[schwarzschacher])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (mummer_strictness[White]!=mummer_strictness_none /* counting opponents moves not useful */
      || is_piece_neutral(some_pieces_flags)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || anymars || CondFlag[phantom]
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[circecage]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || may_exist[UbiUbi] /* sorting by nr of opponents moves doesn't work - why?? */
      || may_exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(White);

  if (mummer_strictness[Black]!=mummer_strictness_none /* counting opponents moves not useful */
      || is_piece_neutral(some_pieces_flags)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || anymars || CondFlag[phantom]
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[circecage]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || may_exist[UbiUbi] /* sorting by nr of opponents moves doesn't work  - why?? */
      || may_exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(Black);

  if (CondFlag[takemake])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[protean])
  {
    flagfee = true;
    disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[castlingchess] || CondFlag[platzwechselrochade])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (mummer_strictness[Black]!=mummer_strictness_none
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(some_pieces_flags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(Black);
  if (mummer_strictness[White]!=mummer_strictness_none
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(some_pieces_flags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(White);

  move_effect_journal_reset();

  return true;
}

/* Solve a twin (maybe the only one of a problem)
 * @param si identifies the root slice of the stipulation
 * @param twin_index 0 for first, 1 for second ...; if the problem has
 *                   a zero position, solve_twin() is invoked with
 *                   1, 2, ... but not with 0
 * @param end_of_twin_token token that ended this twin
 */
static void solve_twin(slice_index si,
                       unsigned int twin_index,
                       Token end_of_twin_token)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParamListEnd();

  if (twin_index==0)
  {
    if (LaTeXout)
      LaTeXBeginDiagram();

    if (end_of_twin_token==TwinProblem)
    {
      Message(NewLine);
      WriteTwinNumber(TwinNumber);
      Message(NewLine);
    }
  }

  solve(si,length_unspecified);

#ifdef _SE_DECORATE_SOLUTION_
    se_end_half_duplex();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void solve_any_stipulation(slice_index stipulation_root_hook,
                                  unsigned int twin_index,
                                  Token prev_token)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  {
    slice_index const root_slice = build_solvers(stipulation_root_hook);
    TraceStipulation(root_slice);
    solve_twin(root_slice,twin_index,prev_token);
    dealloc_slices(root_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void solve_proofgame_stipulation(slice_index stipulation_root_hook,
                                        unsigned int twin_index,
                                        Token prev_token)
{
  slice_index const stipulation_root = slices[stipulation_root_hook].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  {
    Goal const unique_goal = find_unique_goal(stipulation_root);
    if (unique_goal.type==no_goal)
      VerifieMsg(MultipleGoalsWithProofGameNotAcceptable);
    else
    {
      assert(unique_goal.type==goal_proofgame || unique_goal.type==goal_atob);

      countPieces();
      if (initialise_piece_flags() && locate_royals())
      {
        ProofSaveTargetPosition();

        if (stip_ends_in(stipulation_root,goal_proofgame))
          ProofInitialiseStartPosition();

        ProofRestoreStartPosition();

        countPieces();
        if (initialise_piece_flags() && locate_royals() && verify_position(stipulation_root))
        {
          ProofSaveStartPosition();
          ProofRestoreTargetPosition();

          ProofInitialise(stipulation_root);

          if (!OptFlag[noboard] && twin_index==0)
            WritePosition();

          ProofRestoreStartPosition();
          if (unique_goal.type==goal_atob && !OptFlag[noboard] && twin_index==0)
            ProofWriteStartPosition(stipulation_root);

          solve_any_stipulation(stipulation_root_hook,twin_index,prev_token);
        }

        ProofRestoreTargetPosition();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void solve_non_proofgame_stipulation(slice_index stipulation_root_hook,
                                            unsigned int twin_index,
                                            Token prev_token)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  countPieces();
  if (initialise_piece_flags()
      && locate_royals()
      && verify_position(slices[stipulation_root_hook].next1))
  {
    if (!OptFlag[noboard] && twin_index==0)
      WritePosition();

    solve_any_stipulation(stipulation_root_hook,twin_index,prev_token);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void solve_stipulation(slice_index stipulation_root_hook,
                              unsigned int twin_index,
                              Token prev_token)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParamListEnd();

  initPieces();

  if (stip_ends_in(slices[stipulation_root_hook].next1,goal_proofgame)
      || stip_ends_in(slices[stipulation_root_hook].next1,goal_atob))
    solve_proofgame_stipulation(stipulation_root_hook,twin_index,prev_token);
  else
    solve_non_proofgame_stipulation(stipulation_root_hook,twin_index,prev_token);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate over the twins of a problem
 * @prev_token token that ended the previous twin
 * @return token that ended the current twin
 */
Token iterate_twins(Token prev_token)
{
  unsigned int twin_index = 0;
  slice_index stipulation_root_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  stipulation_root_hook = alloc_proxy_slice();

  do
  {
    InitAlways();

    prev_token = ReadTwin(prev_token,stipulation_root_hook);

    if (twin_index==0)
      /* Set the timer for real calculation time */
      StartTimer();

    if (prev_token==ZeroPosition)
    {
      if (!OptFlag[noboard])
        WritePosition();

      prev_token = ReadTwin(prev_token,stipulation_root_hook);
      if (LaTeXout)
        LaTeXBeginDiagram();

      ++twin_index;
    }

    if (slices[stipulation_root_hook].starter==no_side)
      complete_stipulation(stipulation_root_hook);

    TraceStipulation(stipulation_root_hook);

    if (slices[slices[stipulation_root_hook].next1].starter==no_side)
      VerifieMsg(CantDecideWhoIsAtTheMove);
    else
    {
      Side const regular_starter = slices[stipulation_root_hook].starter;

      if (!OptFlag[halfduplex])
        solve_stipulation(stipulation_root_hook,
                          twin_index,
                          prev_token);

      if (OptFlag[halfduplex] || OptFlag[duplex])
      {
        stip_impose_starter(stipulation_root_hook,advers(regular_starter));
        solve_stipulation(stipulation_root_hook,
                          twin_index+OptFlag[duplex],
                          prev_token);
        stip_impose_starter(stipulation_root_hook,regular_starter);
      }

      Message(NewLine);

      WRITE_COUNTER(add_to_move_generation_stack);
      WRITE_COUNTER(play_move);
      WRITE_COUNTER(is_white_king_square_attacked);
      WRITE_COUNTER(is_black_king_square_attacked);
    }

    ++twin_index;
  } while (prev_token==TwinProblem);

  dealloc_slices(stipulation_root_hook);

  assert_no_leaked_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",prev_token);
  TraceFunctionResultEnd();
  return prev_token;
}
