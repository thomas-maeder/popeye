#include "platform/maxtime_impl.h"
#include "utilities/boolean.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#if defined(SIGNALS)

#include <unistd.h>
#include "optimisations/hash.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/position.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"
#include "pieces/pieces.h"
#include "debugging/assert.h"

/* default signal handler: */
static void ReportSignalAndBailOut(int sig)
{
  output_plaintext_report_aborted(sig);
  exit(1);
}

/* specific signal handlers: */
#if defined(HASHRATE)
static void sigUsr1Handler(int sig)
{
  IncHashRateLevel();
  signal(sig, &sigUsr1Handler);
}

static void sigUsr2Handler(int sig)
{
  DecHashRateLevel();
  signal(sig, &sigUsr2Handler);
}
#endif

/* this initialisation is valid only for Unix.
 * I do not know whether there are other handling
 * possiblilities. May be on Mac's and Amigas.
 * or windows?
 */
static int const SignalToCatch[] =
{
  SIGHUP
  , SIGINT
  , SIGQUIT
  , SIGTERM
#if !defined(_hpux) && !defined(__GO32__)
  /* not supported by HP-UX */
  /* not supported by DJGPP */
  , SIGVTALRM
  , SIGXCPU
  , SIGXFSZ
#endif /* HPUX, __GO32__ */
};

enum
{
  nrSignals = sizeof SignalToCatch / sizeof SignalToCatch[0]
};

static piece_walk_type find_promotion(ply ply, square sq_arrival)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const top = move_effect_journal_base[ply+1];
  move_effect_journal_index_type curr;
  piece_walk_type result = Empty;

  for (curr = base+move_effect_journal_index_offset_other_effects; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_change
        && move_effect_journal[curr].u.piece_change.on==sq_arrival)
    {
      result = move_effect_journal[curr].u.piece_change.to;
      break;
    }

  return result;
}

static void ReDrawPly(ply curr_ply)
{
  ply const parent = parent_ply[curr_ply];

  if (parent>ply_retro_move)
    ReDrawPly(parent);

  {
    move_effect_journal_index_type const top = move_effect_journal_base[curr_ply];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    piece_walk_type const pi_moving = move_effect_journal[movement].u.piece_movement.moving;
    piece_walk_type const promotee = find_promotion(curr_ply,move_effect_journal[movement].u.piece_movement.to);
    WriteWalk(&output_plaintext_engine,stdout,pi_moving);
    WriteSquare(&output_plaintext_engine,stdout,move_generation_stack[CURRMOVE_OF_PLY(curr_ply)].departure);
    protocol_fputc('-',stdout);
    WriteSquare(&output_plaintext_engine,stdout,move_generation_stack[CURRMOVE_OF_PLY(curr_ply)].arrival);
    if (promotee!=Empty)
    {
      protocol_fputc('=',stdout);
      WriteWalk(&output_plaintext_engine,stdout,promotee);
    }
    protocol_fprintf(stdout,"%s","   ");
  }
}

static void ReDrawBoard(int sig)
{
  /* I did this, to see more accurately what position popeye is
     working on.  ElB
  */
  /* If a position can be reached by 1000's of move sequences than the
     position is of almost no value. The history is more important.
     TLi
  */

  WriteBoard(&being_solved);

  /* and write (some information about) the sequences of moves that
     lead to this position.
  */
  ReDrawPly(nbply);
  fputc('\n',stdout);

  signal(sig,&ReDrawBoard);
}

static void solvingTimeOver(int sig)
{
  /* To stop the calculation of a problem after a given amount of time
   * is over.
   */
  periods_counter = nr_periods;

  signal(sig,&solvingTimeOver);
}

void platform_init(void)
{
  /* register default handler for all supported signals */
  int i;
  for (i=0; i<nrSignals; ++i)
    signal(SignalToCatch[i],&ReportSignalAndBailOut);

  /* override default handler with specific handlers.
   * this code would be much more robust, if some information about
   * the signals were available without knowing the semantics of the
   * Unix-Signal numbers.
   * At least the maximum signal-number should be defined and for what
   * signals the handling can be redefined
   */
#if defined(HASHRATE)
  signal(SIGUSR1, &sigUsr1Handler);
  signal(SIGUSR2, &sigUsr2Handler);
#endif /*HASHRATE*/
  signal(SIGALRM, &solvingTimeOver);
  signal(SIGHUP,  &ReDrawBoard);
}

boolean platform_set_maxtime_timer(maxtime_type seconds)
{
  periods_counter = 0;
  nr_periods = 1;

  if (seconds!=no_time_set)
  {
#if !defined(NDEBUG)
    unsigned int const last_alarm_due_time =
#endif
        alarm(seconds);

    assert(last_alarm_due_time==0);
  }

  return true;
}

void platform_reset_maxtime_timer(void)
{
  alarm(0);
}

#else

boolean platform_set_maxtime_timer(maxtime_type seconds)
{
  if (seconds==no_time_set)
  {
    periods_counter = 0;
    nr_periods = 1;
    return true;
  }
  else
  {
    periods_counter = 1;
    nr_periods = 0;
    return false;
  }
}

void platform_reset_maxtime_timer(void)
{
}

#endif /*SIGNALS*/
