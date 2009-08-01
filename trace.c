#include "trace.h"
#include "pyproc.h"
#include "pydata.h"
#include "pyhash.h"
#include "pystip.h"

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
    fprintf(stdout,"> #%lu %s",level,name);
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
    ecritcoup(current_ply,no_goal);
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

static void TraceStipulationRecursive(slice_index si, boolean done_slices[])
{
  if (si!=no_slice && !done_slices[si])
  {
    done_slices[si] = true;

    fprintf(stdout,"[%2u]: ",si);
    fprintf(stdout,"%s ",SliceType_names[slices[si].type]);
    fprintf(stdout,"starter:%s ",
            slices[si].starter==White ? "White" : "Black");
    switch (slices[si].type)
    {
      case STBranchDirect:
        fprintf(stdout,"length:%u ",slices[si].u.pipe.u.branch.length);
        fprintf(stdout,"min_length:%u ",slices[si].u.pipe.u.branch.min_length);
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STBranchDirectDefender:
        fprintf(stdout,"length:%u ",
                slices[si].u.pipe.u.branch_d_defender.length);
        fprintf(stdout,"min_length:%u ",
                slices[si].u.pipe.u.branch_d_defender.min_length);
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"towards_goal:%u ",
                slices[si].u.pipe.u.branch_d_defender.towards_goal);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.pipe.u.branch_d_defender.towards_goal,done_slices);
        break;

      case STHelpRoot:
        fprintf(stdout,"length:%u ",
                slices[si].u.pipe.u.help_adapter.length);
        fprintf(stdout,"min_length:%u ",
                slices[si].u.pipe.u.help_adapter.min_length);
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STBranchHelp:
      case STHelpHashed:
        fprintf(stdout,"length:%u ",slices[si].u.pipe.u.branch.length);
        fprintf(stdout,"min_length:%u ",slices[si].u.pipe.u.branch.min_length);
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STBranchFork:
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"towards_goal:%u ",
                slices[si].u.pipe.u.branch_fork.towards_goal);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.pipe.u.branch_fork.towards_goal,
                                  done_slices);
        break;

      case STLeafSelf:
        fprintf(stdout,"next:%u ",slices[si].u.leafself.next);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.leafself.next,done_slices);
        break;

      case STQuodlibet:
      case STReciprocal:
        fprintf(stdout,"op1:%u ",slices[si].u.fork.op1);
        fprintf(stdout,"op2:%u ",slices[si].u.fork.op2);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.fork.op1,done_slices);
        TraceStipulationRecursive(slices[si].u.fork.op2,done_slices);
        break;

      case STMoveInverter:
      case STSelfCheckGuard:
      case STGoalReachableGuard:
        fprintf(stdout,"next:%u ",slices[si].u.pipe.next);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      default:
        fprintf(stdout,"\n");
        break;
    }
  }
}

void TraceStipulation(void)
{
  if (level<=max_level)
  {
    boolean done_slices[max_nr_slices] = { false };
    fprintf(stdout,"stipulation structure:\n");
    TraceStipulationRecursive(root_slice,done_slices);
  }
}

#endif
