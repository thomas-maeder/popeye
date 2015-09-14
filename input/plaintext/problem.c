#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
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
 */
void input_plaintext_problem_handle(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_diagram_setup);

  InitMetaData();
  InitBoard();

  ply_reset();

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STStartOfStipulationSpecific),
        alloc_pipe(STEndOfStipulationSpecific),
        alloc_pipe(STOutputPlainTextPositionWriterBuilder),
        alloc_pipe(STOutputPlaintextTwinIntroWriterBuilder),
        alloc_pipe(STOutputPlainTextInstrumentSolversBuilder),
        alloc_pipe(STOutputLaTeXInstrumentSolversBuilder),
        alloc_pipe(STSolvingMachineryIntroBuilder),
        alloc_pipe(STStartOfCurrentTwin)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);

    pipe_solve_delegate(si);

    {
      slice_index const writer = branch_find_slice(STStipulationCompleter,
                                                   si,
                                                   stip_traversal_context_intro);
      dealloc_slices(SLICE_NEXT1(writer));
      SLICE_NEXT1(writer) = no_slice;
    }
  }

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
