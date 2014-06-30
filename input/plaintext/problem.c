#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/twin.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "options/maxsolutions/maxsolutions.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "pieces/walks/hunters.h"
#include "position/underworld.h"
#include "solving/move_generator.h"
#include "stipulation/proxy.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "debugging/assert.h"

char ActAuthor[256];
char ActOrigin[256];
char ActTitle[256];
char ActAward[256];
char ActStip[37];

static void InitMetaData(void)
{
  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActAward[0] = '\0';
  ActStip[0] = '\0';
}

static void InitBoard(void)
{
  square i;
  square const *bnp;

  for (i= maxsquare-1; i>=0; i--)
  {
    empty_square(i);
    block_square(i);
  }

  /* dummy squares for various purposes -- must be empty */
  empty_square(pawn_multistep);
  empty_square(messigny_exchange);
  empty_square(kingside_castling);
  empty_square(queenside_castling);
  empty_square(retro_capture_departure);

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  being_solved.king_square[White] = initsquare;
  being_solved.king_square[Black] = initsquare;
}

static void write_problem_footer(void)
{
  if (max_solutions_reached()
      || was_max_nr_solutions_per_target_position_reached()
      || has_short_solution_been_found_in_problem()
      || hasMaxtimeElapsed())
    output_plaintext_message(InterMessage);
  else
    output_plaintext_message(FinishProblem);

  output_plaintext_print_time(" ","");
  output_plaintext_message(NewLine);
  output_plaintext_message(NewLine);
  output_plaintext_message(NewLine);
  protocol_fflush(stdout);
}

/* Handle (read, solve, write) the current problem
 * @return the input token that ends the problem (NextProblem or EndProblem)
 */
Token input_plaintext_problem_handle(void)
{
  Token result;
  slice_index stipulation_root_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_diagram_setup);

  InitMetaData();
  InitBoard();
  InitCond();
  InitOpt();

  stipulation_root_hook = alloc_proxy_slice();

  ply_reset();

  result = ReadInitialTwin(stipulation_root_hook);

  if (slices[stipulation_root_hook].next1==no_slice)
    output_plaintext_input_error_message(NoStipulation,0);
  else
  {
    StartTimer();
    initialise_piece_ids();
    result = input_plaintext_twins_iterate(result,stipulation_root_hook);
    write_problem_footer();
  }

  dealloc_slices(stipulation_root_hook);
  assert_no_leaked_slices();

  reset_max_solutions();
  reset_was_max_nr_solutions_per_target_position_reached();
  reset_short_solution_found_in_problem();
  hunters_reset();
  underworld_reset();

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
