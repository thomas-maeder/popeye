#include "output/plaintext/tree/check_detector.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "output/plaintext/tree/tree.h"
#include "trace.h"

typedef enum
{
  pending_check_invalid,
  pending_check_none,
  pending_check_detected,
  no_pending_check_detected,
  pending_check_written
} pending_check_type;

static pending_check_type pending_check[maxply+1] =
{
  pending_check_invalid,
  pending_check_written /* no check needed for virtual 'last move' */
};

/* Reset the pending check state
 */
void reset_pending_check(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  pending_check[nbply] = pending_check_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a possible pending check
 */
void flush_pending_check(ply move_ply)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",move_ply);
  TraceFunctionParamListEnd();

  if (pending_check[move_ply]==pending_check_detected)
    StdString(" +");
  pending_check[move_ply] = pending_check_written;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect whether the move leading to the current position has
 * delivered a check that we have to write before the next move
 */
static void detect_pending_check(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  if (encore() /* no need to test check if we are solving threats */
      && pending_check[nbply]==pending_check_none) /* already tested? */
  {
    pending_check[nbply] = (echecc(nbply,slices[si].starter)
                            ? pending_check_detected
                            : no_pending_check_detected);
    TraceValue("%u\n",pending_check[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STOutputPlaintextTreeCheckDetectorAttackerFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_detector_attacker_filter_slice(stip_length_type length,
                                                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STOutputPlaintextTreeCheckDetectorAttackerFilter,
                        length,
                        min_length);

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
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_detector_has_solution_in_n(slice_index si,
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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_detector_solve_in_n(slice_index si,
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

  detect_pending_check(si);
  result = attack_solve_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STOutputPlaintextTreeCheckDetectorDefenderFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_detector_defender_filter_slice(stip_length_type length,
                                                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STOutputPlaintextTreeCheckDetectorDefenderFilter,
                        length,
                        min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
output_plaintext_tree_check_detector_can_defend_in_n(slice_index si,
                                                     stip_length_type n,
                                                     stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+4;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,n,n_max_unsolvable);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
output_plaintext_tree_check_detector_defend_in_n(slice_index si,
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

  pending_check[nbply] = (attack_gives_check[nbply]
                          ? pending_check_detected
                          : no_pending_check_detected);
  TraceValue("%u",nbply);
  TraceValue("%u\n",pending_check[nbply]);
  result = defense_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
