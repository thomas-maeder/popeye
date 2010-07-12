#include "output/plaintext/tree/zugzwang_writer.h"
#include "pyoutput.h"
#include "pymsg.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/threat.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/tree/check_detector.h"
#include "trace.h"

/* Allocate a STZugzwangWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_zugzwang_writer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STZugzwangWriter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
zugzwang_writer_has_solution_in_n(slice_index si,
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

  result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type zugzwang_writer_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  ply const threats_ply = nbply+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (threat_activities[threats_ply]==threat_solving)
  {
    TraceValue("%u",nbply);
    if (nbply==2)
      /* option postkey is set - write "threat:" or "zugzwang" on a new
       * line
       */
      Message(NewLine);

    result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);

    {
      /* We don't signal "Zugzwang" after the last attacking move of a
       * self play variation
       */
      if (n>slack_length_battle && result==n+2)
      {
        flush_pending_check(nbply-1);
        output_plaintext_tree_write_pending_move_decoration();
        StdChar(blank);
        Message(Zugzwang);
      }
    }
  }
  else
    result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
