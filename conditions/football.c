#include "conditions/football.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

piece current_football_substitution[maxply+1];

static PieNam const *bench[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static PieNam const *get_bench(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (sq_arrival!=king_square[Black] && sq_arrival!=king_square[White]
      && (sq_arrival%onerow==left_file || sq_arrival%onerow==right_file))
  {
    PieNam const p = abs(e[sq_arrival]);
    PieNam tmp = getfootballpiece[Empty];

    /* ensure moving piece is on list to allow null (= non-) substitutions */
    if (tmp!=p)
    {
      /* remove old head-of-list if not part of standard set */
      if (!footballpiece[tmp])
        getfootballpiece[Empty] = getfootballpiece[tmp];

      /* add moving piece to head-of-list if not already part of standard set */
      if (!footballpiece[p])
      {
        getfootballpiece[p] = getfootballpiece[vide];
        getfootballpiece[Empty] = p;
      }
    }

    return getfootballpiece;
  }
  else
    return NULL;
}

static void init_substitution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  bench[nbply] = get_bench();
  current_football_substitution[nbply] = bench[nbply]==0 ? vide : bench[nbply][vide];
  TracePiece(current_football_substitution[nbply]);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean advance_substitution(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_football_substitution[nbply] = bench[nbply][current_football_substitution[nbply]];
  TracePiece(current_football_substitution[nbply]);TraceText("\n");
  result = current_football_substitution[nbply]!=vide;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type football_chess_substitutor_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_substitution();

  if (current_football_substitution[nbply]==vide)
    result = attack(slices[si].next1,n);
  else
  {
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
    piece const substituted = e[sq_arrival];
    piece const substitute = substituted<vide ? -current_football_substitution[nbply] : current_football_substitution[nbply];

    if (substitute!=substituted)
      move_effect_journal_do_piece_change(move_effect_reason_football_chess_substituition,
                                          sq_arrival,
                                          substitute);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      if (advance_substitution())
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

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
stip_length_type football_chess_substitutor_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_substitution();

  if (current_football_substitution[nbply]==vide)
    result = defend(slices[si].next1,n);
  else
  {
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
    piece const substituted = e[sq_arrival];
    piece const substitute = substituted<vide ? -current_football_substitution[nbply] : current_football_substitution[nbply];

    if (substitute!=substituted)
      move_effect_journal_do_piece_change(move_effect_reason_football_chess_substituition,
                                          sq_arrival,
                                          substitute);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      if (advance_substitution())
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_football_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STFootballChessSubsitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
