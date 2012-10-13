#include "conditions/sentinelles.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "stipulation/has_solution_type.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

unsigned int sentinelles_max_nr_pawns[nr_sides];
unsigned int sentinelles_max_nr_pawns_total;

static void insert_sentinelle(Side trait_ply)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const pi_departing = pjoue[nbply];
  Flags const spec_pi_moving = spec[sq_arrival];
  piece const pi_captured = pprise[nbply];

  if (sq_departure>=square_a2 && sq_departure<=square_h7
      && !is_pawn(abs(pi_departing))
      && e[sq_departure]==vide)
  {
    if (SentPionNeutral && TSTFLAG(spec_pi_moving,Neutral))
    {
      Flags const sentinelle_spec = BIT(White)|BIT(Black)|BIT(Neutral);
      move_effect_journal_do_piece_addition(move_effect_reason_sentinelles,
                                            sq_departure,
                                            sentinelle[White],
                                            sentinelle_spec);
    }
    else
    {
      Side sentinelle_side = SentPionAdverse ? advers(trait_ply) : trait_ply;

      if (flagparasent)
      {
        unsigned int prev_nr_other_sentinelles = nbpiece[-sentinelle[sentinelle_side]];

        if (pi_captured==-sentinelle[sentinelle_side])
          ++prev_nr_other_sentinelles;

        if (nbpiece[sentinelle[sentinelle_side]]>prev_nr_other_sentinelles)
          sentinelle_side = no_side;
      }

      if (sentinelle_side!=no_side)
      {
        if (nbpiece[sentinelle[sentinelle_side]]+1>sentinelles_max_nr_pawns[sentinelle_side]
            || nbpiece[sentinelle[White]]+nbpiece[sentinelle[Black]]+1 > sentinelles_max_nr_pawns_total)
          sentinelle_side = no_side;
      }

      if (sentinelle_side!=no_side)
      {
        Flags const sentinelle_spec = BIT(sentinelle_side);
        move_effect_journal_do_piece_addition(move_effect_reason_sentinelles,
                                              sq_departure,
                                              sentinelle[sentinelle_side],
                                              sentinelle_spec);
      }
    }
  }
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sentinelles_inserter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  insert_sentinelle(slices[si].starter);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_sentinelles_inserters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSentinellesInserter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
