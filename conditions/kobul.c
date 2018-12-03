#include "conditions/kobul.h"
#include "pieces/walks/classification.h"
#include "position/king_square.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "position/walk_change.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

boolean kobul_who[nr_sides];

enum
{
  transferred_flags_mask = PieSpMask&~COLOURFLAGS&~BIT(Royal)
};

static void substitute(Side trait_ply)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  square const king_pos = being_solved.king_square[advers(trait_ply)];

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && kobul_who[advers(trait_ply)]
      && king_pos!=initsquare)
  {
    piece_walk_type const pi_captured = move_effect_journal[capture].u.piece_removal.walk;
    piece_walk_type const kobul_kind = is_pawn(pi_captured) ? King : pi_captured;

    Flags const capturee_flags = move_effect_journal[capture].u.piece_removal.flags;
    Flags spec_kobul = being_solved.spec[king_pos];
    CLRFLAGMASK(spec_kobul,transferred_flags_mask);
    SETFLAGMASK(spec_kobul,capturee_flags&transferred_flags_mask);

    if (get_walk_of_piece_on_square(king_pos)!=kobul_kind)
      move_effect_journal_do_walk_change(move_effect_reason_kobul_king,
                                          king_pos,
                                          kobul_kind);

    if (being_solved.spec[king_pos]!=spec_kobul)
      move_effect_journal_do_flags_change(move_effect_reason_kobul_king,
                                          king_pos,
                                          spec_kobul);
  }
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
void kobul_king_substitutor_solve(slice_index si)
{
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_squares();
  substitute(SLICE_STARTER(si));
  pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_kobul_king_substitutors(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STKobulKingSubstitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
