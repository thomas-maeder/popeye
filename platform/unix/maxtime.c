#include "platform/maxtime_impl.h"
#include "platform/platform.h"
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

#if !defined(HAVE_SIGACTION)
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#if defined(_POSIX_C_SOURCE)
#define HAVE_SIGACTION 1
#else /* TODO: Can we detect sigaction in other libraries? */
#define HAVE_SIGACTION 0
#endif /*_POSIX_C_SOURCE*/
#else
#define HAVE_SIGACTION 0
#endif /*__GLIBC__,__GLIBC_MINOR__*/
#endif /*HAVE_SIGACTION*/

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
#if !HAVE_SIGACTION
  signal(sig, &sigUsr1Handler);
#endif /*!HAVE_SIGACTION*/
}

static void sigUsr2Handler(int sig)
{
  DecHashRateLevel();
#if !HAVE_SIGACTION
  signal(sig, &sigUsr2Handler);
#endif /*!HAVE_SIGACTION*/
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
    if (move_effect_journal[curr].type==move_effect_walk_change
        && move_effect_journal[curr].u.piece_walk_change.on==sq_arrival)
    {
      result = move_effect_journal[curr].u.piece_walk_change.to;
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
  /* If a position can be reached by 1000's of move sequences then the
     position is of almost no value. The history is more important.
     TLi
  */

  WriteBoard(&being_solved);

  /* and write (some information about) the sequences of moves that
     lead to this position.
  */
  ReDrawPly(nbply);
  putchar('\n');

#if !HAVE_SIGACTION
  signal(sig,&ReDrawBoard);
#endif /*!HAVE_SIGACTION*/
}

static void solvingTimeOver(int sig)
{
  /* To stop the calculation of a problem after a given amount of time
   * is over.
   */
  periods_counter = nr_periods;

#if !HAVE_SIGACTION
  signal(sig,&solvingTimeOver);
#endif /*!HAVE_SIGACTION*/
}

void platform_init(void)
{
  /* register default handler for all supported signals */
  int i;
#if HAVE_SIGACTION
  struct sigaction act;
  act.sa_handler = &ReportSignalAndBailOut;
  act.sa_mask = 0;
  act.sa_flags = 0;
  for (i=0; i<nrSignals; ++i)
    if (sigaction(SignalToCatch[i],&act,NULL))
#else
  for (i=0; i<nrSignals; ++i)
    if (signal(SignalToCatch[i],&ReportSignalAndBailOut)==SIG_ERR)
#endif /*HAVE_SIGACTION*/
      perror(__func__);

  /* override default handler with specific handlers.
   * this code would be much more robust, if some information about
   * the signals were available without knowing the semantics of the
   * Unix-Signal numbers.
   * At least the maximum signal-number should be defined and for what
   * signals the handling can be redefined
   */
#if HAVE_SIGACTION
#if defined(HASHRATE)
  act.sa_handler = &sigUsr1Handler; 
  if (sigaction(SIGUSR1, &act, NULL))
    perror(__func__);
  act.sa_handler = &sigUsr2Handler; 
  if (sigaction(SIGUSR2, &act, NULL))
    perror(__func__);
#endif /*HASHRATE*/
  act.sa_handler = &solvingTimeOver; 
  if (sigaction(SIGALRM, &act, NULL))
    perror(__func__);
  act.sa_handler = &ReDrawBoard; 
  if (sigaction(SIGHUP,  &act, NULL))
    perror(__func__);
#else
#if defined(HASHRATE)
  if (signal(SIGUSR1, &sigUsr1Handler) == SIG_ERR)
    perror(__func__);
  if (signal(SIGUSR2, &sigUsr2Handler) == SIG_ERR)
    perror(__func__);
#endif /*HASHRATE*/
  if (signal(SIGALRM, &solvingTimeOver) == SIG_ERR)
    perror(__func__);
  if (signal(SIGHUP,  &ReDrawBoard) == SIG_ERR)
    perror(__func__);
#endif /*HAVE_SIGACTION*/
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
