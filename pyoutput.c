#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyint.h"
#include "pyslice.h"
#include "pypipe.h"
#include "py1.h"
#include "conditions/republican.h"
#include "optimisations/maxsolutions/maxsolutions.h"
#include "trace.h"
#ifdef _SE_
#include "se.h"
#endif 

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

static output_mode current_mode = output_mode_none;

static unsigned int nr_color_inversions_in_ply[maxply];
static unsigned int nr_color_inversions;

slice_index active_slice[maxply];

static boolean is_threat[maxply];

static unsigned int nr_moves_written[maxply];

static attack_type pending_decoration = attack_regular;


void set_output_mode(output_mode mode)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(output_mode,mode,"");
  TraceFunctionParamListEnd();

  current_mode = mode;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_treemode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = output_mode_tree;
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_linemode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = output_mode_line;
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_binary(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;
  output_mode mode1;
  output_mode mode2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  mode1 = *mode;

  stip_traverse_structure(slices[si].u.binary.op2,st);
  mode2 = *mode;

  *mode = mode2==output_mode_none ? mode1 : mode2;
  
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_self_defense(slice_index si,
                                     stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length_battle)
    *mode = output_mode_tree;
  else
    stip_traverse_structure(slices[si].u.branch.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const output_mode_detectors[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &output_mode_linemode,             /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &output_mode_linemode,             /* STSeriesFork */
  &stip_traverse_structure_children, /* STLeafDirect */
  &stip_traverse_structure_children, /* STLeafHelp */
  &stip_traverse_structure_children, /* STLeafForced */
  &output_mode_binary,               /* STReciprocal */
  &output_mode_binary,               /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &output_mode_treemode,             /* STAttackRoot */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STRefutationsWriter */
  &stip_traverse_structure_children, /* STThreatWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &pipe_traverse_next,               /* STSelfCheckGuardRootSolvableFilter */
  &pipe_traverse_next,               /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &pipe_traverse_next,               /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &output_mode_treemode,             /* STDirectDefenderFilter */
  &pipe_traverse_next,               /* STReflexHelpFilter */
  &pipe_traverse_next,               /* STReflexSeriesFilter */
  &output_mode_treemode,             /* STReflexAttackerFilter */
  &output_mode_treemode,             /* STReflexDefenderFilter */
  &output_mode_self_defense,         /* STSelfDefense */
  &pipe_traverse_next,               /* STRestartGuardRootDefenderFilter */
  &pipe_traverse_next,               /* STRestartGuardHelpFilter */
  &pipe_traverse_next,               /* STRestartGuardSeriesFilter */
  &pipe_traverse_next,               /* STIntelligentHelpFilter */
  &pipe_traverse_next,               /* STIntelligentSeriesFilter */
  &pipe_traverse_next,               /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &output_mode_treemode,             /* STKeepMatingGuardAttackerFilter */
  &output_mode_treemode,             /* STKeepMatingGuardDefenderFilter */
  &pipe_traverse_next,               /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &pipe_traverse_next,               /* STMaxFlightsquares */
  &pipe_traverse_next,               /* STDegenerateTree */
  &pipe_traverse_next,               /* STMaxNrNonTrivial */
  &pipe_traverse_next,               /* STMaxNrNonTrivialCounter */
  &pipe_traverse_next,               /* STMaxThreatLength */
  &output_mode_treemode,             /* STMaxTimeRootDefenderFilter */
  &output_mode_treemode,             /* STMaxTimeDefenderFilter */
  &pipe_traverse_next,               /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &pipe_traverse_next,               /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_traverse_next,               /* STStopOnShortSolutionsHelpFilter */
  &pipe_traverse_next                /* STStopOnShortSolutionsSeriesFilter */
};

/* Initialize based on the stipulation
 */
void init_output(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  current_mode = output_mode_none;
  
  stip_structure_traversal_init(&st,&output_mode_detectors,&current_mode);
  stip_traverse_structure(si,&st);

  TraceEnumerator(output_mode,current_mode,"\n");
  
  if (current_mode==output_mode_tree)
    nr_moves_written[nbply] = 0;
  else
    current_mode = output_mode_line;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a possibly pending move decoration
 */
static void write_pending_decoration(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (pending_decoration)
  {
    case attack_try:
      StdString(" ?");
      break;

    case attack_key:
      StdString(" !");
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
      break;

    default:
      break;
  }

  pending_decoration = attack_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Start a new output level consisting of set play
 */
void output_start_move_inverted_level(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nr_color_inversions;
  ++nr_color_inversions_in_ply[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the inner-most output level (which consists of set play)
 */
void output_end_move_inverted_level(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#ifdef _SE_DECORATE_SOLUTION_
  se_end_set_play();   
#endif

  --nr_color_inversions_in_ply[nbply+1];
  --nr_color_inversions;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_end_half_duplex(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#ifdef _SE_DECORATE_SOLUTION_
  se_end_half_duplex();  
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Start a new output level consisting of threats
 */
void output_start_threat_level(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    TraceValue("%u",nbply);
    TraceValue("%u\n",nr_moves_written[nbply]);
    if (nr_moves_written[nbply-1]==0)
      /* option postkey is set - write "threat:" or "zugzwang" on a
       * new line
       */
      Message(NewLine);

    /* nbply will be increased by genmove() in a moment */
    is_threat[nbply+1] = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the inner-most output level (which consists of threats)
 */
void output_end_threat_level(slice_index si, boolean is_zugzwang)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    if (is_zugzwang)
    {
      write_pending_decoration();
      StdChar(blank);
      Message(Zugzwang);
    }

    is_threat[nbply+1] = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Start a new output level consisting of regular continuations
 */
void output_start_continuation_level(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    if (nbply>1
        && encore()
        && echecc(nbply,slices[si].starter))
    {
      StdString(" +");
      StdChar(blank);
    }

    /* nbply will be increased by genmove() in a moment */
    nr_moves_written[nbply+1] = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the inner-most output level (which consists of regular
 * continuations)
 */
void output_end_continuation_level(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Start a new output level for defenses
 */
void output_start_defense_level(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    if (echecc(nbply,slices[si].starter))
      StdString(" +");

    write_pending_decoration();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the inner-most output level (which consists of defenses)
 */
void output_end_defense_level(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void linesolution(void)
{
  int next_movenumber = 1;
  Side starting_side;
  Goal end_marker;
  slice_index slice;
  ply current_ply;

  ply const start_ply = 2;

  if (isIntelligentModeActive)
  {
    if (SolAlreadyFound())
      return;
    else
    {
      increase_nr_found_solutions();
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }
    StoreSol();
  }
  else
  {
    increase_nr_found_solutions();
    if (OptFlag[beep])
      BeepOnSolution(maxbeep);
  }
      
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  Message(NewLine);

  slice = active_slice[start_ply];
  starting_side = slices[root_slice].starter;

  ResetPosition();

  TraceValue("%u\n",nr_color_inversions_in_ply[start_ply]);

  switch (nr_color_inversions_in_ply[start_ply])
  {
    case 2:
      StdString("  1...  ...");
      next_movenumber = 2;
      break;

    case 1:
      StdString("  1...");
      next_movenumber = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  TraceValue("%u\n",nbply);
  for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
  {
    TraceValue("%u",current_ply);
    TraceValue("%u",slice);
    TraceValue("%u\n",active_slice[current_ply]);
    if (slice!=active_slice[current_ply])
    {
      if (slices[slice].type==STSeriesMove
          && slices[active_slice[current_ply]].type==STSeriesMove
          && trait[current_ply-1]!=trait[current_ply])
      {
        next_movenumber = 1;
        starting_side = trait[current_ply];
      }

      slice = active_slice[current_ply];
    }

    TraceEnumerator(Side,starting_side," ");
    TraceEnumerator(Side,trait[current_ply],"\n");
    if (trait[current_ply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }

    end_marker = (nbply==current_ply
                  ? slices[active_slice[current_ply]].u.leaf.goal
                  : no_goal);
    TraceValue("%u\n",end_marker);
    initneutre(advers(trait[current_ply]));
    jouecoup_no_test(current_ply);
    ecritcoup(current_ply);
    if (end_marker==no_goal)
    {
      if (echecc(current_ply,advers(trait[current_ply])))
        StdString(" +");
    }
    else
      StdString(goal_end_marker[end_marker]);
    StdChar(blank);
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_end_pos();
#endif
#ifdef _SE_FORSYTH_
  se_forsyth();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the decoration (! or ?) for the first move if appropriate
 * @param current_ply identifies ply in which move was played
 * @param type identifies decoration to be added
 */
void write_battle_move_decoration(ply current_ply, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  pending_decoration = type;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a move in battle play
 */
void write_battle_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    unsigned int const move_depth = nbply+nr_color_inversions;

    write_pending_decoration();

    if (is_threat[nbply] && nr_moves_written[nbply]==0)
    {
      StdChar(blank);
      Message(Threat);
    }

    Message(NewLine);

    sprintf(GlobalStr,"%*c%3u.",4*move_depth-8,blank,move_depth/2);
    StdString(GlobalStr);
    if (move_depth%2==1)
    {
      sprintf(GlobalStr,"..");
      StdString(GlobalStr);
    }

    ecritcoup(nbply);

    ++nr_moves_written[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void write_goal(Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  assert(goal!=no_goal);

  if (current_mode==output_mode_tree)
    StdString(goal_end_marker[goal]);
  else
    linesolution();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Mark the defense about to be written as refutation
 */
void write_refutation_mark(void)
{
  unsigned int const move_depth = nbply+nr_color_inversions;

  Message(NewLine);
  sprintf(GlobalStr,"%*c",4*move_depth-4,blank);
  StdString(GlobalStr);
  Message(Refutation);
}

/* Write the end of a solution
 */
void write_end_of_solution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
  {
    write_pending_decoration();
    Message(NewLine);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the end of a solution phase
 */
void write_end_of_solution_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  Message(NewLine);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void editcoup(ply ply_id, coup *mov)
{
  char    BlackChar= *GetMsgString(BlackColor);
  char    WhiteChar= *GetMsgString(WhiteColor);
#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (mov->cazz==nullsquare) return;

  /* Did we castle ?? */
  if (mov->cpzz == kingside_castling
      || mov->cpzz == queenside_castling)
  {
    /* castling */
    StdString("0-0");
    if (mov->cpzz == queenside_castling) {
      StdString("-0");
    }
    if (CondFlag[einstein]) {
      StdChar('=');
      if (CondFlag[reveinstein])
        WritePiece(db);
      else
        WritePiece(fb);
    }
  } else {  /* no, we didn't castle */
    if (mov->cpzz == messigny_exchange) {
      WritePiece(mov->pjzz);
      WriteSquare(mov->cdzz);
      StdString("<->");
      WritePiece(mov->ppri);
      WriteSquare(mov->cazz);
    }
    else {
      if (mov->sb3what!=vide) {
        StdString("[");
        WriteSquare(mov->sb3where);
        StdString("=");
        WritePiece(mov->sb3what);
        StdString("]");
      }
      if (WriteSpec(mov->speci, false)
          || (mov->pjzz != pb && mov->pjzz != pn))
        WritePiece(mov->pjzz);

      WriteSquare(mov->cdzz);
      if (anyantimars && (mov->ppri == vide || mov->cdzz == mov->cpzz))
      {
        StdString("->");
        WriteSquare(mov->mren);
      }
      if (mov->ppri == vide || (anyantimars && mov->cdzz == mov->cpzz))
        StdChar('-');
      else
        StdChar('*');

      if (mov->cpzz != mov->cazz && mov->roch_sq == initsquare) {
        if (is_pawn(mov->pjzz) && !CondFlag[takemake]) {
          WriteSquare(mov->cazz);
          StdString(" ep.");
        }
        else {
          WriteSquare(mov->cpzz);
          StdChar('-');
          WriteSquare(mov->cazz);
        }
      }
      else {
        WriteSquare(mov->cazz);
      }
    }

    if (mov->bool_norm_cham_prom) {
      SETFLAG(mov->speci, Chameleon);
    }

    if ((mov->pjzz != mov->pjazz)
        || ((mov->speci != mov->new_spec) && (mov->new_spec != 0)))
    {
      if (mov->pjazz == vide) {
        if (mov->promi) {
          StdString ("=I");
        }
      }
      else if (!((CondFlag[white_oscillatingKs] && mov->tr == White && mov->pjzz == roib) ||
                 (CondFlag[black_oscillatingKs] && mov->tr == Black && mov->pjzz == roin))) {
        StdChar('=');
        WriteSpec(mov->new_spec, mov->speci != mov->new_spec);
        WritePiece(mov->pjazz);
      }
    }

    if (mov->roch_sq != initsquare) {
      StdChar('/');
      WriteSpec(mov->roch_sp, true);
      WritePiece(mov->roch_pc);
      WriteSquare(mov->roch_sq);
      StdChar('-');
      WriteSquare((mov->cdzz + mov->cazz) / 2);
    }

    if (mov->sqren != initsquare) {
      piece   p= CondFlag[antieinstein]
          ? inc_einstein(mov->ppri)
          : CondFlag[parrain]
          ? mov->ren_parrain
          : CondFlag[chamcirce]
          ? ChamCircePiece(mov->ppri)
          : (anyclone && abs(mov->pjzz) != roib)
          ? -mov->pjzz
          : (anytraitor && abs(mov->ppri) >= roib)
          ? -mov->ppri
          : mov->ppri;
      StdString(" [+");
      WriteSpec(mov->ren_spec, p!=vide);
      WritePiece(p);

      WriteSquare(mov->sqren);
      if (mov->bool_cir_cham_prom) {
        SETFLAG(mov->ren_spec, Chameleon);
      }
      if (mov->cir_prom) {
        StdChar('=');
        WriteSpec(mov->ren_spec, p!=vide);
        WritePiece(mov->cir_prom);
      }

      if (TSTFLAG(mov->ren_spec, Volage)
          && SquareCol(mov->cpzz) != SquareCol(mov->sqren))
      {
        sprintf(GlobalStr, "=(%c)",
                (mov->tr == White) ? WhiteChar : BlackChar);
        StdString(GlobalStr);
      }
      StdChar(']');
    }

    if (mov->sb2where!=initsquare) {
      assert(mov->sb2what!=vide);
      StdString(" [");
      WriteSquare(mov->sb2where);
      StdString("=");
      WritePiece(mov->sb2what);
      StdString("]");
    }

    if (CondFlag[republican])
      write_republican_king_placement(mov);

    if (mov->renkam) {
      StdChar('[');
      WriteSpec(mov->speci, mov->pjazz != vide);
      WritePiece(mov->pjazz);
      WriteSquare(mov->cazz);
      StdString("->");
      WriteSquare(mov->renkam);
      if (mov->norm_prom != vide &&
          (!anyanticirce || (CondFlag[antisuper] && 
                             ((is_forwardpawn(mov->pjzz)
                               && !PromSq(mov->tr, mov->cazz)) || 
                              (is_reversepawn(mov->pjzz)
                               && !ReversePromSq(mov->tr, mov->cazz)))))) {
        StdChar('=');
        WriteSpec(mov->speci, true);
        WritePiece(mov->norm_prom);
      }
      StdChar(']');
    }
    if (mov->bool_senti) {
      StdString("[+");
      StdChar((!SentPionNeutral || !TSTFLAG(mov->speci, Neutral))
              ?  ((mov->tr==White) != SentPionAdverse
                  ? WhiteChar
                  : BlackChar)
              : 'n');
      WritePiece(sentinelb); WriteSquare(mov->cdzz);
      StdChar(']');
    }
    if (TSTFLAG(mov->speci, ColourChange)
        && (abs(e[mov->hurdle])>roib))
    {
      Side hc= e[mov->hurdle] < vide ? Black : White;
      StdString("[");
      WriteSquare(mov->hurdle);
      StdString("=");
      StdChar(hc == White ? WhiteChar : BlackChar);
      StdString("]");
    }
    if (flag_outputmultiplecolourchanges)
    {
      if (mov->push_bottom != NULL) {

        if (mov->push_top - mov->push_bottom > 0) 
        {
          change_rec const * rec;
          StdString(" [");
          for (rec= mov->push_bottom; rec - mov->push_top < 0; rec++)
          {
            StdChar(rec->pc > vide ? WhiteChar : BlackChar);
            WritePiece(rec->pc);
            WriteSquare(rec->square);
            if (mov->push_top - rec > 1)
              StdString(", ");
          } 
          StdChar(']');
        }

      } else {

        if (colour_change_sp[ply_id] > colour_change_sp[parent_ply[ply_id]]) 
        {
          change_rec const * rec;
          StdString(" [");
          for (rec = colour_change_sp[parent_ply[ply_id]];
               rec<colour_change_sp[ply_id];
               rec++)
          {
            StdChar(rec->pc > vide ? WhiteChar : BlackChar);
            WritePiece(rec->pc);
            WriteSquare(rec->square);
            if (colour_change_sp[ply_id]-rec > 1)
              StdString(", ");
          } 
          StdChar(']');
        }

      }
    }

  } /* No castling */

  if (mov->numi && CondFlag[imitators])
  {
    unsigned int icount;
    int const diff = im0-isquare[0];
    StdChar('[');
    for (icount = 1; icount<=mov->numi; icount++)
    {
      StdChar('I');
      WriteSquare(isquare[icount-1] + mov->sum + diff);
      if (icount<mov->numi)
        StdChar(',');
    }
    StdChar(']');
  }
  if (mov->osc) {
    StdString("[");
    StdChar(WhiteChar);
    WritePiece(roib);
    StdString("<>");
    StdChar(BlackChar);
    WritePiece(roib);
    StdString("]");
  }

  if ((CondFlag[ghostchess] || CondFlag[hauntedchess])
      && mov->ghost_piece!=vide)
  {
    StdString("[+");
    WriteSpec(mov->ghost_flags, mov->ghost_piece != vide);
    WritePiece(mov->ghost_piece);
    WriteSquare(mov->cdzz);
    StdString("]");
  }
  
  if (CondFlag[BGL])
  {
    char s[30], buf1[12], buf2[12];
    if (BGL_global)
      sprintf(s, " (%s)", WriteBGLNumber(buf1, BGL_white));
    else
      sprintf(s, " (%s/%s)",
              WriteBGLNumber(buf1, BGL_white),
              WriteBGLNumber(buf2, BGL_black));
    StdString(s);
  }
} /* editcoup */

void ecritcoup(ply ply_id)
{
  coup mov;
  current(ply_id,&mov);
  editcoup(ply_id,&mov);
}

/* Write a move as a refutation
 * @param c address of the structure representing the move
 */
static void write_refutation(coup *c)
{
  StdString("\n      1...");
  editcoup(nbply,c);
  if (c->echec)
    StdString(" +");
  StdChar(blank);
  StdString(" !");
}

/* Write the refutations stored in a table
 * @param refutations table containing refutations
 */
void write_refutations(table refutations)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (table_length(refutations)!=0)
  {
    Message(NewLine);
    sprintf(GlobalStr,"%*c",4,blank);
    StdString(GlobalStr);
    Message(But);
    table_iterate(refutations,&write_refutation);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
