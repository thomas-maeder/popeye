#include "retro/retro.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "position/null_move.h"
#include "solving/pipe.h"

#include "debugging/assert.h"

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
void retro_play_null_move(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_null_move();
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
void retro_start_retraction_ply(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_retro_move_takeback);

  pipe_solve_delegate(si);

  undo_move_effects();

  finply();

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
void retro_start_retro_move_ply(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Make sure that trait is the opposite of the first move (or there
   * will be no e.p. capture). */
  assert(slices[si].starter!=no_side);
  nextply(advers(slices[si].starter));

  assert(nbply==ply_retro_move);

  {
    /* TODO let Mars Circe or SingleBox Type 3 add slices here that do this */
    unsigned int i;
    for (i = 0; i!=move_effect_journal_index_offset_capture; ++i)
      move_effect_journal_do_null_effect();
  }

  pipe_solve_delegate(si);

  undo_move_effects();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with a retro move retractor slice
 * @param solving_machinery index of entry slice into solving machinery
 * @param type type of retractor slice
 */
void retro_instrument_retractor(slice_index solving_machinery, slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",solving_machinery);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  {
    slice_index const proto = alloc_pipe(type);
    slice_insertion_insert(solving_machinery,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute(slice_index si, stip_structure_traversal*st)
{
  slice_type const *type = st->param;
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(*type));
}

/* Substitute the default slice playing the last retro move by a slice of a
 * different type
 * @param solving_machinery index of entry slice into solving machinery
 * @param type type of the substitute
 */
void retro_substitute_last_move_player(slice_index solving_machinery,
                                       slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",solving_machinery);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&type);
    stip_structure_traversal_override_single(&st,
                                             STRetroPlayNullMove,
                                             &substitute);
    stip_traverse_structure(solving_machinery,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void retro_instrument_solving_default(slice_index solving_machinery)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",solving_machinery);
  TraceFunctionParamListEnd();

  {
    slice_index const protos[] = {
        alloc_pipe(STRetroStartRetractionPly),
        alloc_pipe(STRetroStartRetroMovePly),
        alloc_pipe(STRetroPlayNullMove)
    };
    enum { nr_prototypes = sizeof protos / sizeof protos[0] };
    slice_insertion_insert(solving_machinery,protos,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
