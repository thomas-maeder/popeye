#include "output/plaintext/tree/end_of_solution_writer.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/battle_play/defense_play.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/tree/check_detector.h"
#include "trace.h"

/* Allocate a STEndOfSolutionWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_solution_writer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEndOfSolutionWriter);

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
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
end_of_solution_writer_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = defense_defend_in_n(next,n,n_min,n_max_unsolvable);
  if (result<=n+2)
  {
    flush_pending_check(nbply);
    write_pending_decoration();
    Message(NewLine);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
