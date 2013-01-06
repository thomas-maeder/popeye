#include "pieces/attributes/neutral/initialiser.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "pieces/side_change.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides slice type STPiecesNeutralInitialiser
 */

/* Side that the neutral pieces currently belong to
 */
Side neutral_side;

/* Change the side of the piece on a square so that the piece belongs to the
 * neutral side
 * @param p address of piece whose side to change
 */
void setneutre(piece *p)
{
  Side const current_side = *p<=roin ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,current_side,"");
  TraceEnumerator(Side,neutral_side,"\n");

 if (neutral_side!=current_side)
    piece_change_side(p);

 TraceFunctionExit(__func__);
 TraceFunctionResultEnd();
}

/* Initialise the neutral pieces to belong to the side to be captured in the
 * subsequent move
 * @param captured_side side of pieces to be captured
 */
void initialise_neutrals(Side captured_side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,captured_side,"");
  TraceFunctionParamListEnd();

  /* I don't know why, but the solution below is not slower */
  /* than the double loop solution of genblanc(). NG */

  TraceEnumerator(Side,neutral_side,"\n");
  if (neutral_side!=captured_side)
  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
    {
      if (e[*bnp]!=vide)
      {
        TraceSquare(*bnp);
        TraceValue("%d",e[*bnp]);
        TraceValue("%u\n",TSTFLAG(spec[*bnp],Neutral));
      }
      if (TSTFLAG(spec[*bnp],Neutral))
        piece_change_side(&e[*bnp]);
    }

    neutral_side = captured_side;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index alloc_neutral_initialiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesNeutralInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_initialiser_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  initialise_neutrals(advers(slices[si].starter));
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* make sure that the retracting neutrals belong to the right side if our
 * posteriority has changed the neutral side*/
void neutral_initialiser_recolor_retracting(void)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_top[parent_ply[nbply]]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
      {
        square const from = move_effect_journal[curr].u.piece_movement.from;
        if (TSTFLAG(spec[from],Neutral))
          setneutre(&e[from]);
        break;
      }

      case move_effect_piece_removal:
      {
        square const from = move_effect_journal[curr].u.piece_removal.from;
        if (TSTFLAG(spec[from],Neutral))
          setneutre(&e[from]);
        break;
      }

      case move_effect_piece_change:
      {
        square const on = move_effect_journal[curr].u.piece_change.on;
        if (TSTFLAG(spec[on],Neutral))
          setneutre(&e[on]);
        break;
      }

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_undo_neutral_initialisation(void)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_neutral_recoloring_undo;
  top_elmt->reason = move_effect_reason_neutral_recoloring;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_retracting_recolorer_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_undo_neutral_initialisation();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void neutral_initialiser_recolor_replaying(void)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_top[parent_ply[nbply]]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_addition:
      {
        square const on = move_effect_journal[curr].u.piece_addition.on;
        if (TSTFLAG(spec[on],Neutral))
          setneutre(&e[on]);
        break;
      }

      case move_effect_piece_change:
      {
        square const on = move_effect_journal[curr].u.piece_change.on;
        if (TSTFLAG(spec[on],Neutral))
          setneutre(&e[on]);
        break;
      }

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_do_neutral_initialisation(void)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_neutral_recoloring_do;
  top_elmt->reason = move_effect_reason_neutral_recoloring;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  neutral_initialiser_recolor_replaying();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_replaying_recolorer_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_neutral_initialisation();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_initialiser(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_neutral_initialiser_slice();
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_neutral_initialisers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STGeneratingMoves,
                                           &insert_initialiser);
  stip_traverse_structure(si,&st);

  stip_instrument_moves(si,STPiecesNeutralRetractingRecolorer);
  stip_instrument_moves(si,STPiecesNeutralReplayingRecolorer);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
