#include "input/plaintext/twin.h"
#include "input/plaintext/token.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/stipulation.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/input_stack.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/condition.h"
#include "output/latex/latex.h"
#include "conditions/imitator.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/pipe.h"
#include "platform/maxmem.h"
#include "pymsg.h"
#include "pyproc.h"

#include "debugging/trace.h"

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
  if (LaTeXout)
    LaTeXEchoAddedPiece(spec[sq1],get_walk_of_piece_on_square(sq1),sq1);

  WriteSpec(spec[sq1],get_walk_of_piece_on_square(sq1),!is_square_empty(sq1));
  WritePiece(get_walk_of_piece_on_square(sq1));
  WriteSquare(sq1);
  if (indexx == TwinningExchange) {
    StdString("<-->");
    WriteSpec(spec[sq2], get_walk_of_piece_on_square(sq2),!is_square_empty(sq2));
    WritePiece(get_walk_of_piece_on_square(sq2));
    if (LaTeXout)
      LaTeXEchoExchangedPiece(spec[sq2],get_walk_of_piece_on_square(sq2));
  }
  else {
    StdString("-->");
    if (LaTeXout) {
      strcat(ActTwinning, "{\\ra}");
    }
  }
  WriteSquare(sq2);
  if (LaTeXout) {
    sprintf(GlobalStr, "%c%c",
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwinning, GlobalStr);
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

static char *ParseTwinningRotate(void)
{
  char *tok = ReadNextTokStr();

  if (strcmp(tok,"90")==0)
    transformPosition(rot90);
  else if (strcmp(tok,"180")==0)
    transformPosition(rot180);
  else if (strcmp(tok,"270")==0)
    transformPosition(rot270);
  else
    IoErrorMsg(UnrecRotMirr,0);

  if (LaTeXout)
  {
    sprintf(GlobalStr, "%s $%s^\\circ$", TwinningTab[TwinningRotate], tok);
    strcat(ActTwinning, GlobalStr);
  }

  StdString(TwinningTab[TwinningRotate]);
  StdString(" ");
  StdString(tok);

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
  if (!is_first)
  {
    if (LaTeXout)
      strcat(ActTwinning, ", ");
    StdString("\n   ");
  }
  StdString(CondLine);
  if (LaTeXout)
    strcat(ActTwinning, CondLine);
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
  while (sq2 == 0) {
    tok = ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == initsquare) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* issue the twinning */
  if (LaTeXout) {
    sprintf(GlobalStr, "%s %c%c$\\Rightarrow$%c%c",
            TwinningTab[TwinningShift],
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow,
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwinning, GlobalStr);
  }

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
    strcat(ActTwinning, TwinningTab[TwinningPolish]);

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
        strcat(ActTwinning, "{\\newline}");
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

    if (!TwinningRead)
    {
      if (continued)
      {
        StdChar('+');
        if (LaTeXout)
          strcat(ActTwinning, "+");
      }
      else
        TwinResetPosition();

      WriteTwinNumber();
    }
    else
    {
      StdString("  ");
      if (LaTeXout)
        strcat(ActTwinning, ", ");
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
        if (LaTeXout) {
          strcat(ActTwinning, AlphaStip);
          if (OptFlag[solapparent]) {
            strcat(ActTwinning, "*");
          }
          if (OptFlag[whitetoplay]) {
            char temp[10];        /* increased due to buffer overflow */
            sprintf(temp, " %c{\\ra}",
                    tolower(*PieSpString[UserLanguage][White]));
            strcat(ActTwinning, temp);
          }
        }
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
        if (LaTeXout) {
          strcat(ActTwinning, AlphaStip);
          if (OptFlag[solapparent]) {
            strcat(ActTwinning, "*");
          }
          if (OptFlag[whitetoplay]) {
            char temp[10];        /* increased due to buffer overflow */
            sprintf(temp, " %c{\\ra}",
                    tolower(*PieSpString[UserLanguage][White]));
            strcat(ActTwinning, temp);
          }
        }
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
            LaTeXClose();

            ReadToEndOfLine();
            LaTeXout = LaTeXOpen();

            if (SolFile!=NULL)
              fclose(SolFile);

            SolFile = tmpfile();
            if (SolFile==NULL)
              IoErrorMsg(WrOpenError,0);
            else
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

void WriteTwinNumber(void)
{
  if (TwinNumber-1<='z'-'a')
    sprintf(GlobalStr, "%c) ", 'a'+TwinNumber-1);
  else
    sprintf(GlobalStr, "z%u) ", (unsigned int)(TwinNumber-1-('z'-'a')));

  StdString(GlobalStr);
  if (LaTeXout)
    strcat(ActTwinning, GlobalStr);
}
