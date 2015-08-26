#include "solving/duplex.h"
#include "solving/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "stipulation/structure_traversal.h"
#include "options/options.h"
#include "debugging/trace.h"

static void swap_side(slice_index si, stip_structure_traversal *st)
{
  slice_index const stpiulation_root = SLICE_NEXT2(si);
  Side const regular_starter = SLICE_STARTER(stpiulation_root);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_impose_starter(stpiulation_root,advers(regular_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a "half-duplex" problem
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
void half_duplex_solve(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STSolvingMachineryBuilder,&swap_side);
  stip_traverse_structure(si,&st);

  pipe_solve_delegate(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STSolvingMachineryBuilder,&swap_side);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a duplex problem
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
void duplex_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  twin_duplex_type = twin_has_duplex;

  pipe_solve_delegate(si);

  twin_duplex_type = twin_is_duplex;

  half_duplex_solve(si);

  twin_duplex_type = twin_no_duplex;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepare the solving machinery for a duplex problem
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
void input_duplex_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (OptFlag[duplex])
  {
    slice_index const prototypes[] = {
        alloc_pipe(STDuplexSolver)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }
  else if (OptFlag[halfduplex])
  {
    slice_index const prototypes[] = {
        alloc_pipe(STHalfDuplexSolver)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
