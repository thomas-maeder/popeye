#include "pyleaf.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pymsg.h"
#include "pyint.h"

#include <assert.h>

/* Determine whether a side has reached the goal of a leaf slice.
 * @param camp side
 * @param leaf slice index of leaf slice
 * @return true iff camp has reached leaf's goal
 */
boolean leaf_is_goal_reached(couleur just_moved, slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.target!=initsquare);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_mate:
      if (CondFlag[blackultraschachzwang]
          || CondFlag[whiteultraschachzwang])
        result = goal_checker_mate_ultraschachzwang(just_moved);
      else
        result = goal_checker_mate(just_moved);
      break;

    case goal_stale:
      result = goal_checker_stale(just_moved);
      break;

    case goal_dblstale:
      result = goal_checker_dblstale(just_moved);
      break;

    case goal_target:
      result = goal_checker_target(just_moved,slices[leaf].u.leaf.target);
      break;

    case goal_check:
      result = goal_checker_check(just_moved);
      break;

    case goal_capture:
      result = goal_checker_capture(just_moved);
      break;

    case goal_steingewinn:
      result = goal_checker_steingewinn(just_moved);
      break;

    case goal_ep:
      result = goal_checker_ep(just_moved);
      break;

    case goal_doublemate:
    case goal_countermate:
      result = goal_checker_doublemate(just_moved);
      break;

    case goal_castling:
      result = goal_checker_castling(just_moved);
      break;

    case goal_autostale:
      result = goal_checker_autostale(just_moved);
      break;

    case goal_circuit:
      result = goal_checker_circuit(just_moved);
      break;

    case goal_exchange:
      result = goal_checker_exchange(just_moved);
      break;

    case goal_circuitB:
      result = goal_checker_circuitB(just_moved);
      break;

    case goal_exchangeB:
      result = goal_checker_exchangeB(just_moved);
      break;

    case goal_mate_or_stale:
      result = goal_checker_mate_or_stale(just_moved);
      break;

    case goal_any:
      result = goal_checker_any(just_moved);
      break;

    case goal_proof:
    case goal_atob:
    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Generate (piece by piece) candidate moves for the last move of a s#
 * or r#. Do *not* generate moves for the piece on square
 * initiallygenerated; this piece has already been taken care of.
 * @param camp 
 */
/* TODO Find out whether selflastencore() is really more efficient
 * than the usual genmove() */
static boolean selflastencore(couleur camp,
                              square const **selfbnp,
                              square initiallygenerated)
{
  assert(slices[leaf].type==STLeaf);

  if (encore())
    return true;
  else
  {
    square curr_square = **selfbnp;

    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(advers(camp));

    while (curr_square!=initsquare)
    {
      if (curr_square!=initiallygenerated)
      {
        piece p= e[curr_square];
        if (p!=vide)
        {
          if (TSTFLAG(spec[curr_square],Neutral))
            p = -p;
          if (camp==blanc)
          {
            if (p>obs)
              gen_wh_piece(curr_square,p);
          }
          else
          {
            if (p<vide)
              gen_bl_piece(curr_square,p);
          }
        }
      }
    
      ++*selfbnp;
      curr_square = **selfbnp;

      if (encore())
        return true;
    }

    return false;
  }
} /* selflastencore */

/* Determine whether the side at move must end in 1.
 * @param side_at_move
 * @return true iff side_at_move can end in 1 move
 */
static boolean leaf_is_end_in_1_forced(couleur side_at_move,
                                       slice_index leaf)
{
  boolean is_side_at_move_immobile = true;
  boolean escape_found = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",leaf);
  TraceFunctionParam("%d\n",slices[leaf].u.leaf.goal);

  if (side_at_move==noir ? flagblackmummer : flagwhitemummer)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    genmove(side_at_move);
    move_generation_mode = move_generation_optimized_by_killer_move;

    while (!escape_found
           && encore())
    {
      if (jouecoup()
          && !echecc(side_at_move))
      {
        is_side_at_move_immobile = false;
        escape_found = !leaf_is_goal_reached(side_at_move,leaf);
        if (escape_found)
          coupfort();
      }
      repcoup();
    }
    finply();
  }
  else if (slices[leaf].u.leaf.goal==goal_ep
           && ep[nbply]==initsquare
           && ep2[nbply]==initsquare)
  {
    /* a little optimization if end "state" is en passant capture,
     * but no en passant capture is possible */
    /* TODO Should we play the same trick for castling? Other end
     * states? */
  }
  else
  {
    piece p;
    square const *selfbnp = boardnum;
    square initiallygenerated = initsquare;
    couleur other_side = advers(side_at_move);

    nextply();
    init_move_generation_optimizer();
    trait[nbply]= side_at_move;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(other_side);

    p = e[current_killer_state.move.departure];
    if (p!=vide)
    {
      if (TSTFLAG(spec[current_killer_state.move.departure], Neutral))
        p = -p;
      if (side_at_move==blanc)
      {
        if (p>obs)
        {
          initiallygenerated = current_killer_state.move.departure;
          gen_wh_piece(initiallygenerated,p);
        }
      }
      else
      {
        if (p<-obs)
        {
          initiallygenerated = current_killer_state.move.departure;
          gen_bl_piece(initiallygenerated,p);
        }
      }
    }
    finish_move_generation_optimizer();
    while (!escape_found
           && selflastencore(side_at_move,&selfbnp,initiallygenerated))
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(side_at_move))
      {
        is_side_at_move_immobile = false;
        if (!leaf_is_goal_reached(side_at_move,leaf))
        {
          TraceText("escape_found\n");
          escape_found = true;
          coupfort();
        }
      }
      repcoup();
    }
    finply();
  }

  TraceFunctionExit(__func__);
  TraceValue("%d",escape_found);
  TraceValue("%d",is_side_at_move_immobile);
  TraceFunctionResult("%d\n", !(escape_found || is_side_at_move_immobile));
  return !(escape_found || is_side_at_move_immobile);
}

/* Determine whether attacker has an end in 1.
 * This is different from d_leaf_does_attacker_win() in that
 * leaf_is_end_in_1_possible() doesn't write to the hash table.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
/* TODO find out if this difference makes sense */
boolean leaf_is_end_in_1_possible(couleur side_at_move, slice_index leaf)
{
  boolean end_found = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  if (OptFlag[keepmating] && !is_a_mating_piece_left(side_at_move))
    return false;

  if (slices[leaf].u.leaf.goal==goal_mate)
    GenMatingMove(side_at_move);
  else
    genmove(side_at_move);

  while (encore()
         && !end_found)
  {
    TraceCurrentMove();
    if (jouecoup()
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      TraceText("goal reached\n");
      end_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceValue("%d",leaf);
  TraceFunctionResult("%d\n",end_found);
  return end_found;
}

/* Detect a priori unsolvability of a leaf (e.g. because of a forced
 * reflex mate)
 * @param attacker attacking side
 * @param leaf leaf's slice index
 */
boolean leaf_is_unsolvable(couleur attacker, slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      result = OptFlag[keepmating] && !is_a_mating_piece_left(attacker);
      break;

    case EReflex:
      result = leaf_is_end_in_1_possible(attacker,leaf);
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has lost with his move just played.
 * Assumes that there is no short win for the defending side.
 * @param attacker attacking side
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_leaf_has_defender_lost(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return false;

    case ESelf:
    case EReflex:
    case ESemireflex:
      return leaf_is_goal_reached(defender,leaf);

    default:
      assert(0);
      return false;
  }
}

/* Determine whether a side has just solved a leaf slice in direct play.
 * @param attacker attacking side of predecessor slice, has just moved
 * @param leaf slice index
 * @return true iff attacker has just solved leaf
 */
static boolean d_leaf_is_solved(couleur attacker, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(attacker,leaf);

    case ESelf:
    {
      couleur const defender = advers(attacker);
      return leaf_is_end_in_1_forced(defender,leaf);
    }

    case EReflex:
    case ESemireflex:
    {
      couleur const defender = advers(attacker);
      return leaf_is_end_in_1_possible(defender,leaf);
    }

    default:
      assert(0);
      return false;
  }
}

/* Determine and write forced end moves in 1 by the attacker in reflex
 * stipulations; we know that at least 1 exists.
 * @param attacker attacking side
 */
static void d_leaf_r_solve_forced_keys(couleur attacker, slice_index leaf)
{
  Goal const goal = slices[leaf].u.leaf.goal;

  assert(slices[leaf].type==STLeaf);

  ++zugebene;

  GenMatingMove(attacker);

  while(encore())
  {
    if (jouecoup()
        && leaf_is_goal_reached(attacker,leaf))
      d_write_attack(goal);

    repcoup();
  }

  finply();

  --zugebene;
}

/* Write a priori unsolvability (if any) of a leaf in direct play
 * (e.g. forced reflex mates)
 * @param attacker attacking side
 * @param leaf leaf's slice index
 */
void d_leaf_write_unsolvability(couleur attacker, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EReflex:
      d_leaf_r_solve_forced_keys(attacker,leaf);
      break;

    default:
      break;
  }
}

/* Determine and write keys in a direct stipulation in 1 move
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @return true iff >=1 key was found and written
 */
static boolean d_leaf_d_solve(couleur attacker,
                              boolean restartenabled,
                              slice_index si,
                              int solutions)
{
  boolean const is_try = false;
  boolean key_found = false;

  assert(slices[leaf].type==STLeaf);

  genmove(attacker);
  while (encore())
  {
    if (jouecoup()
        && !echecc(attacker)
        && leaf_is_goal_reached(attacker,si))
    {
      key_found = true;
      d_write_key(slices[si].u.leaf.goal,is_try);
      pushtabsol(solutions);
      StdString("\n\n");
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();
  }

  finply();

  return key_found;
}

/* Determine and write all final moves of a self/reflex variation.
 * @param defender defending side (i.e. side executing the final move(s))
 * @param leaf slice index of the leaf slice
 */
static void d_leaf_sr_solve_final_move(couleur defender, slice_index leaf)
{
  boolean const tree_mode = slices[0].u.composite.play==PDirect; /* TODO */

  assert(slices[leaf].type==STLeaf);

  if (tree_mode)
    StdString("\n");

  GenMatingMove(defender);

  while(encore())
  {
    if (jouecoup()
        && leaf_is_goal_reached(defender,leaf))
    {
      if (tree_mode)
        d_write_defense(slices[leaf].u.leaf.goal);
      else
        linesolution(leaf);
    }

    repcoup();
  }

  finply();
}

/* Determine and write solutions in a self stipulation in 1 move
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * 
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 key was found and written
 */
static boolean d_leaf_s_solve(couleur attacker,
                              boolean restartenabled,
                              slice_index leaf,
                              int solutions)
{
  couleur const defender = advers(attacker);
  boolean key_found = false;

  assert(slices[leaf].type==STLeaf);

  genmove(attacker);

  while (encore())
  {
    if (jouecoup()
        && !echecc(attacker)
        && leaf_is_end_in_1_forced(defender,leaf))
    {
      key_found = true;
      d_write_key(no_goal,false);
      pushtabsol(solutions);
      marge += 4;
      d_leaf_sr_solve_final_move(defender,leaf);
      marge -= 4;
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();
  }

  finply();

  return key_found;
}

/* Determine and write solutions in a reflex stipulation in 1 move
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index of the leaf slice
 * @param solutions table where to add found solutions
 * @return true iff >=1 key was found and written
 */
static boolean d_leaf_r_solve(couleur attacker,
                              boolean restartenabled,
                              slice_index leaf,
                              int solutions)
{
  couleur const defender = advers(attacker);
  boolean key_found = false;

  assert(slices[leaf].type==STLeaf);

  genmove(attacker);

  while (encore())
  {
    if (jouecoup()
        && !echecc(attacker)
        && leaf_is_end_in_1_possible(defender,leaf))
    {
      key_found = true;
      d_write_key(no_goal,false);
      pushtabsol(solutions);
      marge += 4;
      d_leaf_sr_solve_final_move(defender,leaf);
      marge -= 4;
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();
  }

  finply();

  return key_found;
}

/* Write the solutions of a leaf in direct/self/reflex play.
 * Unsolvability (e.g. because of a forced reflex move) has already
 * been delat with.
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index of the leaf slice
 * @param solutions table where to add found solutions
 * @return true iff >=1 key was found and written
 */
boolean d_leaf_solve(couleur attacker,
                     boolean restartenabled,
                     slice_index leaf,
                     int solutions)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return d_leaf_d_solve(attacker,restartenabled,leaf,solutions);

    case ESelf:
      return d_leaf_s_solve(attacker,restartenabled,leaf,solutions);

    case EReflex:
    case ESemireflex:
      return d_leaf_r_solve(attacker,restartenabled,leaf,solutions);

    default:
      assert(0);
      return false;
  }
}

/* Determine whether the defender is not forced to end in 1 in a
 * self stipulation.
 * @param defender defending side (at move)
 * @param leaf slice identifier
 * @return true iff defender wins
 */
boolean d_leaf_s_does_defender_win(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  if (OptFlag[keepmating] && !is_a_mating_piece_left(defender))
    return true;
  else
    return !leaf_is_end_in_1_forced(defender,leaf);
}

/* Determine whether the defender is not forced to end in 1 in a
 * reflex stipulation.
 * @param defender defending side (at move)
 * @param leaf slice identifier
 * @return true iff defender wins
 */
static boolean d_leaf_r_does_defender_win(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  if (OptFlag[keepmating] && !is_a_mating_piece_left(defender))
    return true;
  else
    return !leaf_is_end_in_1_possible(defender,leaf);
}

/* Determine whether the defending side wins
 * @param defender defending side
 * @param leaf slice identifier
 * @return true iff defender wins
 */
boolean d_leaf_does_defender_win(couleur defender, slice_index leaf)
{
  boolean result = true;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    {
      couleur const attacker = advers(defender);
      result = !leaf_is_goal_reached(attacker,leaf);
      break;
    }

    case ESelf:
      result = d_leaf_s_does_defender_win(defender,leaf);
      break;

    case EReflex:
    case ESemireflex:
      result = d_leaf_r_does_defender_win(defender,leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Write the move just played as key of a solution of a leaf
 * @param is_try key true iff we are writing the key of a try
 * @param leaf slice index
 */
static void d_leaf_write_key(boolean is_try, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",is_try);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      d_write_key(slices[leaf].u.leaf.goal,is_try);
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      d_write_key(no_goal,is_try);
      break;


    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
}

/* Find and write threats and verations in direct play
 * @param attacker attacking side (has just played)
 * @param refutations table containing refutations
 * @param leaf slice index
 */
static void d_leaf_solve_threats_variations(couleur attacker,
                                            slice_index leaf)
{
  couleur defender = advers(attacker);

  assert(slices[leaf].type==STLeaf);

  marge += 4;
  d_leaf_solve_variations(defender,leaf);
  marge -= 4;
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param attacker attacking side
 * @param refutations table containing the refutations
 * @param leaf slice index
 * @param is_try key true iff we are writing the key of a try
 */
void d_leaf_write_key_solve_postkey(couleur attacker,
                                    int refutations,
                                    slice_index leaf,
                                    boolean is_try)
{
  assert(slices[leaf].type==STLeaf);

  d_leaf_write_key(is_try,leaf);

  marge+= 4;

  if (OptFlag[solvariantes])
    d_leaf_solve_threats_variations(attacker,leaf);
  else
    Message(NewLine);

  d_write_refutations(refutations);

  marge-= 4;
}

/* Determine whether the attacking side has directly lost by the move
 * that it has just played.
 * @param defender defending side
 * @param leaf slice index
 * @return true iff the attacking side has directly lost
 */
boolean d_leaf_has_attacker_lost(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return false;

    case ESelf:
    case EReflex:
    case ESemireflex:
      return OptFlag[keepmating] && !is_a_mating_piece_left(defender);

    default:
      assert(0);
      return false;
  }
}

/* Determine whether the attacking side has directly won by the move
 * that it has just played.
 * @param defender defending side
 * @param leaf slice index
 * @return true iff the attacking side has directly won
 */
boolean d_leaf_has_attacker_won(couleur defender, slice_index leaf)
{
  couleur const attacker = advers(defender);

  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(attacker,leaf);

    case ESelf:
      return false;

    case EReflex:
    case ESemireflex:
      return leaf_is_end_in_1_possible(defender,leaf);

    default:
      assert(0);
      return false;
  }
}

/* Determine whether attacker can end in 1 move in direct play.
 * @param attacker attacking side (i.e. side attempting to reach the
 * end)
 * @param leaf slice index of leaf slice
 * @param should_hash true iff the findings should be added to the
 *                    hash table
 * @return true iff attacker can end in 1 move
 */
static boolean d_leaf_d_does_attacker_win(couleur attacker,
                                          slice_index leaf,
                                          boolean should_hash)
{
  boolean end_found = false;
  HashBuffer hb;

  assert(slices[leaf].type==STLeaf);

  /* In move orientated stipulations (%, z, x etc.) it's less
   * expensive to compute an end in 1. TLi
   */
  if (should_hash && !FlagMoveOrientatedStip)
  {
    /* It is more likely that a position has no solution. 
     * Therefore let's check for "no solution" first.  TLi
     */
    (*encode)(&hb);
    if (inhash(WhDirNoSucc,1,&hb))
      return false;
    if (inhash(WhDirSucc,1,&hb))
      return true;
  }

  if (OptFlag[keepmating] && !is_a_mating_piece_left(attacker))
    return false;

  GenMatingMove(attacker);

  while (encore() && !end_found)
  {
    if (jouecoup())
    {
      end_found = leaf_is_goal_reached(attacker,leaf);
      if (end_found)
        coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  if (should_hash && !FlagMoveOrientatedStip)
    addtohash(end_found ? WhDirSucc : WhDirNoSucc, 1, &hb);

  return end_found;
}

/* Determine whether the attacker wins in a self/reflex leaf slice
 * @param attacker attacking side (at move)
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
static boolean d_leaf_sr_does_attacker_win(couleur attacker,
                                           slice_index leaf,
                                           boolean should_hash)
{
  boolean win_found = false;
  couleur defender = advers(attacker);
  HashBuffer hb;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  if (should_hash)
  {
    /* It is more likely that a position has no solution. */
    /*    Therefore let's check for "no solution" first. TLi */
    (*encode)(&hb);
    if (inhash(WhDirNoSucc,1,&hb))
    {
      assert(!inhash(WhDirSucc,1,&hb));
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",false);
      return false;
    }
    if (inhash(WhDirSucc,1,&hb))
    {
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",true);
      return true;
    }
  }

  genmove(attacker);

  while (!win_found && encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(attacker)
        && !d_leaf_does_defender_win(defender,leaf))
    {
      TraceText("wins\n");
      win_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  if (should_hash)
    addtohash(win_found ? WhDirSucc : WhDirNoSucc, 1, &hb);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",win_found);
  return win_found;
}

/* Determine whether the attacker wins in a direct/self/reflex
 * stipulation
 * @param attacker attacking side (at move)
 * @param leaf slice index
 * @param should_hash true iff the findings should be added to the
 *                    hash table
 * @return true iff attacker wins
 */
boolean d_leaf_does_attacker_win(couleur attacker,
                                 slice_index si,
                                 boolean should_hash)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].u.leaf.end)
  {
    case EDirect:
      result = d_leaf_d_does_attacker_win(attacker,si,should_hash);
      break;

    case ESelf:
    case ESemireflex:
    case EReflex:
      result = d_leaf_sr_does_attacker_win(attacker,si,should_hash);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write all set play of a self/reflex stipulation.
 * @param defender defending side (i.e. side executing the set play)
 * @param leaf slice index of the leaf slice
 */
static void d_leaf_sr_solve_setplay(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  StdString("\n");

  GenMatingMove(defender);

  while(encore())
  {
    if (jouecoup()
        && leaf_is_goal_reached(defender,leaf))
    {
      d_write_defense(slices[leaf].u.leaf.goal);
      if (OptFlag[maxsols]) 
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();
}

/* Find and write defender's set play in self/reflex play
 * @param defender defending side
 * @param leaf slice index
 */
void d_leaf_solve_setplay(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].type)
  {
    case STLeaf:
      switch (slices[leaf].u.leaf.end)
      {
        case EDirect:
          Message(NewLine);
          break;

        case ESelf:
        case EReflex:
        case ESemireflex:
          d_leaf_sr_solve_setplay(defender,leaf);
          break;

        default:
          assert(0);
          break;
      }
      break;

    default:
      assert(0);
      break;
  }
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param defender defending side
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean d_leaf_solve_complete_set(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
      if (!d_leaf_s_does_defender_win(defender,leaf))
      {
        d_leaf_sr_solve_setplay(defender,leaf);
        return true;
      }
      else
        break;

    case EReflex:
    case ESemireflex:
      if (!d_leaf_r_does_defender_win(defender,leaf))
      {
        d_leaf_sr_solve_setplay(defender,leaf);
        return true;
      }
      else
        break;

    default:
      break;
  }

  return false;
}

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param defender attacking side
 * @param leaf slice index
 */
void d_leaf_solve_variations(couleur defender, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      Message(NewLine);
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      d_leaf_sr_solve_final_move(defender,leaf);
      break;
    
    default:
      assert(0);
      break;
  }
}

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param attacker attacking side
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
void d_leaf_solve_continuations(couleur attacker,
                                int solutions,
                                slice_index leaf)
{
  couleur defender = advers(attacker);

  assert(slices[leaf].type==STLeaf);

  if (slices[leaf].u.leaf.end==EDirect)
    GenMatingMove(attacker);
  else
    genmove(attacker);

  while (encore())
  {
    if (jouecoup()
        && !echecc(attacker)
        && d_leaf_is_solved(attacker,leaf))
    {
      /* TODO function that writes attacker's move just played plus
       * variations (needs only 1 dispatch by end) */
      d_write_attack(slices[leaf].u.leaf.end==EDirect
                     ? slices[leaf].u.leaf.goal
                     : no_goal);
      marge += 4;
      d_leaf_solve_variations(defender,leaf);
      marge -= 4;
      pushtabsol(solutions);
    }

    repcoup();
  }

  finply();
}

/* Find the final (ending) move in a self stipulation
 * @param side_at_move ending side
 * @param leaf slice index
 * @return true iff >=1 ending move was found
 */
static boolean h_leaf_s_solve_final_move(couleur side_at_move,
                                         slice_index leaf)
{
  boolean found_solution = false;

  assert(slices[leaf].type==STLeaf);

  if (!d_leaf_s_does_defender_win(side_at_move,leaf))
  {
    GenMatingMove(side_at_move);

    while (encore())
    {
      if (jouecoup()
          && !echecc(side_at_move)
          && leaf_is_goal_reached(side_at_move,leaf))
      {
        found_solution = true;
        linesolution(leaf);
      }

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in a helpself
 * stipulation.
 * @param side_at_move side at the move
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_s_solve(couleur side_at_move, slice_index leaf)
{
  boolean found_solution = false;
  couleur other_side = advers(side_at_move);

  assert(slices[leaf].type==STLeaf);

  genmove(side_at_move);

  while (encore())
  {
    if (jouecoup()
        && !echecc(side_at_move))
      found_solution = h_leaf_s_solve_final_move(other_side,leaf);

    repcoup();
  }

  finply();

  return found_solution;
}

/* Find the final (ending) move in a reflex stipulation
 * @param side_at_move ending side
 * @param leaf slice index
 * @return true iff >=1 ending move was found
 */
static boolean h_leaf_r_solve_final_move(couleur side_at_move,
                                         slice_index leaf)
{
  boolean found_solution = false;

  assert(slices[leaf].type==STLeaf);

  if (!d_leaf_r_does_defender_win(side_at_move,leaf))
  {
    GenMatingMove(side_at_move);
    while (encore())
    {
      if (jouecoup()
          && leaf_is_goal_reached(side_at_move,leaf))
      {
        found_solution = true;
        linesolution(leaf);
      }

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in a helpreflex
 * stipulation.
 * @param side_at_move side at the move
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_r_solve(couleur side_at_move, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);

  if (slices[leaf].u.leaf.end==EReflex
      && leaf_is_end_in_1_possible(side_at_move,leaf))
    return false;
  else
  {
    boolean found_solution = false;
    couleur other_side = advers(side_at_move);

    genmove(side_at_move);

    while (encore())
    {
      if (jouecoup()
          && !echecc(side_at_move))
        found_solution = h_leaf_r_solve_final_move(other_side,leaf);

      repcoup();
    }

    finply();

    return found_solution;
  }
}

#if !defined(DATABASE)
/* Determine and find final moves in a help stipulation
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
boolean h_leaf_h_solve_ending_move(couleur side_at_move, slice_index leaf)
{
  boolean final_move_found = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  GenMatingMove(side_at_move);

  if (side_at_move==blanc)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && (!OptFlag[intelligent] || MatePossible()))
    {
      if (leaf_is_goal_reached(side_at_move,leaf))
      {
        final_move_found = true;
        linesolution(leaf);
      }
    }
    repcoup();
  }

  if (side_at_move==blanc)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",final_move_found);
  return final_move_found;
}
#endif

/* Determine and write the final move pair in help countermate.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_h_cmate_solve(couleur side_at_move,
                                    hashwhat no_succ_hash_category,
                                    boolean restartenabled,
                                    slice_index leaf)
{
  boolean found_solution = false;
  couleur other_side = advers(side_at_move);

  assert(slices[leaf].type==STLeaf);

  GenMatingMove(side_at_move);

  while (encore())
  {
    if (jouecoup()
        && !echecc(side_at_move)
        && !(restartenabled && MoveNbr<RestartNbr))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(no_succ_hash_category,1,&hb))
      {
        if (goal_checker_mate(side_at_move))
        {
          GenMatingMove(other_side);

          while (encore())
          {
            if (jouecoup()
                && leaf_is_goal_reached(other_side,leaf))
            {
              found_solution = true;
              linesolution(leaf);
            }
            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(no_succ_hash_category,1,&hb);
      }
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();
  }

  finply();

  return found_solution;
}

/* Determine and write the final move pair in help doublemate.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_h_dmate_solve(couleur side_at_move,
                                    hashwhat no_succ_hash_category,
                                    boolean restartenabled,
                                    slice_index leaf)
{
  boolean found_solution = false;
  couleur other_side = advers(side_at_move);

  assert(slices[leaf].type==STLeaf);

  genmove(side_at_move);

  while (encore())
  {
    if (jouecoup()
        && !echecc(side_at_move)
        && !(restartenabled && MoveNbr<RestartNbr))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(no_succ_hash_category,1,&hb))
      {
        if (!immobile(other_side))
        {
          GenMatingMove(other_side);

          while (encore())
          {
            if (jouecoup()
                && leaf_is_goal_reached(other_side,leaf))
            {
              found_solution = true;
              linesolution(leaf);
            }

            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(no_succ_hash_category,1,&hb);
      }
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  return found_solution;
}

/* Determine and write the final move pair in help stipulation with
 * "regular" goal. 
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_h_othergoals_solve(couleur side_at_move,
                                         hashwhat no_succ_hash_category,
                                         boolean restartenabled,
                                         slice_index leaf)
{
  boolean found_solution = false;
  couleur other_side = advers(side_at_move);

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(side_at_move);

  if (side_at_move==noir)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && (!OptFlag[intelligent] || MatePossible())
        && !echecc(side_at_move)
        && !(restartenabled && MoveNbr<RestartNbr))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(no_succ_hash_category,1,&hb))
      {
        if (h_leaf_h_solve_ending_move(other_side,leaf))
          found_solution = true;
        else
          addtohash(no_succ_hash_category,1,&hb);
      }
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  if (side_at_move==noir)
    BlMovesLeft++;
  else
    WhMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine and write the solution of a help leaf slice in help play.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean h_leaf_h_solve(couleur side_at_move,
                              hashwhat no_succ_hash_category,
                              boolean restartenabled,
                              slice_index leaf)
{
  boolean result;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = h_leaf_h_cmate_solve(side_at_move,
                                    no_succ_hash_category,
                                    restartenabled,
                                    leaf);
      break;

    case goal_doublemate:
      result = h_leaf_h_dmate_solve(side_at_move,
                                    no_succ_hash_category,
                                    restartenabled,
                                    leaf);
      break;


    default:
      result = h_leaf_h_othergoals_solve(side_at_move,
                                         no_succ_hash_category,
                                         restartenabled,
                                         leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write the solution of a leaf slice in help play.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
boolean h_leaf_solve(couleur side_at_move,
                     hashwhat no_succ_hash_category,
                     boolean restartenabled,
                     slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
      result = h_leaf_s_solve(side_at_move,leaf);
      break;

    case EReflex:
    case ESemireflex:
      result = h_leaf_r_solve(side_at_move,leaf);
      break;

    case EHelp:
      result = h_leaf_h_solve(side_at_move,
                              no_succ_hash_category,
                              restartenabled,
                              leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Solve the set play in a help stipulation
 * @param side_at_move side at move (going to play set move)
 * @param leaf slice index
 * @return true iff >=1 set play was found
 */
boolean h_leaf_solve_setplay(couleur side_at_move, slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
      result = h_leaf_s_solve_final_move(side_at_move,leaf);
      break;

    case EReflex:
    case ESemireflex:
      result = h_leaf_r_solve_final_move(side_at_move,leaf);
      break;

    case EHelp:
      result = h_leaf_h_solve_ending_move(side_at_move,leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write final move of the attacker in a series
 * direct stipulation.
 * This is different from non-series play because series solutions are
 * written one 1 line each, while non-series solutions are written in
 * tree form.
 * @param attacker attacking side
 * @param leaf slice index
 * @return true iff >= 1 final move (sequence) was found
 */
static boolean ser_leaf_d_solve(couleur attacker, slice_index leaf)
{
  boolean solution_found = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  GenMatingMove(attacker);

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && leaf_is_goal_reached(attacker,leaf))
    {
      TraceText("solution found\n");
      linesolution(leaf);
      solution_found = true;
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
} /* ser_leaf_d_solve */

/* Determine and write final move of the attacker in a series
 * self/reflex stipulation, plus the (subsequent) final move of the
 * defender.
 * This is different from non-series play because series solutions are
 * written one 1 line each, while non-series solutions are written in
 * tree form.
 * @param attacker attacking side
 * @param leaf slice index
 * @return true iff >= 1 final move (sequence) was found
 */
static boolean ser_leaf_sr_solve(couleur attacker, slice_index leaf)
{
  couleur defender = advers(attacker);
  boolean solution_found = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(attacker);

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(attacker)
        && !d_leaf_does_defender_win(defender,leaf))
    {
      TraceText("solution found\n");
      solution_found = true;
      d_leaf_sr_solve_final_move(defender,leaf);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
} /* ser_leaf_sr_solve */

/* Determine and write the solution of a leaf slice in series play.
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
boolean ser_leaf_solve(couleur series_side,
                       hashwhat no_succ_hash_category,
                       boolean restartenabled,
                       slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  TraceValue("%d\n",slices[leaf].u.leaf.end);
  switch (slices[leaf].u.leaf.end)
  {
    case EHelp:
      result = h_leaf_h_solve(series_side,
                              no_succ_hash_category,
                              restartenabled,
                              leaf);
      break;

    case EDirect:
      result = ser_leaf_d_solve(series_side,leaf);
      break;

    case EReflex:
      if (!leaf_is_end_in_1_possible(series_side,leaf))
        result = ser_leaf_sr_solve(series_side,leaf);
      break;

    case ESelf:
    case ESemireflex:
      result = ser_leaf_sr_solve(series_side,leaf);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}
