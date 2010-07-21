#include "stipulation/battle_play/defense_move.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseMove defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_move_slice(stip_length_type length,
                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDefenseMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void defense_move_make_setplay_slice(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>slack_length_battle)
  {
    stip_length_type const length_h = (length-slack_length_battle
                                       +slack_length_help);
    *result = alloc_help_move_slice(length_h,length_h);
    pipe_set_successor(*result,slices[si].u.branch.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void defense_move_reduce_to_postkey_play(slice_index si,
                                         stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_make_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index * const root = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    *root = alloc_defense_move_slice(length,min_length);
    pipe_set_successor(*root,slices[si].u.pipe.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void defense_move_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure_pipe(si,st);
    slices[si].starter = advers(slices[slices[si].u.pipe.next].starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type defense_move_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result = n_min;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = (n-slack_length_battle)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n_min>=slack_length_battle);

  if (n_min==slack_length_battle)
    n_min = slack_length_battle+2;

  n_max_unsolvable = slack_length_battle-parity;

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const nr_moves_needed
          = attack_solve_in_n(next,n-1,n_min-1,n_max_unsolvable-1)+1;
      if (nr_moves_needed>result)
        result = nr_moves_needed;
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean last_defense_stalemate;

/* Try the defenses generated in the current ply
 * @param si identifies slice
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int try_last_defenses(slice_index si,
                                      unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result = 0;
  stip_length_type const n_max_unsolvable = slack_length_battle-2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  while (result<=max_nr_refutations && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      switch (attack_has_solution_in_n(next,
                                       slack_length_battle,slack_length_battle,
                                       n_max_unsolvable))
      {
        case slack_length_battle-2:
          break;

        case slack_length_battle:
          last_defense_stalemate = false;
          break;

        case slack_length_battle+2:
          last_defense_stalemate = false;
          ++result;
          coupfort();
          break;

        default:
          assert(0);
          break;
      }
    }

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses by pieces other than the killer piece
 * @param si identifies slice
 * @param defender defending side
 * @param killer_pos square occupied by killer piece
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int iterate_non_killer(slice_index si,
                                       Side defender,
                                       square killer_pos,
                                       unsigned int max_nr_refutations)
{
  square const *selfbnp;
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killer_pos);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  for (selfbnp = boardnum;
       *selfbnp!=initsquare && result<=max_nr_refutations;
       ++selfbnp)
    if (*selfbnp!=killer_pos)
    {
      piece p = e[*selfbnp];
      if (p!=vide)
      {
        if (TSTFLAG(spec[*selfbnp],Neutral))
          p = -p;
        if (defender==White)
        {
          if (p>obs)
            gen_wh_piece(*selfbnp,p);
        }
        else
        {
          if (p<vide)
            gen_bl_piece(*selfbnp,p);
        }
      }

      result += try_last_defenses(si,max_nr_refutations-result);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses first by the killer piece, then by the other pieces
 * @param si identifies slice
 * @param killer_pos square occupied by killer piece
 * @param defender defending side
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int iterate_killer(slice_index si,
                                   Side defender,
                                   square killer_pos,
                                   piece killer,
                                   unsigned int max_nr_refutations)
{
  Side const attacker = advers(defender);
  unsigned int result;
    
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);

  nextply(nbply);
  trait[nbply] = defender;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(attacker);
  if (TSTFLAG(spec[killer_pos],Neutral))
    killer = -e[killer_pos];

  init_move_generation_optimizer();

  if (defender==White)
  {
    if (killer>obs)
      gen_wh_piece(killer_pos,killer);
  }
  else
  {
    if (killer<-obs)
      gen_bl_piece(killer_pos,killer);
  }

  finish_move_generation_optimizer();

  result = try_last_defenses(si,max_nr_refutations);

  if (result<=max_nr_refutations)
  {
    unsigned int const remaining = max_nr_refutations-result;
    result += iterate_non_killer(si,defender,killer_pos,remaining);
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try last defenses
 * @param si identifies slice
 * @param defender defending side
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static
unsigned int iterate_last_self_defenses(slice_index si,
                                        Side defender,
                                        unsigned int max_nr_refutations)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.imminent_goal.type==goal_ep
      && ep[nbply]==initsquare
      && ep2[nbply]==initsquare)
  {
    /* nothing */
    /* TODO transform to defender filter */
    /* TODO ?create other filters? */
    result = max_nr_refutations+1;
  }
  else
  {
    square const killer_pos = kpilcd[nbply+1];
    piece const killer = e[killer_pos];

    TraceSquare(killer_pos);
    TracePiece(killer);
    TraceText("\n");

    last_defense_stalemate = true;

    if ((defender==Black ? flagblackmummer : flagwhitemummer)
        || killer==obs || killer==vide)
    {
      move_generation_mode = move_generation_optimized_by_killer_move;
      TraceValue("->%u\n",move_generation_mode);
      genmove(defender);
      result = try_last_defenses(si,max_nr_refutations);
      finply();
    }
    else
      result = iterate_killer(si,
                              defender,
                              killer_pos,
                              killer,
                              max_nr_refutations);

    if (last_defense_stalemate)
      result = max_nr_refutations+1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try the defenses generated in the current ply
 * @param si identifies slice
 * @param max_nr_refutations maximum number of refutations to look for
 * @return n_min-2  defending side is stalemated
 *         n_min..n length of longest continuation (no refutation found)
 *         n+2      >=1 refutation found
 */
static stip_length_type try_defenses(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min,
                                     unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n_min-2;
  unsigned int nr_refutations = 0;
  stip_length_type const parity = (n-slack_length_battle)%2;
  stip_length_type n_max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  n_max_unsolvable = slack_length_battle-parity;

  while (nr_refutations<=max_nr_refutations && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const
          length_sol = attack_has_solution_in_n(next,
                                                n-1,n_min-1,
                                                n_max_unsolvable-1);
      if (length_sol>=n)
      {
        ++nr_refutations;
        coupfort();
      }

      if (length_sol>result)
        result = length_sol+1;
    }

    repcoup();
  }

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
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
defense_move_can_defend_in_n(slice_index si,
                             stip_length_type n,
                             stip_length_type n_max_unsolvable,
                             unsigned int max_nr_refutations)
{
  Side const defender = slices[si].starter;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n==slack_length_battle+1
      && slices[si].u.branch.imminent_goal.type!=no_goal)
    result = (iterate_last_self_defenses(si,defender,max_nr_refutations)==0
              ? slack_length_battle+1
              : slack_length_battle+5);
  else
  {
    stip_length_type const n_min = battle_branch_calc_n_min(si,n);
    stip_length_type max_len_continuation;

    if (n<=slack_length_battle+3)
      move_generation_mode = move_generation_optimized_by_killer_move;
    else
      move_generation_mode = move_generation_mode_opti_per_side[defender];

    TraceValue("->%u\n",move_generation_mode);
    genmove(defender);
    max_len_continuation = try_defenses(si,n,n_min,max_nr_refutations);
    finply();

    if (max_len_continuation<n_min /* stalemate */
        || max_len_continuation>n) /* refuted */
      result = n+4;
    else
      result = max_len_continuation;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
