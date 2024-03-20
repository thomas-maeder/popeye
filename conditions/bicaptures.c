#include "conditions/bicaptures.h"
#include "solving/pipe.h"
#include "solving/king_capture_avoider.h"
#include "position/position.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void bicaptures_recolor_pieces(slice_index si)
{
  square const *bnp;
  Side const side_moving = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    if (TSTFLAG(being_solved.spec[*bnp],side_moving))
      SETFLAG(being_solved.spec[*bnp],advers(side_moving));

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void bicaptures_unrecolor_pieces(slice_index si)
{
  square const *bnp;
  Side const side_moving = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    if (TSTFLAG(being_solved.spec[*bnp],side_moving))
      CLRFLAG(being_solved.spec[*bnp],advers(side_moving));

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
static void insert_move_generator(slice_index si, stip_structure_traversal *st)
{
  boolean *is_insertion_skipped = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (*is_insertion_skipped)
    *is_insertion_skipped = false;
  else
  {
    slice_index const prototypes[] = {
        alloc_pipe(STBicapturesRecolorPieces),
        alloc_pipe(STBicapturesUnrecolorPieces)
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

// TODO us a parametrized version of solving_insert_move_generators()?
static void skip_insertion(slice_index si, stip_structure_traversal *st)
{
  boolean *is_insertion_skipped = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  *is_insertion_skipped = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const solver_inserters[] =
{
  { STGeneratingMoves,    &insert_move_generator },
  { STSkipMoveGeneration, &skip_insertion        }
};

enum
{
  nr_solver_inserters = sizeof solver_inserters / sizeof solver_inserters[0]
};

static void insert_recolorers(slice_index si)
{
  stip_structure_traversal st;
  boolean is_insertion_skipped = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&is_insertion_skipped);
  stip_structure_traversal_override(&st,solver_inserters,nr_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with bicaptures
 */
void solving_insert_bicaptures(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  insert_recolorers(si);

  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
