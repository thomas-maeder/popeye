#include "conditions/actuated_revolving_centre.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_actuated_revolving_centre(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STActuatedRevolvingCentre);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void revolve(void)
{
  piece const piece_temp = e[square_d4];
  Flags const spec_temp = spec[square_d4];

  e[square_d4] = e[square_e4];
  spec[square_d4] = spec[square_e4];

  e[square_e4] = e[square_e5];
  spec[square_e4] = spec[square_e5];

  e[square_e5] = e[square_d5];
  spec[square_e5] = spec[square_d5];

  e[square_d5] = piece_temp;
  spec[square_d5] = spec_temp;
}

static void unrevolve(void)
{
  piece const piece_temp = e[square_d5];
  Flags const spec_temp = spec[square_d5];

  e[square_d5] = e[square_e5];
  spec[square_d5] = spec[square_e5];

  e[square_e5] = e[square_e4];
  spec[square_e5] = spec[square_e4];

  e[square_e4] = e[square_d4];
  spec[square_e4] = spec[square_d4];

  e[square_d4] = piece_temp;
  spec[square_d4] = spec_temp;
}

static square revolve_square(square s)
{
  square result;

  if (s==square_d4)
    result = square_d5;
  else if (s==square_d5)
    result = square_e5;
  else if (s==square_e5)
    result = square_e4;
  else if (s==square_e4)
    result = square_d4;
  else
    result = initsquare;

  return result;
}

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
static void move_effect_journal_do_centre_revolution(move_effect_reason_type reason)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceValue("%u",transformation);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_centre_revoluation;
  top_elmt->reason = reason;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  revolve();

  {
    square revolved = revolve_square(king_square[White]);
    if (revolved!=initsquare)
      move_effect_journal_do_king_square_movement(reason,White,revolved);
  }

  {
    square revolved = revolve_square(king_square[Black]);
    if (revolved!=initsquare)
      move_effect_journal_do_king_square_movement(reason,Black,revolved);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_centre_revolution(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  unrevolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void replay_centre_revolution(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  revolve();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean does_move_trigger_revolution(void)
{
  boolean result;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = (sq_departure==square_d4    || sq_departure==square_e4
            || sq_departure==square_d5 || sq_departure==square_e5
            || sq_arrival==square_d4   || sq_arrival==square_e4
            || sq_arrival==square_d5   || sq_arrival==square_e5);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type actuated_revolving_centre_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
  {
    move_effect_journal_do_centre_revolution(move_effect_reason_actuate_revolving_centre);
    result = attack(next,n);
  }
  else
    result = attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type actuated_revolving_centre_defend(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
  {
    move_effect_journal_do_centre_revolution(move_effect_reason_actuate_revolving_centre);
    result = defend(next,n);
  }
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
