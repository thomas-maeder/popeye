#include "optimisations/hash.h"
#include "solving/moves_traversal.h"
#include "output/plaintext/language_dependant.h"
#include "output/latex/latex.h"
#include "platform/priority.h"
#include "position/effects/piece_creation.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/piece_removal.h"
#include "position/effects/board_transformation.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/side_change.h"
#include "position/effects/piece_movement.h"
#include "position/effects/piece_exchange.h"
#include "input/commandline.h"
#include "input/plaintext/token.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

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
#if 'b' != ('a' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'b' != ('a' + 1); unable to compile."
#endif
#if 'c' != ('b' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'c' != ('b' + 1); unable to compile."
#endif
#if 'd' != ('c' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'd' != ('c' + 1); unable to compile."
#endif
#if 'e' != ('d' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'e' != ('d' + 1); unable to compile."
#endif
#if 'f' != ('e' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'f' != ('e' + 1); unable to compile."
#endif
#if 'g' != ('f' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'g' != ('f' + 1); unable to compile."
#endif
#if 'h' != ('g' + 1)
#   error "COMPILE-TIME ASSERTION FAILURE: 'h' != ('g' + 1); unable to compile."
#endif

  check_hash_assumptions();
  enforce_piecename_uniqueness();
}

int main(int argc, char *argv[])
{
  checkGlobalAssumptions();

  platform_set_nice_priority();

  initialise_slice_properties();
  init_structure_children_visitors();
  init_moves_children_visitors();

  move_effect_journal_init_move_effect_doers();

  position_piece_creation_initialise();
  position_piece_removal_initialise();
  position_piece_readdition_initialise();
  position_board_transformation_initialise();
  position_piece_movement_initialise();
  position_walk_change_initialise();
  position_piece_exchange_initialise();
  position_flags_change_initialise();
  position_side_change_initialise();

  {
    slice_index const parser = alloc_command_line_options_parser(argc,argv);

    slice_index const prototypes[] =
    {
      alloc_pipe(STPlatformInitialiser),
      alloc_pipe(STHashTableDimensioner),
      alloc_pipe(STOutputLaTeXCloser),
      /* start timer to be able to display a reasonable time if the user
       * aborts execution before the timer is started for the first
       * problem */
      alloc_pipe(STTimerStarter),
      alloc_pipe(STCheckDirInitialiser),
      output_plaintext_alloc_writer(stdout),
      alloc_pipe(STInputPlainTextUserLanguageDetector),
      alloc_pipe(STInputPlainTextProblemsIterator),
      alloc_pipe(STConditionsResetter),
      alloc_pipe(STOptionsResetter),
      alloc_pipe(STInputPlainTextProblemHandler),
      alloc_pipe(STTimerStarter),
      alloc_pipe(STInputPlainTextInitialTwinReader),
      alloc_pipe(STProblemSolvingIncomplete)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(parser,prototypes,nr_prototypes);

    solve(parser);

    dealloc_slices(parser);

    assert_no_leaked_slices();
  }

  return 0;
}
