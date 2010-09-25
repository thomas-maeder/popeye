#include "conditions/ultraschachzwang/goal_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STUltraschachzwangGoalFilter
 */

/* Allocate a STUltraschachzwangGoalFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_ultraschachzwang_goal_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STUltraschachzwangGoalFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ultraschachzwang_goal_filter_has_solution(slice_index si)
{
  has_solution_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = slice_has_solution(next);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ultraschachzwang_goal_filter_solve(slice_index si)
{
  has_solution_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = slice_solve(next);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
ultraschachzwang_goal_filter_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = defense_defend_in_n(next,n,n_max_unsolvable);
  CondFlag[cond] = true;

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
ultraschachzwang_goal_filter_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = defense_can_defend_in_n(next,n,n_max_unsolvable);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
ultraschachzwang_goal_filter_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = attack_solve_in_n(next,n,n_max_unsolvable);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
ultraschachzwang_goal_filter_has_solution_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = attack_has_solution_in_n(next,n,n_max_unsolvable);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void prepend_goal_filter(slice_index si, stip_structure_traversal *st)
{
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.goal_reached_tester.goal.type==goal_mate && CondFlag[cond])
    pipe_append(slices[si].prev,alloc_ultraschachzwang_goal_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors ultraschachzwang_filter_inserters[] =
{
  { STGoalReachedTester, &prepend_goal_filter }
};

enum
{
  nr_ultraschachzwang_filter_inserters =
      (sizeof ultraschachzwang_filter_inserters
       / sizeof ultraschachzwang_filter_inserters[0])
};

/* Instrument a stipulation with Ultraschachzwang goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_ultraschachzwang_goal_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    ultraschachzwang_filter_inserters,
                                    nr_ultraschachzwang_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
