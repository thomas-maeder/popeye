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
#include "pieces/walks/hunters.h"
#include "position/underworld.h"
#include "solving/move_generator.h"
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

/* Handle (read, solve, write) the current problem
 * @return the input token that ends the problem (NextProblem or EndProblem)
 */
char *input_plaintext_problem_handle(char *tok)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_diagram_setup);

  InitMetaData();
  InitBoard();
  InitCond();
  InitOpt();

  ply_reset();

  {
    slice_index const start_of_current_problem = alloc_pipe(STStartOfCurrentProblem);
    tok = input_plaintext_twins_handle(tok,start_of_current_problem);
    dealloc_slices(start_of_current_problem);
  }

  assert_no_leaked_slices();

  reset_max_solutions();
  reset_was_max_nr_solutions_per_target_position_reached();
  reset_short_solution_found_in_problem();
  hunters_reset();
  underworld_reset();

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
