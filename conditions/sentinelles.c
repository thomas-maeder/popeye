#include "conditions/sentinelles.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/classification.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/effects/piece_creation.h"
#include "position/effects/piece_movement.h"

#include "debugging/assert.h"

unsigned int sentinelles_max_nr_pawns[nr_sides];
unsigned int sentinelles_max_nr_pawns_total;

piece_walk_type sentinelle_walk;

sentinelles_pawn_mode_type sentinelles_pawn_mode;

boolean sentinelles_is_para;

static void insert_sentinelle(Side trait_ply)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal_get_departure_square(nbply);
  piece_walk_type const pi_departing = move_effect_journal[movement].u.piece_movement.moving;
  Flags const spec_pi_moving = move_effect_journal[movement].u.piece_movement.movingspec;
  SquareFlags const prom_square = BIT(WhPromSq)|BIT(BlPromSq);

  if (!TSTFLAGMASK(sq_spec(sq_departure),prom_square)
      && !is_pawn(pi_departing)
      && is_square_empty(sq_departure))
  {
    if (sentinelles_pawn_mode==sentinelles_pawn_neutre
        && is_piece_neutral(spec_pi_moving))
    {
      Flags const sentinelle_spec = all_pieces_flags|NeutralMask;
      move_effect_journal_do_piece_creation(move_effect_reason_sentinelles,
                                            sq_departure,
                                            sentinelle_walk,
                                            sentinelle_spec,
                                            trait[nbply]);
    }
    else
    {
      Side sentinelle_side = sentinelles_pawn_mode==sentinelles_pawn_adverse ? advers(trait_ply) : trait_ply;

      if (sentinelles_is_para)
      {
        unsigned int prev_nr_other_sentinelles = being_solved.number_of_pieces[advers(sentinelle_side)][sentinelle_walk];
        piece_walk_type const pi_captured = move_effect_journal[capture].u.piece_removal.walk;

        if (pi_captured==sentinelle_walk)
          ++prev_nr_other_sentinelles;

        if (being_solved.number_of_pieces[sentinelle_side][sentinelle_walk]>prev_nr_other_sentinelles)
          sentinelle_side = no_side;
      }

      if (sentinelle_side!=no_side)
      {
        if (being_solved.number_of_pieces[sentinelle_side][sentinelle_walk]+1>sentinelles_max_nr_pawns[sentinelle_side]
            || being_solved.number_of_pieces[White][sentinelle_walk]+being_solved.number_of_pieces[Black][sentinelle_walk]+1 > sentinelles_max_nr_pawns_total)
          sentinelle_side = no_side;
      }

      if (sentinelle_side!=no_side)
      {
        Flags const sentinelle_spec = all_pieces_flags|BIT(sentinelle_side);
        move_effect_journal_do_piece_creation(move_effect_reason_sentinelles,
                                              sq_departure,
                                              sentinelle_walk,
                                              sentinelle_spec,
                                              sentinelle_side);
      }
    }
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
void sentinelles_inserter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_sentinelle(SLICE_STARTER(si));
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_sentinelles_inserters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSentinellesInserter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
