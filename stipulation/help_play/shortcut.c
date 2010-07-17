#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/play.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpShortcut slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return allocated slice
 */
slice_index alloc_help_shortcut(stip_length_type length,
                                stip_length_type min_length,
                                slice_index short_sols)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpShortcut,length,min_length); 
  slices[result].u.shortcut.short_sols = short_sols;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void help_shortcut_resolve_proxies(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].u.shortcut.short_sols!=no_slice)
    proxy_slice_resolve(&slices[si].u.shortcut.short_sols);

  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a help shortcut slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_help_shortcut(slice_index si, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==slices[si].u.shortcut.length)
    stip_traverse_moves_pipe(si,st);
  else
    stip_traverse_moves(slices[si].u.shortcut.short_sols,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_shortcut_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const full_length = slices[si].u.shortcut.length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const short_sols = slices[si].u.shortcut.short_sols;
  slice_index next_branch = n<full_length ? short_sols : next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_solve_in_n(next_branch,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_shortcut_has_solution_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  stip_length_type const full_length = slices[si].u.shortcut.length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const short_sols = slices[si].u.shortcut.short_sols;
  slice_index next_branch = n<full_length ? short_sols : next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_has_solution_in_n(next_branch,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
