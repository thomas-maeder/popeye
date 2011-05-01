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
    fprintf(stdout,"%s---%s ",prefix,suffix);
  else
    fprintf(stdout,"%s%3u%s ",prefix,si,suffix);
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

    fprintf(stdout,"[%3u] ",si);
    fprintf(stdout,"%-34s ",SliceType_names[slices[si].type]);
    fprintf(stdout,"%c ",Side_names[slices[si].starter][0]);
    switch (slices[si].type)
    {
      case STSetplayFork:
      case STEndOfBattleBranch:
      case STEndOfBranch:
      case STEndOfBranchGoal:
      case STEndOfBranchGoalImmobile:
      case STStipulationReflexAttackSolver:
      case STThreatSolver:
      case STParryFork:
      case STMaxThreatLength:
      case STReflexAttackerFilter:
      case STEndOfBranchForced:
        Trace_pipe(si);
        Trace_link("fork:",slices[si].u.fork.fork,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.fork.fork,
                                  done_slices);
        break;

      case STForkOnRemaining:
        Trace_pipe(si);
        Trace_link("fork:",slices[si].u.fork_on_remaining.fork,"");
        fprintf(stdout,"threshold:%u\n",slices[si].u.fork_on_remaining.threshold);
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.fork_on_remaining.fork,
                                  done_slices);
        break;

      case STAttackAdapter:
      case STDefenseAdapter:
      case STAttackFindShortest:
      case STReadyForAttack:
      case STReadyForDefense:
      case STContinuationSolver:
      case STAttackHashed:
      case STHelpAdapter:
      case STHelpFindByIncreasingLength:
      case STHelpFindShortest:
      case STReadyForHelpMove:
      case STSeriesAdapter:
      case STSeriesFindByIncreasingLength:
      case STSeriesFindShortest:
      case STReadyForSeriesMove:
      case STReadyForSeriesDummyMove:
      case STHelpHashed:
      case STSeriesHashed:
      case STStopOnShortSolutionsFilter:
      case STVariationWriter:
      case STMinLengthOptimiser:
      case STMinLengthGuard:
      case STIntelligentSeriesFilter:
      case STIntelligentHelpFilter:
        Trace_branch(si);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STKeepMatingFilter:
        Trace_pipe(si);
        fprintf(stdout,"mating:%s ",
                Side_names[slices[si].u.keepmating_guard.mating]);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.keepmating_guard.next,
                                  done_slices);
        break;

      case STEndOfRoot:
      case STSelfCheckGuard:
      case STProxy:
      case STMoveInverter:
      case STNot:
      case STGoalReachedTested:
      case STMaxThreatLengthHook:
      case STSeriesDummyMove:
      case STRefutationsAllocator:
      case STTrySolver:
      case STRefutationsSolver:
      case STRefutationsCollector:
      case STPostKeyPlaySuppressor:
      case STKeyWriter:
      case STTryWriter:
      case STThreatEnforcer:
      case STThreatStart:
      case STThreatCollector:
      case STZugzwangWriter:
      case STRestartGuard:
      case STIntelligentDuplicateAvoider:
      case STMaxTimeGuard:
      case STMaxSolutionsInitialiser:
      case STMaxSolutionsGuard:
      case STStopOnShortSolutionsInitialiser:
      case STMaxNrNonChecks:
      case STMaxNrNonTrivial:
      case STMaxNrNonTrivialCounter:
      case STMaxFlightsquares:
      case STDegenerateTree:
      case STNoShortVariations:
      case STAmuMateFilter:
      case STUltraschachzwangGoalFilter:
      case STCirceSteingewinnFilter:
      case STCirceCircuitSpecial:
      case STPiecesKamikazeTargetSquareFilter:
      case STAnticirceTargetSquareFilter:
      case STAnticirceCircuitSpecial:
      case STAnticirceExchangeFilter:
      case STAnticirceExchangeSpecial:
      case STEndOfPhaseWriter:
      case STEndOfSolutionWriter:
      case STContinuationWriter:
      case STRefutationWriter:
      case STRefutingVariationWriter:
      case STGoalMateReachedTester:
      case STGoalStalemateReachedTester:
      case STGoalDoubleStalemateReachedTester:
      case STGoalTargetReachedTester:
      case STGoalCheckReachedTester:
      case STGoalCaptureReachedTester:
      case STGoalSteingewinnReachedTester:
      case STGoalEnpassantReachedTester:
      case STGoalDoubleMateReachedTester:
      case STGoalCounterMateReachedTester:
      case STGoalCastlingReachedTester:
      case STGoalAutoStalemateReachedTester:
      case STGoalCircuitReachedTester:
      case STGoalExchangeReachedTester:
      case STGoalExchangeByRebirthReachedTester:
      case STGoalCircuitByRebirthReachedTester:
      case STGoalAnyReachedTester:
      case STGoalProofgameReachedTester:
      case STGoalNotCheckReachedTester:
      case STGoalAToBReachedTester:
      case STIllegalSelfcheckWriter:
      case STOutputPlaintextTreeMoveInversionCounter:
      case STOutputPlaintextLineMoveInversionCounter:
      case STOutputPlaintextLineEndOfIntroSeriesMarker:
      case STOutputPlaintextTreeCheckWriter:
      case STOutputPlaintextTreeDecorationWriter:
      case STGoalReachableGuardFilter:
      case STCastlingFilter:
      case STEnPassantFilter:
      case STCheckDetector:
      case STKillerMoveCollector:
      case STDoubleMateFilter:
      case STCounterMateFilter:
      case STPrerequisiteOptimiser:
      case STDeadEnd:
      case STDeadEndGoal:
      case STKillerMoveMoveGenerator:
      case STAttackMoveGenerator:
      case STAttackMove:
      case STDefenseMoveGenerator:
      case STDefenseMove:
      case STHelpMoveGenerator:
      case STHelpMove:
      case STSeriesMoveGenerator:
      case STSeriesMove:
      case STCountNrOpponentMovesMoveGenerator:
      case STOrthodoxMatingMoveGenerator:
      case STKillerMoveFinalDefenseMove:
        Trace_pipe(si);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STPiecesParalysingMateFilter:
      case STPiecesParalysingStalemateSpecial:
      case STGoalImmobileReachedTester:
      {
        Trace_pipe(si);
        TraceValue("%u",slices[si].u.goal_filter.applies_to_who);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.goal_filter.next,done_slices);
        break;
      }

      case STQuodlibet:
      case STReciprocal:
        Trace_slice(si);
        Trace_link("op1:",slices[si].u.binary.op1,"");
        Trace_link("op2:",slices[si].u.binary.op2,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.binary.op1,done_slices);
        TraceStipulationRecursive(slices[si].u.binary.op2,done_slices);
        break;

      case STGoalReachedTesting:
      case STOutputPlaintextLineLineWriter:
      case STOutputPlaintextTreeGoalWriter:
        Trace_pipe(si);
        fprintf(stdout,"goal:%u\n",slices[si].u.goal_handler.goal.type);
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STLeaf:
        Trace_link("",slices[si].prev,"<");
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
