#include "trace.h"
#include "pyproc.h"
#include "pydata.h"
#include "pyhash.h"
#include "pystip.h"
#include "pyoutput.h"
#include "output/plaintext/plaintext.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOTRACE)

static trace_level level;

static unsigned long move_counter;

trace_level max_level = ULONG_MAX;

static char const *entryNames[1000];

void TraceSetMaxLevel(trace_level tl)
{
  max_level = tl;
}

void TraceFunctionEntry(char const *name)
{
  ++level;
    
  if (level<=max_level)
  {
    fprintf(stdout,"> #%lu %s ",level,name);
    fflush(stdout);
  }

  entryNames[level] = name;
}

void TraceFunctionParamListEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");
}

void TraceFunctionExit(char const *name)
{
  if (strcmp(name,entryNames[level])!=0)
    fprintf(stderr,"Level:%lu Expected: %s. Got: %s\n",
            level,entryNames[level],name);
    
  assert(strcmp(name,entryNames[level])==0);

  if (level<=max_level)
  {
    fprintf(stdout,"< #%lu %s",level,name);
    fflush(stdout);
  }

  --level;
}

void TraceFunctionResultImpl(char const *format, size_t value)
{
  if (level+1<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }
}

void TraceFunctionResultEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");
}

void TraceValueImpl(char const *format, size_t value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }
}

void TracePointerValueImpl(char const *format, void const *value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }
}

void TraceText(char const *text)
{
  if (level<=max_level)
    fprintf(stdout,"  #%lu %s",level,text);
}

void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value)
{
  if (level<=max_level)
    fprintf(stdout,format,enumerator_name,value);
}

void TraceSquareImpl(char const *prefix, square s)
{
  if (level<=max_level)
  {
    fprintf(stdout,"%s",prefix);

    if (s==initsquare)
      printf("initsquare");
    else
      WriteSquare(s);
    fflush(stdout);
  }
}

void TracePieceImpl(char const *prefix, piece p)
{
  if (level<=max_level)
  {
    fprintf(stdout,"%s",prefix);

    if (p==vide)
      printf("vide");
    else if (p==obs)
      printf("obs");
    else
      WritePiece(p);
    fflush(stdout);
  }
}

boolean TraceCurrentMove(ply current_ply)
{
  if (level<=max_level)
  {
    fprintf(stdout," #%lu %lu ",level,move_counter++);
    output_plaintext_write_move(current_ply);
    fprintf(stdout," nbcou:%d",nbcou);
    fprintf(stdout," current_ply:%d\n",current_ply);
    fflush(stdout);
  }

  return true;
}

void TraceCurrentHashBuffer(void)
{
  if (level<=max_level)
  {
    HashBuffer const *hb = &hashBuffers[nbply];
    unsigned int i;

    fprintf(stdout," #%lu nbply:%u Leng:%u ",level,nbply,hb->cmv.Leng);
    for (i = 0; i<hb->cmv.Leng; ++i)
      fprintf(stdout,"%02x ",(unsigned int)hb->cmv.Data[i]);
    fprintf(stdout,"\n");
    fflush(stdout);
  }
}

void TracePosition(echiquier e, Flags flags[maxsquare+4])
{
  if (level<=max_level)
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (e[*bnp]!=vide && e[*bnp]!=obs)
      {
        WriteSpec(spec[*bnp],true);
        WritePiece(abs(e[*bnp]));
        WriteSquare(*bnp);
        fprintf(stdout," ");
      }

    fprintf(stdout,"\n");
    fflush(stdout);
  }
}

static void Trace_link(char const *prefix, slice_index si, char const *suffix)
{
  if (si==no_slice)
    fprintf(stdout,"%s--%s ",prefix,suffix);
  else
    fprintf(stdout,"%s%2u%s ",prefix,si,suffix);
}

static void Trace_slice(slice_index si)
{
  Trace_link("",slices[si].prev,"<");
}

static void Trace_pipe(slice_index si)
{
  Trace_slice(si);
  Trace_link(">",slices[si].u.pipe.next,"");
}

static void Trace_branch(slice_index si)
{
  Trace_pipe(si);
  fprintf(stdout,"%2u/",slices[si].u.branch.length);
  fprintf(stdout,"%2u ",slices[si].u.branch.min_length);
}

static void TraceStipulationRecursive(slice_index si, boolean done_slices[])
{
  if (si!=no_slice && !done_slices[si])
  {
    done_slices[si] = true;

    fprintf(stdout,"[%2u]: ",si);
    fprintf(stdout,"%-34s ",SliceType_names[slices[si].type]);
    fprintf(stdout,"%s ",Side_names[slices[si].starter]);
    switch (slices[si].type)
    {
      case STDirectDefenderFilter:
      case STSelfDefense:
      case STAttackFork:
      case STDefenseFork:
      case STHelpFork:
      case STSeriesFork:
        Trace_branch(si);
        Trace_link("2goal:",slices[si].u.branch_fork.towards_goal,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.branch_fork.towards_goal,
                                  done_slices);
        break;

      case STDoubleMateAttackerFilter:
      case STDoubleMateHelpFilter:
      case STDoubleMateSeriesFilter:
      case STCounterMateAttackerFilter:
      case STCounterMateHelpFilter:
      case STCounterMateSeriesFilter:
      case STEnPassantAttackerFilter:
      case STEnPassantDefenderFilter:
      case STEnPassantHelpFilter:
      case STCastlingAttackerFilter:
      case STCastlingHelpFilter:
      case STCastlingSeriesFilter:
      case STAttackRoot:
      case STDefenseRoot:
      case STAttackEnd:
      case STAttackMove:
      case STAttackMoveToGoal:
      case STContinuationSolver:
      case STBattlePlaySolver:
      case STDefenseEnd:
      case STDefenseMove:
      case STKillerMoveFinalDefenseMove:
      case STRefutationsCollector:
      case STSelfCheckGuardAttackerFilter:
      case STSelfCheckGuardDefenderFilter:
      case STSelfCheckGuardHelpFilter:
      case STSelfCheckGuardSeriesFilter:
      case STVariationWriter:
      case STRefutingVariationWriter:
      case STAttackHashed:
      case STHelpRoot:
      case STHelpMove:
      case STHelpMoveToGoal:
      case STSeriesRoot:
      case STSeriesMove:
      case STSeriesMoveToGoal:
      case STHelpHashed:
      case STSeriesHashed:
      case STIntelligentHelpFilter:
      case STIntelligentSeriesFilter:
      case STGoalReachableGuardHelpFilter:
      case STGoalReachableGuardSeriesFilter:
      case STStopOnShortSolutionsHelpFilter:
      case STStopOnShortSolutionsSeriesFilter:
      case STOutputPlaintextTreeCheckDetectorAttackerFilter:
      case STOutputPlaintextTreeCheckDetectorDefenderFilter:
        Trace_branch(si);
        if (slices[si].u.branch.imminent_goal.type!=no_goal)
        {
          fprintf(stdout,"imminent:%2u ",slices[si].u.branch.imminent_goal.type);
          if (slices[si].u.branch.imminent_goal.type==goal_target)
            TraceSquare(slices[si].u.branch.imminent_goal.target);
        }
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STThreatSolver:
        Trace_branch(si);
        Trace_link("attack_side:",slices[si].u.threat_writer.defense_move,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STHelpShortcut:
      case STSeriesShortcut:
        Trace_branch(si);
        Trace_link("short_sols:",slices[si].u.shortcut.short_sols,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.shortcut.next,done_slices);
        TraceStipulationRecursive(slices[si].u.shortcut.short_sols,done_slices);
        break;

      case STKeepMatingGuardAttackerFilter:
      case STKeepMatingGuardDefenderFilter:
      case STKeepMatingGuardHelpFilter:
      case STKeepMatingGuardSeriesFilter:
        Trace_pipe(si);
        fprintf(stdout,"mating:%s ",
                Side_names[slices[si].u.keepmating_guard.mating]);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.keepmating_guard.next,
                                  done_slices);
        break;

      case STSelfCheckGuardRootSolvableFilter:
      case STSelfCheckGuardSolvableFilter:
      case STProxy:
      case STMoveInverterRootSolvableFilter:
      case STMoveInverterSolvableFilter:
      case STMoveInverterSeriesFilter:
      case STNot:
      case STPostKeyPlaySuppressor:
      case STBattlePlaySolutionWriter:
      case STContinuationWriter:
      case STThreatEnforcer:
      case STThreatCollector:
      case STZugzwangWriter:
      case STRestartGuardRootDefenderFilter:
      case STRestartGuardHelpFilter:
      case STRestartGuardSeriesFilter:
      case STIntelligentDuplicateAvoider:
      case STMaxTimeRootDefenderFilter:
      case STMaxTimeDefenderFilter:
      case STMaxTimeHelpFilter:
      case STMaxTimeSeriesFilter:
      case STMaxSolutionsRootSolvableFilter:
      case STMaxSolutionsSolvableFilter:
      case STMaxSolutionsRootDefenderFilter:
      case STMaxSolutionsHelpFilter:
      case STMaxSolutionsSeriesFilter:
      case STStopOnShortSolutionsRootSolvableFilter:
      case STMaxNrNonTrivial:
      case STMaxNrNonChecks:
      case STMaxNrNonTrivialCounter:
      case STMaxFlightsquares:
      case STDegenerateTree:
      case STNoShortVariations:
      case STEndOfPhaseWriter:
      case STEndOfSolutionWriter:
      case STRefutationWriter:
      case STOutputPlaintextTreeMoveInversionCounter:
      case STOutputPlaintextLineMoveInversionCounter:
      case STOutputPlaintextLineEndOfIntroSeriesMarker:
        Trace_pipe(si);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STParryFork:
        Trace_pipe(si);
        Trace_link("parrying:",slices[si].u.parry_fork.parrying,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.parry_fork.parrying,done_slices);
        break;

      case STMaxThreatLength:
        Trace_pipe(si);
        Trace_link("to_attacker:",
                   slices[si].u.maxthreatlength_guard.to_attacker,
                   "");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.maxthreatlength_guard.next,
                                  done_slices);
        break;

      case STQuodlibet:
      case STReciprocal:
        Trace_slice(si);
        Trace_link("op1:",slices[si].u.binary.op1,"");
        Trace_link("op2:",slices[si].u.binary.op2,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.binary.op1,done_slices);
        TraceStipulationRecursive(slices[si].u.binary.op2,done_slices);
        break;

      case STReflexRootFilter:
      case STReflexHelpFilter:
      case STReflexSeriesFilter:
      case STReflexAttackerFilter:
      case STReflexDefenderFilter:
        Trace_branch(si);
        Trace_link("avoided:",slices[si].u.reflex_guard.avoided,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.reflex_guard.avoided,done_slices);
        TraceStipulationRecursive(slices[si].u.reflex_guard.next,done_slices);
        break;

      case STGoalReachedTester:
      case STOutputPlaintextLineLineWriter:
      case STOutputPlaintextTreeGoalWriter:
        Trace_pipe(si);
        fprintf(stdout,"goal:%u\n",slices[si].u.goal_reached_tester.goal.type);
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STLeaf:
        Trace_link("",slices[si].prev,"<-");
        fprintf(stdout,"\n");
        break;

      default:
        fprintf(stdout,"\n");
        break;
    }
  }
}

void TraceStipulation(slice_index si)
{
  if (level<=max_level)
  {
    boolean done_slices[max_nr_slices] = { false };
    fprintf(stdout,"stipulation structure:\n");
    TraceStipulationRecursive(si,done_slices);
  }
}

#endif
