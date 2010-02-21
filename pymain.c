#include "pydata.h"
#include "pyhash.h"
#include "pymsg.h"
#include "py1.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"

#include <limits.h>
#include <stdio.h>

/* Guess the "bitness" of the platform
 * @return 32 if we run on a 32bit platform etc.
 */
static unsigned int guessPlatformBitness(void)
{
#if defined(__unix) || __APPLE__ & __MACH__
#  if defined(ULONG_MAX) && ULONG_MAX==18446744073709551615U
  return 64;
#  else
  return 32;
#  endif
#elif defined(_WIN64)
  return 64;
#elif defined(_WIN32)
  return 32;
#endif
}

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made throughout the program. Abort if one of them
 * isn't met. */
static void checkGlobalAssumptions(void)
{
  /* Make sure that the characters relevant for entering problems are
   * encoded contiguously and in the natural order. This is assumed
   * in pyio.c.
   *
   * NB: There is no need for the analoguous check for digits, because
   * decimal digits are guaranteed by the language to be encoded
   * contiguously and in the natural order. */
  assert('b'=='a'+1);
  assert('c'=='b'+1);
  assert('d'=='c'+1);
  assert('e'=='d'+1);
  assert('f'=='e'+1);
  assert('g'=='f'+1);
  assert('h'=='g'+1);

  check_hash_assumptions();
}

int main(int argc, char *argv[])
{
  int idx_end_of_options;

  checkGlobalAssumptions();

  set_nice_priority();

  init_slice_allocator();

  sprintf(versionString,
          "Popeye %s-%uBit v%.2f",
          OSTYPE,guessPlatformBitness(),VERSION);
  
  MaxPositions = ULONG_MAX;
  LaTeXout = false;
  flag_regression = false;

  initMaxmem();

  /* Initialize message table with default language.
   * This default setting is hopefully overriden later by ReadBeginSpec().
   */
  InitMsgTab(LanguageDefault);

  idx_end_of_options = parseCommandlineOptions(argc,argv);
  
  OpenInput(idx_end_of_options<argc ? argv[idx_end_of_options] : " ");

  initMaxtime();

  if (!dimensionHashtable())
    pyfputs("Couldn't allocate the requested amount of memory\n",stdout);

  /* start timer to be able to display a reasonable time if the user
   * aborts execution before the timer is started for the first
   * problem */
  StartTimer();

  InitCheckDir();

  /* Don't use StdString() - possible trace file is not yet opened
   */
  pyfputs(versionString,stdout);
  pyfputs(maxmemString(),stdout);

  iterate_problems();

  CloseInput();

  if (LaTeXout)
    LaTeXClose();
  
  return 0;
}
