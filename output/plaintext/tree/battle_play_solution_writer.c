#include "output/plaintext/tree/battle_play_solution_writer.h"
#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/try.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/variation_writer.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBattlePlaySolutionWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_battle_play_solution_writer(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STBattlePlaySolutionWriter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static attack_type last_attack_success;

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
battle_play_solution_writer_can_defend_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (are_we_solving_refutations)
  {
    /* refutations never lead to play that is too short to be
     * interesting
     */
    max_variation_length[nbply+1] = slack_length_battle+1;

    output_plaintext_tree_write_pending_move_decoration();
    Message(NewLine);
    sprintf(GlobalStr,"%*c",4,blank);
    StdString(GlobalStr);
    Message(But);
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);
  }
  else
  {
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);
    if (result>n)
    {
      max_variation_length[nbply+1] = n;
      last_attack_success = attack_try;
    }
    else
    {
      max_variation_length[nbply+1] = result;
      if (refutations==table_nil)
        last_attack_success = attack_key;
      else if (table_length(refutations)==0)
        last_attack_success = attack_key;
      else
        last_attack_success = attack_try;
    }
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
battle_play_solution_writer_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  output_plaintext_tree_write_move();
  output_plaintext_tree_remember_move_decoration(last_attack_success);
  result = defense_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}
