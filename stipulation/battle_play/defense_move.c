#include "stipulation/battle_play/defense_move.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/move.h"
#include "pyoutput.h"
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

  if (min_length<slack_length_battle)
    min_length += 2;
  assert(min_length>=slack_length_battle);
  result = alloc_branch(STDefenseMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_insert_root(slice_index si, stip_structure_traversal *st)
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

    slices[si].u.branch.length -= 2;
    if (slices[si].u.branch.min_length>=slack_length_battle+2)
      slices[si].u.branch.min_length -= 2;
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
    slices[si].starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean defense_move_root_defend(slice_index si, stip_length_type n_min)
{
  Side const defender = slices[si].starter;
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && attack_solve_in_n(next,length-1,n_min-1)>=length)
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean defense_move_defend_in_n(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_min)
{
  boolean result = false;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && attack_solve_in_n(next,n-1,n_min-1)>=n)
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void generate_other_pieces(square generated_last, Side defender)
{
  square const *selfbnp;

  TraceFunctionEntry(__func__);
  TraceSquare(generated_last);
  TraceEnumerator(Side,defender,"");
  TraceFunctionParamListEnd();

  for (selfbnp = boardnum; *selfbnp!=initsquare; ++selfbnp)
    if (*selfbnp!=generated_last)
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
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void genmove_priorise_killer_piece(Side defender,
                                          square killerDepartureSquare,
                                          piece killerPiece)
{
  Side const attacker = advers(defender);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killerDepartureSquare);
  TracePiece(killerPiece);
  TraceFunctionParamListEnd();

  nextply(nbply);
  trait[nbply] = defender;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(attacker);
  if (TSTFLAG(spec[killerDepartureSquare],Neutral))
    killerPiece = -e[killerDepartureSquare];

  init_move_generation_optimizer();

  generate_other_pieces(killerDepartureSquare,defender);

  if (defender==White)
  {
    if (killerPiece>obs)
      gen_wh_piece(killerDepartureSquare,killerPiece);
  }
  else
  {
    if (killerPiece<-obs)
      gen_bl_piece(killerDepartureSquare,killerPiece);
  }

  finish_move_generation_optimizer();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void genmove_last_self_defense(slice_index si, Side defender)
{
  square const killerDepartureSquare = kpilcd[nbply+1];
  piece const killerPiece = e[killerDepartureSquare];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceFunctionParamListEnd();

  TraceSquare(killerDepartureSquare);
  TracePiece(killerPiece);
  TraceText("\n");

  if (slices[si].u.branch.imminent_goal==goal_ep
      && ep[nbply]==initsquare
      && ep2[nbply]==initsquare)
  {
    /* nothing */
    /* TODO transform to defender filter */
    /* TODO ?create other filters? */
  }
  else
  {
    if ((defender==Black ? flagblackmummer : flagwhitemummer)
        || killerPiece==obs || killerPiece==vide)
      genmove(defender);
    else
      genmove_priorise_killer_piece(defender,killerDepartureSquare,killerPiece);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_nr_refutations how many refutations should we look for
 * @return n+4 refuted - >max_nr_refutations refutations found
           n+2 refuted - <=max_nr_refutations refutations found
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
 */
stip_length_type defense_move_can_defend_in_n(slice_index si,
                                              stip_length_type n,
                                              unsigned int max_nr_refutations)
{
  Side const defender = slices[si].starter;
  stip_length_type result = 0;
  unsigned int nr_refutations = 0;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n+1-slack_length_battle)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n-1+min_length>slack_length_battle+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_battle-parity;

  active_slice[nbply+1] = si;

  /* TODO create a design for representing all these move generation
   * modes
   */
  if (n-1>slack_length_battle)
  {
    move_generation_mode = move_generation_mode_opti_per_side[defender];
    TraceValue("->%u\n",move_generation_mode);
    genmove(defender);
  }
  else
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    TraceValue("->%u\n",move_generation_mode);
    if (n-1==slack_length_battle || slices[si].u.branch.imminent_goal==no_goal)
      genmove(defender);
    else
      genmove_last_self_defense(si,defender);
  }

  while (nr_refutations<=max_nr_refutations && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const length_sol = attack_has_solution_in_n(next,
                                                                   n-1,
                                                                   n_min_next);
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

  finply();

  if (result==0 || nr_refutations>max_nr_refutations)
    result = n+4;
  else if (nr_refutations>0)
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void defense_move_make_setplay_slice(slice_index si,
                                     stip_structure_traversal *st)
{
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    prod->setplay_slice = alloc_help_move_slice(length,min_length);
    pipe_set_successor(prod->setplay_slice,slices[si].u.pipe.next);
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
