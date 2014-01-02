#include "input/plaintext/condition.h"
#include "input/plaintext/pieces.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/annan.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/bgl.h"
#include "conditions/circe/april.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/chameleon.h"
#include "conditions/football.h"
#include "conditions/geneva.h"
#include "conditions/grid.h"
#include "conditions/imitator.h"
#include "conditions/immune.h"
#include "conditions/isardam.h"
#include "conditions/kobul.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/koeko.h"
#include "conditions/madrasi.h"
#include "conditions/magic_square.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/messigny.h"
#include "conditions/oscillating_kings.h"
#include "conditions/protean.h"
#include "conditions/marscirce/phantom.h"
#include "conditions/republican.h"
#include "conditions/sat.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type1.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/woozles.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/castling.h"
#include "debugging/trace.h"
#include "pymsg.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long int ReadBGLNumber(char* inptr, char** endptr)
{
  /* input must be of form - | {d}d(.|,(d(d))) where d=digit ()=0 or 1 {}=0 or more
     in - and all other cases return infinity (no limit) */
  char buf[12];
  int res= BGL_infinity;
  size_t len, dp;
  char* dpp;
  *endptr= inptr;
  while (**endptr && strchr("0123456789.,-", **endptr))
    /* isdigit(**endptr) || **endptr == '.' || **endptr == ',' || **endptr == '-')) */
    (*endptr)++;
  len= (*endptr-inptr);
  if (len > 11)
    return res;
  strncpy(buf, inptr, len);
  buf[len]= '\0';
  if (len == 1 && buf[0] == '-')
    return res;
  for (dpp=buf; *dpp; dpp++)
    if (*dpp == ',')  /* allow 3,45 notation */
      *dpp= '.';
  for (dpp=buf; *dpp && *dpp != '.'; dpp++);
  dp= len-(dpp-buf);
  if (!dp)
    return 100*(long int)atoi(buf);
  while ((size_t)(dpp-buf) < len) {
    *dpp=*(dpp+1);
    dpp++;
  }
  for (dpp=buf; *dpp; dpp++)
    if (*dpp == '.')
      return res;  /* 2 d.p. characters */
  switch (dp) /* N.B> d.p. is part of count */
  {
  case 1 :
    return 100*(long int)atoi(buf);
  case 2 :
    return 10*(long int)atoi(buf);
  case 3 :
    return (long int)atoi(buf);
  default :
    return res;
  }
}

static char *ParseRex(boolean *rex, Cond what)
{
  char *tok = ReadNextTokStr();
  *rex = what==GetUniqIndex(CondCount,CondTab,tok);
  if (*rex)
    tok = ReadNextTokStr();
  return tok;
}

char *ReadSquares(SquareListContext context)
{
  char *tok = ReadNextTokStr();
  char *lastTok = tok;
  unsigned int nr_squares_read = 0;

  size_t const l = strlen(tok);
  if (l%2==1)
  {
    if (context!=ReadGrid)
      IoErrorMsg(WrongSquareList, 0);
    currentgridnum = 0;
    return tok;
  }

  while (*tok)
  {
    square const sq = SquareNum(*tok,tok[1]);
    if (sq==initsquare)
    {
      if (context==ReadGrid || nr_squares_read!=0)
      {
        currentgridnum = 0;
        return lastTok;
      }
      if (nr_squares_read!=0)
      {
        IoErrorMsg(WrongSquareList,0);
        return tok;
      }
    }
    else
    {
      switch (context)
      {
        case ReadImitators:
          isquare[nr_squares_read] = sq;
          break;

        case ReadHoles:
          block_square(sq);
          break;

        case ReadEpSquares:
        {
          move_effect_journal_index_type const base = move_effect_journal_base[nbply];
          move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

          switch (nr_squares_read)
          {
            case 0:
              move_effect_journal[movement].u.piece_movement.from = sq;
              break;

            case 1:
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            case 2:
              en_passant_remember_multistep_over(move_effect_journal[movement].u.piece_movement.to);
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            case 3:
              en_passant_remember_multistep_over(move_effect_journal[movement].u.piece_movement.to);
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            default:
              Message(TooManyEpKeySquares);
              break;
          }
          break;
        }

        case ReadBlRoyalSq:
          royal_square[Black]= sq;
          break;

        case ReadWhRoyalSq:
          royal_square[White]= sq;
          break;

        case ReadNoCastlingSquares:
          switch (sq)
          {
            case square_a1:
              CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(White*black_castling_offset));
              break;
            case square_e1:
              CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(White*black_castling_offset));
              break;
            case square_h1:
              CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(White*black_castling_offset));
              break;
            case square_a8:
              CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(Black*black_castling_offset));
              break;
            case square_e8:
              CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(Black*black_castling_offset));
              break;
            case square_h8:
              CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(Black*black_castling_offset));
              break;
            default:
              break;
          }
          break;

        case ReadGrid:
          ClearGridNum(sq);
          sq_spec[sq] += currentgridnum << Grid;
          break;

        default:
          SETFLAG(sq_spec[sq],context);
          break;
      }

      ++nr_squares_read;
      tok += 2;
    }
  }

  if (context==ReadImitators)
    number_of_imitators = nr_squares_read;

  return tok;
}

static Side OscillatingKingsSide;  /* TODO this is all a hack */

static char *ParseMaximumPawn(unsigned int *result,
                              unsigned int defaultVal,
                              unsigned int boundary)
{
  char *tok = ReadNextTokStr();

  char *end;
  unsigned long tmp = strtoul(tok,&end,10);
  if (tok==end || tmp>boundary)
    *result = defaultVal;
  else
    *result = tmp;

  return end;
}

static nocontactfunc_t *nocontactfunc_parsed;

typedef enum
{
  gpType,
  gpSentinelles,
  gpAntiCirce,
  gpKoeko,
  gpOsc,
  gpAnnan,
  gpGrid,
  gpRepublican,
  gpMagicSquare,
  gpColour
} VariantGroup;

static char *ParseVariant(boolean *is_variant_set, VariantGroup group)
{
  char    *tok=ReadNextTokStr();

  if (is_variant_set!=NULL && group != gpColour)
    *is_variant_set= false;

  do
  {
    VariantType type = GetUniqIndex(VariantTypeCount,VariantTypeTab,tok);

    if (type==VariantTypeCount)
      break;

    if (type>VariantTypeCount)
      IoErrorMsg(CondNotUniq,0);
    else if (type==TypeB && group==gpType)
      *is_variant_set= true;
    else if (type==TypeB && group==gpOsc)
      OscillatingKingsTypeB[OscillatingKingsSide]= true;
    else if (type==TypeC && group==gpOsc)
      OscillatingKingsTypeC[OscillatingKingsSide]= true;
    else if (type==TypeB && group==gpAnnan)
      annan_type= annan_type_B;
    else if (type==TypeC && group==gpAnnan)
      annan_type= annan_type_C;
    else if (type==TypeD && group==gpAnnan)
      annan_type= annan_type_D;
    else if (type==Type1 && group==gpType)
      SingleBoxType = singlebox_type1;
    else if (type==Type2 && group==gpType)
      SingleBoxType = singlebox_type2;
    else if (type==Type3 && group==gpType)
      SingleBoxType = singlebox_type3;
    else if (type==Type1 && group==gpRepublican)
      *is_variant_set = true;
    else if (type==Type2 && group==gpRepublican)
      *is_variant_set = false;
    else if (type==Type1 && group==gpMagicSquare)
      *is_variant_set = false;
    else if (type==Type2 && group==gpMagicSquare)
      *is_variant_set = true;
    else if (type==PionAdverse && group==gpSentinelles)
      *is_variant_set= true;
    else if (type==PionNeutral && group==gpSentinelles)
      SentPionNeutral= true;
    else if (type==PionNoirMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[Black],8,64);
    else if (type==PionBlancMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[White],8,64);
    else if (type==PionTotalMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns_total,16,64);
    else if (type==ParaSent && group==gpSentinelles)
      flagparasent= true;
    else if (type==SentBerolina && group==gpSentinelles)
      sentinelle = BerolinaPawn;
    else if (type==AntiCirceTypeCheylan && group==gpAntiCirce)
      *is_variant_set= true;
    else if (type==AntiCirceTypeCalvet && group==gpAntiCirce)
      *is_variant_set= false;
    else if (type==Neighbour && group==gpKoeko)
    {
      PieNam tmp_piece;
      tok = ReadNextTokStr();
      switch (strlen(tok))
      {
        case 1:
          tmp_piece= GetPieNamIndex(*tok,' ');
          break;
        case 2:
          tmp_piece= GetPieNamIndex(*tok,tok[1]);
          break;
        default:
          IoErrorMsg(WrongPieceName,0);
          return tok;
      }

      switch (tmp_piece)
      {
        case King:
          break;
        case Knight:
          *nocontactfunc_parsed= noknightcontact;
          break;
        case Wesir:
          *nocontactfunc_parsed= nowazircontact;
          break;
        case Fers:
          *nocontactfunc_parsed= noferscontact;
          break;
        case Camel:
          *nocontactfunc_parsed= nocamelcontact;
          break;
        case Alfil:
          *nocontactfunc_parsed= noalfilcontact;
          break;
        case Zebra:
          *nocontactfunc_parsed= nozebracontact;
          break;
        case Dabbaba:
          *nocontactfunc_parsed= nodabbabacontact;
          break;
        case Giraffe:
          *nocontactfunc_parsed= nogiraffecontact;
          break;
        case Antilope:
          *nocontactfunc_parsed= noantelopecontact;
          break;
        default:
          IoErrorMsg(WrongPieceName,0);
          break;
      }
    }
    else if (type==ShiftRank && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 8)/2)+4*((*bnp/24-7)/2)) << Grid;
      }
      grid_type = grid_vertical_shift;
    }
    else if (type==ShiftFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-8)/2)) << Grid;
      }
      grid_type = grid_horizontal_shift;
    }
    else if (type==ShiftRankFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-7)/2)) << Grid;
      }
      grid_type = grid_diagonal_shift;
    }
    else if (type==Orthogonal && group==gpGrid)
    {
      square const *bnp;
      int files[8], ranks[8];
      int filenum=1;
      int i;
      char c;
      tok = ReadNextTokStr();
      for (i=0; i<8; i++)
        files[i]=ranks[i]=0;
      while ((c=*tok++))
      {
        if (c >= '1' && c <= '8')
        {
          for (i=(c-'1')+1; i<8; i++)
            ranks[i]++;
        }
        else if (tolower(c) >= 'a' && tolower(c) <= 'h')
        {
          for (i=(tolower(c)-'a')+1; i<8; i++)
          files[i]++;
          filenum++;
        }
        else
        {
          IoErrorMsg(CondNotUniq, 0);
          return tok;
        }
      }
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (files[*bnp%24-8]+filenum*ranks[*bnp/24-8]) << Grid;
      }
      grid_type = grid_orthogonal_lines;
    }
    else if (type==Irregular && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      ClearGridNum(*bnp);
      grid_type = grid_irregular;
      currentgridnum=1;
      do
      {
        tok=ReadSquares(ReadGrid);
      }
      while (currentgridnum++);
      continue;
    }
    else if (type==ExtraGridLines && group==gpGrid)
    {
      boolean parsed= true;
      numgridlines= 0;
      while (parsed && numgridlines < 100)
      {
        tok = ReadNextTokStr();
        if (strlen(tok) == 4)
        {
          int f=0, r=0, l=0;
          boolean horiz=false;
          char c= tok[0];
          if (tolower(c) == 'h')
            horiz= true;
          else if (tolower(c) == 'v')
            horiz= false;
          else
            parsed= false;
          c= tok[1];
          if (tolower(c) >= 'a' && tolower(c) <= 'h')
            f= (tolower(c)-'a');
          else
            parsed= false;
          c= tok[2];
          if (c >= '1' && c <= '8')
            r=(c-'1');
          else
            parsed= false;
          c= tok[3];
          if (c >= '1' && c <= '8')
            l=(c-'0');
          else
            parsed= false;
          if (parsed)
          {
            gridlines[numgridlines][0]=2*f;
            gridlines[numgridlines][1]=2*r;
            gridlines[numgridlines][2]=2*f+(horiz?2*l:0);
            gridlines[numgridlines][3]=2*r+(horiz?0:2*l);
            numgridlines++;
            grid_type= grid_irregular;
          }
        }
        else
        {
          break;
        }
      }
      continue;
    }
    else if (group == gpColour)
    {
      if (type == WhiteOnly)
        is_variant_set[Black] = false;
      if (type == BlackOnly)
        is_variant_set[White] = false;
    }
    else {
      return tok;
    }
    tok = ReadNextTokStr();
  } while (group==gpSentinelles || group==gpGrid || group ==gpColour);

  return tok;
}

static char *ParseMummerStrictness(mummer_strictness_type *strictness)
{
  char *tok = ReadNextTokStr();

  if (mummer_strictness_ultra==GetUniqIndex(nr_mummer_strictness,mummer_strictness_tab,tok))
  {
    *strictness = mummer_strictness_ultra;
    tok = ReadNextTokStr();
  }
  else if (mummer_strictness_exact==GetUniqIndex(nr_mummer_strictness,mummer_strictness_tab,tok))
  {
    *strictness = mummer_strictness_exact;
    tok = ReadNextTokStr();
  }
  else
    *strictness = mummer_strictness_regular;

  return tok;
}

static char *ParseVaultingPieces(Side side)
{
  char  *tok;

  while (true)
  {
    tok = ReadNextTokStr();
    switch (strlen(tok))
    {
      case 1:
      {
        PieNam const p = GetPieNamIndex(*tok,' ');
        if (side!=Black)
          append_king_vaulter(White,p);
        if (side!=White)
          append_king_vaulter(Black,p);
        break;
      }

      case 2:
      {
        PieNam const p = GetPieNamIndex(*tok,tok[1]);
        if (side!=Black)
          append_king_vaulter(White,p);
        if (side!=White)
          append_king_vaulter(Black,p);
        break;
      }

      default:
        if (GetUniqIndex(VariantTypeCount,VariantTypeTab,tok)==Transmuting)
        {
          if (side!=Black)
            vaulting_kings_transmuting[White]= true;
          if (side!=White)
            vaulting_kings_transmuting[Black]= true;
        }
        else
          return tok;
        break;
    }
  }

  return tok;
}

static boolean handle_chameleon_reborn_piece(boolean *is_implicit,
                                             chameleon_sequence_type* sequence,
                                             PieNam from, PieNam to,
                                             char const *tok)
{
  boolean result;

  if (to==Empty)
  {
    IoErrorMsg(WrongPieceName,0);
    result = false;
  }
  else
  {
    if (from!=Empty)
      chameleon_set_successor_walk_explicit(is_implicit,sequence,from,to);
    result = true;
  }

  return result;
}

static char *ReadChameleonSequence(boolean *is_implicit,
                                   chameleon_sequence_type* sequence)
{
  PieNam from = Empty;

  while (true)
  {
    char *tok = ReadNextTokStr();
    switch (strlen(tok))
    {
      case 1:
      {
        PieNam const to = GetPieNamIndex(tok[0],' ');
        if (handle_chameleon_reborn_piece(is_implicit,sequence,from,to,tok))
          from = to;
        else
          return tok;
        break;
      }

      case 2:
      {
        PieNam const to = GetPieNamIndex(tok[0],tok[1]);
        if (handle_chameleon_reborn_piece(is_implicit,sequence,from,to,tok))
          from = to;
        else
          return tok;
        break;
      }

      default:
        return tok;
    }
  }

  return 0; /* avoid compiler warning */
}

static char *ReadPieces(int condition)
{
  PieNam tmp_piece;
  char  *tok;
  boolean   piece_read= false;

  fflush(stdout);
  while (true)
  {
    tok = ReadNextTokStr();
    switch (strlen(tok)) {
    case 1:
      tmp_piece= GetPieNamIndex(*tok,' ');
      piece_read= true;
      break;
    case 2:
      tmp_piece= GetPieNamIndex(*tok,tok[1]);
      piece_read= true;
      break;
    default:
      if (!piece_read && condition != football) {
        CondFlag[condition]= false;
        IoErrorMsg(WrongPieceName,0);
      }
      return tok;
    }
    if (!tmp_piece && condition != football) {
      IoErrorMsg(WrongPieceName,0);
      break;
    }
    switch (condition) {
    case promotiononly:
      promonly[tmp_piece]= true;
      break;
    case football:
      is_football_substitute[tmp_piece]= true;
      football_are_substitutes_limited = true;
      break;
    case april:
      is_april_kind[tmp_piece]= true;
      break;
    default:
      /* Never mind ... */
      break;
    }
  }
  return tok;
}

char *ParseCond(void)
{
  char    *tok, *ptr;
  unsigned int CondCnt = 0;

  tok = ReadNextTokStr();
  while (true)
  {
    Cond const indexx = GetUniqIndex(CondCount,CondTab,tok);
    TraceValue("%s",tok);
    TraceValue("%u\n",indexx);
    if (indexx==CondCount)
    {
      ExtraCond const extra = GetUniqIndex(ExtraCondCount,ExtraCondTab,tok);
      if (extra>ExtraCondCount)
      {
        IoErrorMsg(CondNotUniq,0);
        tok = ReadNextTokStr();
        break;
      }
      else if (extra==ExtraCondCount)
        break;
      else
      {
        ExtraCondFlag[extra] = true;

        switch (extra)
        {
          case maxi:
            tok = ParseMummerStrictness(&mummer_strictness_default_side);
            ++CondCnt;
            break;

          case ultraschachzwang:
            tok = ReadNextTokStr();
            ++CondCnt;
            break;

          default:
            assert(0);
            break;
        }

        continue;
      }
    }

    if (indexx>CondCount)
    {
      IoErrorMsg(CondNotUniq,0);
      tok = ReadNextTokStr();
      continue;
    }

    CondFlag[indexx]= true;

    CondCnt++;

    switch (indexx)
    {
      case hypervolage:
        CondFlag[volage]= true;
        break;
      case leofamily:
        CondFlag[chinoises]= true;
        break;
      case contactgrid:
        CondFlag[gridchess] = true;
        CondFlag[koeko] = true;
        break;
      case imitators:
        ReadSquares(ReadImitators);
        break;
      case blroyalsq:
        ReadSquares(ReadBlRoyalSq);
        break;
      case whroyalsq:
        ReadSquares(ReadWhRoyalSq);
        break;
      case magicsquare:
        magic_square_type = magic_square_type1;
        ReadSquares(MagicSq);
        break;
      case wormholes:
        ReadSquares(Wormhole);
        break;
      case dbltibet:
        CondFlag[tibet]= true;
        break;
      case holes:
        ReadSquares(ReadHoles);
        break;
      case trans_king:
        CondFlag[whtrans_king] = true;
        CondFlag[bltrans_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        break;
      case whtrans_king:
        calc_reflective_king[White]= true;
        break;
      case bltrans_king:
        calc_reflective_king[Black]= true;
        break;
      case whsupertrans_king:
        calc_reflective_king[White]= true;
        break;
      case blsupertrans_king:
        calc_reflective_king[Black]= true;
        break;
      case refl_king:
        CondFlag[whrefl_king] = true;
        CondFlag[blrefl_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        break;
      case whrefl_king:
        calc_reflective_king[White]= true;
        break;
      case blrefl_king:
        calc_reflective_king[Black]= true;
        break;
      case vault_king:
        CondFlag[whvault_king] = true;
        CondFlag[blvault_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        vaulting_kings_transmuting[White] = false;
        vaulting_kings_transmuting[Black] = false;
        break;
      case whvault_king:
        calc_reflective_king[White]= true;
        vaulting_kings_transmuting[White] = false;
        break;
      case blvault_king:
        calc_reflective_king[Black]= true;
        vaulting_kings_transmuting[Black] = false;
        break;
      case whforsqu:
        ReadSquares(WhForcedSq);
        break;
      case blforsqu:
        ReadSquares(BlForcedSq);
        break;
      case whconforsqu:
        ReadSquares(WhForcedSq);
        break;
      case blconforsqu:
        ReadSquares(BlForcedSq);
        break;

      /* different types of circe */
      case circe:
      case circemalefique:
      case circemalefiquevertical:
      case chamcirce:
      case circediametral:
      case frischauf:
      case circerank:
      case circefile:
      case circefilemalefique:
      case circediagramm:
      case circeassassin:
      case circeturncoats:
      case circedoubleagents:
        anycirce= true;
        break;
      case supercirce:
      case circecage:
      case circeequipollents:
      case circesymmetry:
      case circeantipoden:
      case circetakeandmake:
      case pwc:
      case couscous:
      case couscousmirror:
        anycirprom= true;
        anycirce= true;
        break;
      case parrain:
      case contraparrain:
        anycirce= true;
        anyparrain= true;
        break;
      case circeclone:
      case circeclonemalefique:
        anycirce= true;
        anyclone= true;
        break;

      case anti:
      case antispiegel:
      case antidiagramm:
      case antifile:
      case antisymmetrie:
      case antispiegelfile:
      case antisuper:
      case antiantipoden:
      case antiequipollents:
      case anticlonecirce:
        anyanticirce= true;
        break;

        /* different types of immunchess */
      case immun:
        anyimmun= true;
        break;
      case immunmalefique:
        immunrenai= renspiegel_polymorphic;
        anyimmun= true;
        break;
      case immunfile:
        immunrenai= renfile_polymorphic;
        anyimmun= true;
        break;
      case immundiagramm:
        immunrenai= rendiagramm_polymorphic;
        anyimmun= true;
        break;
      case immunspiegelfile:
        immunrenai= renspiegelfile_polymorphic;
        anyimmun= true;
        break;
      case immunsymmetry:
        immunrenai= rensymmetrie_polymorphic;
        anyimmun= true;
        break;
      case immunantipoden:
        immunrenai= renantipoden_polymorphic;
        anyimmun= true;
        break;
      case immunequipollents:
        immunrenai= renequipollents_polymorphic;
        anyimmun= true;
        break;

        /* different types of mars circe */
      case mars:
        marscirce_determine_rebirth_square= rennormal_polymorphic;
        anymars= true;
        break;
      case marsmirror:
        marscirce_determine_rebirth_square= renspiegel_polymorphic;
        anymars= true;
        break;
      case antimars:
        marscirce_determine_rebirth_square= rennormal_polymorphic;
        anyantimars= true;
        break;
      case antimarsmirror:
        marscirce_determine_rebirth_square= renspiegel_polymorphic;
        anyantimars= true;
        break;
      case antimarsantipodean:
        marscirce_determine_rebirth_square= renantipoden_polymorphic;
        anyantimars= true;
        break;
      case plus:
        marscirce_determine_rebirth_square= 0;
        anymars= true;
        break;

      case whprom_sq:
        ReadSquares(WhPromSq);
        break;
      case blprom_sq:
        ReadSquares(BlPromSq);
        break;

        /*****  different types of geneva chess   *****/
      case geneva:
        anygeneva= true;
        break;

      default:
        break;
    }

    switch (indexx)
    {
      case messigny:
        tok = ParseRex(&messigny_rex_exclusive, rexexcl);
        break;
      case woozles:
      case biwoozles:
      case heffalumps:
      case biheffalumps:
        tok = ParseRex(&woozles_rex_exclusive, rexexcl);
        break;
      case immun:
      case immunmalefique:
      case immundiagramm:
        tok = ParseRex(&immune_is_rex_inclusive, rexincl);
        break;
      case chamcirce:
        chameleon_reset_sequence(&chameleon_circe_is_squence_implicit,
                                 &chameleon_circe_walk_sequence);
        ReadChameleonSequence(&chameleon_circe_is_squence_implicit,
                              &chameleon_circe_walk_sequence);
        break;
      case chameleonsequence:
      case chamchess:
        ReadChameleonSequence(&chameleon_is_squence_implicit,
                              &chameleon_walk_sequence);
        break;
      case circe:
      case circemalefique:
      case circefile:
      case circeequipollents:
      case circediagramm:
      case circesymmetry:
      case circeclone:
      case circefilemalefique:
      case circeclonemalefique:
      case circeantipoden:
      case circemalefiquevertical:
      case circediametral:
      case circerank:
      case frischauf:
        tok = ParseRex(&circe_is_rex_inclusive, rexincl);
        break;
      case protean:
        tok = ParseRex(&protean_is_rex_exclusive, rexexcl);
        break;
      case phantom:
        tok = ParseRex(&phantom_chess_rex_inclusive,rexincl);
        break;
      case madras:
        tok = ParseRex(&madrasi_is_rex_inclusive, rexincl);
        break;
      case isardam:
        tok = ParseVariant(&IsardamB, gpType);
        break;
      case annan:
        annan_type = annan_type_A;
        tok = ParseVariant(NULL, gpAnnan);
        break;
      case kobulkings:
        kobulking[White] = kobulking[Black] = true;
        tok = ParseVariant(kobulking, gpColour);
        if (!kobulking[White] && !kobulking[Black])
          kobulking[White] = kobulking[Black] = true;
        break;
      case sentinelles:
        SentPionNeutral=false;
        tok = ParseVariant(&SentPionAdverse, gpSentinelles);
        break;

        /*****  exact-maxis  *****/
      case blmax:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmax:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blmin:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmin:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blcapt:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whcapt:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;

      case blconforsqu:
        mummer_strictness[Black] = mummer_strictness_ultra;
        tok = ReadNextTokStr();
        break;
      case whconforsqu:
        mummer_strictness[White] = mummer_strictness_ultra;
        tok = ReadNextTokStr();
        break;

      case blfollow:
      case blackalphabetic:
      case blacksynchron:
      case blackantisynchron:
      case blsupertrans_king:
      case blforsqu:
      case schwarzschacher:
        mummer_strictness[Black] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case whfollow:
      case whitealphabetic:
      case whitesynchron:
      case whiteantisynchron:
      case whsupertrans_king:
      case whforsqu:
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case alphabetic:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case duellist:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case losingchess:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        OptFlag[sansrn] = true;
        OptFlag[sansrb] = true;
        tok = ReadNextTokStr();
        break;

      case dynasty:
      case extinction:
        OptFlag[sansrn] = true;
        OptFlag[sansrb] = true;
        tok = ReadNextTokStr();
        break;

        /*****  anticirce type    *****/
      case anti:
      case antispiegel:
      case antidiagramm:
      case antifile:
      case antisymmetrie:
      case antispiegelfile:
      case antiantipoden:
      case antiequipollents:
      case antisuper:
      {
        boolean AntiCirCheylan;
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirceType = AntiCirCheylan ? AntiCirceTypeCheylan : AntiCirceTypeCalvet;
        break;
      }
      case singlebox:
        tok = ParseVariant(NULL, gpType);
        break;
      case republican:
      {
        boolean RepublicanType1 = false;
        tok = ParseVariant(&RepublicanType1, gpRepublican);
        RepublicanType = RepublicanType1 ? republican_type1 : republican_type2;
        break;
      }
      case magicsquare:
      {
        boolean MagicSquaresType2 = false;
        tok = ParseVariant(&MagicSquaresType2, gpMagicSquare);
        magic_square_type = MagicSquaresType2 ? magic_square_type2 : magic_square_type1;
        break;
      }
      case promotiononly:
        tok = ReadPieces(promotiononly);
        break;
      case football:
        football_are_substitutes_limited = false;
        tok = ReadPieces(football);
        break;
      case april:
        tok = ReadPieces(april);
        if (CondFlag[april])
        {
          anycirprom= true;
          anycirce= true;
        }
        break;
      case koeko:
        koeko_nocontact= &nokingcontact;
        nocontactfunc_parsed= &koeko_nocontact;
        tok = ParseVariant(NULL, gpKoeko);
        break;
      case antikoeko:
        antikoeko_nocontact= nokingcontact;
        nocontactfunc_parsed= &antikoeko_nocontact;
        tok = ParseVariant(NULL, gpKoeko);
        break;
      case white_oscillatingKs:
        OscillatingKingsSide= White;
        tok = ParseVariant(NULL, gpOsc);
        break;
      case black_oscillatingKs:
        OscillatingKingsSide= Black;
        tok = ParseVariant(NULL, gpOsc);
        break;
      case swappingkings:
        CondFlag[white_oscillatingKs]= true;
        OscillatingKingsTypeC[White]= true;
        CondFlag[black_oscillatingKs]= true;
        OscillatingKingsTypeC[Black]= true;
        tok = ReadNextTokStr();
        break;
      case SAT:
      case strictSAT:
        tok = ReadNextTokStr();
        SAT_max_nr_allowed_flights[White] = strtoul(tok,&ptr,10) + 1;
        if (tok == ptr) {
          SAT_max_nr_allowed_flights[White]= 1;
          SAT_max_nr_allowed_flights[Black]= 1;
          break;
        }
        tok = ReadNextTokStr();
        SAT_max_nr_allowed_flights[Black] = strtoul(tok,&ptr,10) + 1;
        if (tok == ptr)
          SAT_max_nr_allowed_flights[Black]= SAT_max_nr_allowed_flights[White];
        break;
      case BGL:
        BGL_global= false;
        tok = ReadNextTokStr();
        BGL_values[White] = ReadBGLNumber(tok,&ptr);
        if (tok == ptr)
        {
          BGL_values[White] = BGL_infinity;
          BGL_values[Black] = BGL_infinity;
          return tok;
        }
        else
        {
          tok = ReadNextTokStr();
          BGL_values[Black]= ReadBGLNumber(tok,&ptr);
          if (tok == ptr)
          {
            BGL_values[Black] = BGL_values[White];
            BGL_global= true;
            return tok;
          }
        }
        tok = ReadNextTokStr();
        break;
      case geneva:
        tok = ParseRex(&rex_geneva, rexincl);
        break;
      case whvault_king:
        tok = ParseVaultingPieces(White);
        break;
      case blvault_king:
        tok = ParseVaultingPieces(Black);
        break;
      case vault_king:
        tok = ParseVaultingPieces(no_side);
        break;
      case gridchess:
        tok = ParseVariant(NULL, gpGrid);
        break;
      default:
        tok = ReadNextTokStr();
        break;
    }
  }

  if (CondCnt==0)
    IoErrorMsg(UnrecCondition,0);

  return tok;
}

void InitCond(void)
{
  square const *bnp;
  square i, j;

  mummer_strictness[White] = mummer_strictness_none;
  mummer_strictness[Black] = mummer_strictness_none;

  anyclone = false;
  anycirprom = false;
  anycirce = false;
  anyimmun = false;
  anyanticirce = false;
  anymars = false;
  anyantimars = false;
  anygeneva = false;
  anyparrain= false;

  AntiCirceType = VariantTypeCount;

  immunrenai = rennormal_polymorphic;
  marscirce_determine_rebirth_square = rennormal_polymorphic;

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  CondFlag[circeassassin]= false;
  flagparasent= false;
  madrasi_is_rex_inclusive = false;
  circe_is_rex_inclusive = false;
  immune_is_rex_inclusive = false;
  phantom_chess_rex_inclusive = false;
  rex_geneva =false;
  messigny_rex_exclusive = false;
  woozles_rex_exclusive = false;
  protean_is_rex_exclusive = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle = Pawn;

  grid_type = grid_normal;
  numgridlines = 0;

  {
    PieceIdType id;
    for (id = MinPieceId; id<=MaxPieceId; ++id)
      PiecePositionsInDiagram[id] = initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
    int const row= *bnp/onerow - nr_of_slack_rows_below_board;

    ClearPieceId(spec[*bnp]);
    CLEARFL(sq_spec[*bnp]);
    sq_num[*bnp]= (int)(bnp-boardnum);

    /* initialise sq_spec and set grid number */
    sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
    if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= square_a1; i < square_h8; i+= onerow)
  {
    if (i > square_a1)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
        SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
        SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; ++i)
    CondFlag[i] = false;

  for (i= 0; i < ExtraCondCount; ++i)
    ExtraCondFlag[i] = false;

  number_of_imitators = 0;

  memset((char *) promonly, 0, sizeof(promonly));
  memset((char *) is_football_substitute, 0, sizeof(promonly));
  memset((char *) is_april_kind,0,sizeof(is_april_kind));
  koeko_nocontact= nokingcontact;
  antikoeko_nocontact= nokingcontact;
  OscillatingKingsTypeB[White]= false;
  OscillatingKingsTypeB[Black]= false;
  OscillatingKingsTypeC[White]= false;
  OscillatingKingsTypeC[Black]= false;

  BGL_values[White] = BGL_infinity;
  BGL_values[Black] = BGL_infinity;
  BGL_global= false;

  calc_reflective_king[White] = false;
  calc_reflective_king[Black] = false;

  reset_king_vaulters();

  kobulking[White] = false;
  kobulking[Black] = false;
} /* InitCond */
