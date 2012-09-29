#include "conditions/haunted_chess.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

ghosts_type ghosts;
ghost_index_type nr_ghosts;

static ghost_index_type find_ghost(square pos)
{
  ghost_index_type current = nr_ghosts;
  ghost_index_type result = ghost_not_found;

  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  while (current>0)
  {
    --current;
    if (ghosts[current].ghost_square==pos)
    {
      result = current;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void forget_ghost_at_pos(ghost_index_type ghost_pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ghost_pos);
  TraceFunctionParamListEnd();

  assert(ghost_pos!=ghost_not_found);
  assert(nr_ghosts>0);
  --nr_ghosts;

  TraceValue("->%u ",nr_ghosts);
  TraceSquare(ghosts[ghost_pos].ghost_square);
  TracePiece(ghosts[ghost_pos].ghost_piece);
  TraceText("\n");
  memmove(ghosts+ghost_pos, ghosts+ghost_pos+1,
          (nr_ghosts-ghost_pos) * sizeof ghosts[0]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void haunted_chess_remember_ghost(void)
{
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_ghosts<ghost_capacity);
  ghosts[nr_ghosts].ghost_square = sq_capture;
  ghosts[nr_ghosts].ghost_piece = pprise[nbply];
  ghosts[nr_ghosts].ghost_flags = pprispec[nbply];
  ghosts[nr_ghosts].hidden = false;
  ++nr_ghosts;
  TraceValue("->%u\n",nr_ghosts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void preempt_ghost(void)
{
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  ghost_index_type const ghost_pos = find_ghost(sq_capture);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpreempt_ghost(void)
{
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  ghost_index_type const ghost_pos = find_ghost(sq_capture);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void haunted_chess_forget_ghost(void)
{
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  forget_ghost_at_pos(find_ghost(sq_capture));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void ban_ghost(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_ghosts<ghost_capacity);
  ghosts[nr_ghosts].ghost_square = sq_departure;
  ghosts[nr_ghosts].ghost_piece = e[sq_departure];
  ghosts[nr_ghosts].ghost_flags = spec[sq_departure];
  ghosts[nr_ghosts].hidden = false;
  ++nr_ghosts;
  TraceValue("->%u\n",nr_ghosts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean summon_ghost(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const sq_departure = move_generation_stack[current_move[nbply]].departure;
    ghost_index_type const ghost_pos = find_ghost(sq_departure);
    if (ghost_pos!=ghost_not_found && !ghosts[ghost_pos].hidden)
    {
      move_effect_journal_do_piece_addition(move_effect_reason_summon_ghost,
                                            sq_departure,
                                            ghosts[ghost_pos].ghost_piece,
                                            ghosts[ghost_pos].ghost_flags);

      forget_ghost_at_pos(ghost_pos);

      result = true;
    }
    else
      result = false;
  }

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
stip_length_type haunted_chess_ghost_summoner_attack(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (summon_ghost())
  {
    result = attack(slices[si].next1,n);
    ban_ghost();
  }
  else
    result = attack(slices[si].next1,n);

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
stip_length_type haunted_chess_ghost_summoner_defend(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (summon_ghost())
  {
    result = defend(slices[si].next1,n);
    ban_ghost();
  }
  else
    result = defend(slices[si].next1,n);

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
stip_length_type haunted_chess_ghost_rememberer_attack(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = attack(slices[si].next1,n);
  else
  {
    preempt_ghost();
    haunted_chess_remember_ghost();
    result = attack(slices[si].next1,n);
    haunted_chess_forget_ghost();
    unpreempt_ghost();
  }

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
stip_length_type haunted_chess_ghost_rememberer_defend(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = defend(slices[si].next1,n);
  else
  {
    preempt_ghost();
    haunted_chess_remember_ghost();
    result = defend(slices[si].next1,n);
    haunted_chess_forget_ghost();
    unpreempt_ghost();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_haunted_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STHauntedChessGhostRememberer);
  stip_instrument_moves(si,STHauntedChessGhostSummoner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
