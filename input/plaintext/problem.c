#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/incomplete.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/modifier.h"
#include "position/effects/king_square.h"
#include "debugging/assert.h"

char ActAuthor[256];
char ActOrigin[256];
char ActTitle[256];
char ActAward[256];

static void InitMetaData(void)
{
  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActAward[0] = '\0';
}

static void InitBoard(void)
{
  square i;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i= maxsquare-1; i>=0; i--)
  {
    empty_square(i);
    block_square(i);
  }

  /* dummy squares for various purposes -- must be empty */
  empty_square(retro_capture_departure);
  empty_square(pawn_multistep);
  empty_square(messigny_exchange);
  empty_square(kingside_castling);
  empty_square(queenside_castling);
  empty_square(move_by_invisible);
  empty_square(no_capture);

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  king_square_initialise();

  being_solved.currPieceId = NullPieceId;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
        alloc_pipe(STInputPlainTextTwinsHandler),
        alloc_pipe(STTwinIdAdjuster),
        alloc_phase_solving_incomplete(si),
        alloc_pipe(STEndOfStipulationSpecific),
        alloc_pipe(STSolvingMachineryIntroBuilder),
        alloc_pipe(STStartOfCurrentTwin)
    };
    slice_type const type_first_proto = SLICE_TYPE(prototypes[0]);
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);

    stipulation_modifier_instrument(si,STStipulationStarterDetector);

    pipe_solve_delegate(si);

    undo_move_effects();

    {
      slice_index const first = branch_find_slice(type_first_proto,
                                                  si,
                                                  stip_traversal_context_intro);
      SLICE_NEXT1(SLICE_PREV(first)) = no_slice;
      dealloc_slices(first);
    }
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
