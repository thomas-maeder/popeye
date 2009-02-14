#include "pyleaff.h"
#include "pyleafs.h"
#include "pyleaf.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */


/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_forced_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].u.leaf.starter;

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_forced_has_starter_apriori_lost(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Generate (piece by piece) candidate moves for the last move of a s#.
 * Do *not* generate moves for the piece on square
 * initiallygenerated; this piece has already been taken care of.
 */
/* TODO Find out whether selflastencore() is really more efficient
 * than the usual genmove() */
static boolean selflastencore(square const **selfbnp,
                              square initiallygenerated,
                              Side defender)
{
  if (encore())
    return true;
  else
  {
    Side const attacker = advers(defender);
    square curr_square = **selfbnp;

    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(attacker);

    while (curr_square!=initsquare)
    {
      if (curr_square!=initiallygenerated)
      {
        piece p= e[curr_square];
        if (p!=vide)
        {
          if (TSTFLAG(spec[curr_square],Neutral))
            p = -p;
          if (defender==White)
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
}

/* Determine whether the side at move must end in 1.
 * @return true iff side_at_move can end in 1 move
 */
static boolean is_end_in_1_forced(Side defender, slice_index leaf)
{
  boolean is_defender_immobile = true;
  boolean escape_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",defender);
  TraceFunctionParam("%u\n",slices[leaf].u.leaf.goal);

  if (defender==Black ? flagblackmummer : flagwhitemummer)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    genmove(defender);
    move_generation_mode = move_generation_optimized_by_killer_move;

    while (!escape_found
           && encore())
    {
      if (jouecoup(nbply,first_play)
          && !echecc(nbply,defender))
      {
        is_defender_immobile = false;
        /* TODO this checks for echecc(nbply,defender) again (in most cases
         * anyway); optimise? */
        escape_found = !leaf_is_goal_reached(defender,leaf);
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
    Side const attacker = advers(defender);

    nextply(nbply);
    init_move_generation_optimizer();
    trait[nbply]= defender;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(attacker);

    p = e[current_killer_state.move.departure];
    if (p!=vide)
    {
      if (TSTFLAG(spec[current_killer_state.move.departure], Neutral))
        p = -p;
      if (defender==White)
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
           && selflastencore(&selfbnp,initiallygenerated,defender))
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,defender))
      {
        is_defender_immobile = false;
        /* TODO this checks for echecc(nbply,defender) again (in most cases
         * anyway); optimise? */
        if (!leaf_is_goal_reached(defender,leaf))
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
  TraceValue("%u",escape_found);
  TraceValue("%u",is_defender_immobile);
  TraceFunctionResult("%u\n", !(escape_found || is_defender_immobile));
  return !(escape_found || is_defender_immobile);
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_forced_has_starter_won(slice_index leaf)
{
  boolean result;
  Side const defender = slices[leaf].u.leaf.starter;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = is_end_in_1_forced(defender,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice leaf's goal with
 * his move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_forced_has_starter_reached_goal(slice_index leaf)
{
  boolean const result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param leaf identifies leaf
 * @return true iff the defender wins
 */
boolean leaf_forced_does_defender_win(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].u.leaf.starter;

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  /* TODO do we check for keepmating... twice
   * (cf. leaf_forced_must_starter_resign())?
   */
  result = ((OptFlag[keepmating] && !is_a_mating_piece_left(defender))
            || !is_end_in_1_forced(defender,leaf));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_forced_has_non_starter_solved(slice_index leaf)
{
  Side const defender = slices[leaf].u.leaf.starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceValue("%u\n",defender);

  result = leaf_is_goal_reached(defender,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a move by the non-starter that has reached a leaf's goal
 * @param leaf slice index of leaf
 */
void leaf_forced_write_non_starter_has_solved(slice_index leaf)
{
  write_final_defense(slices[leaf].u.leaf.goal);
}

/* Determine and find final moves
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean solve_final_move(slice_index leaf)
{
  Side const defender = slices[leaf].u.leaf.starter;
  boolean final_move_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",defender);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,defender);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(defender,leaf))
    {
      final_move_found = true;
      write_final_defense(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",final_move_found);
  return final_move_found;
}

/* Determine and write the variations after the move that has just
 * been played in the current ply.
 * We have already determined that >=1 move reaching the goal is forced
 * @param si slice index
 */
void leaf_forced_solve_variations(slice_index leaf)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  output_start_postkey_level();
  solve_final_move(leaf);
  output_end_postkey_level();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  if (leaf_forced_does_defender_win(leaf))
    result = false;
  else
  {
    result = true;

    output_start_postkey_level();
    solve_final_move(leaf);
    output_end_postkey_level();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solution of a leaf slice at root level.
 * @param leaf identifies leaf slice
 */
void leaf_forced_root_solve(slice_index leaf)
{
  Side const defender = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  if (is_end_in_1_forced(defender,leaf))
  {
    output_start_postkey_level();
    solve_final_move(leaf);
    output_end_postkey_level();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_forced_detect_starter(slice_index leaf,
                                                  boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  slices[leaf].u.leaf.starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
