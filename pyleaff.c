#include "pyleaff.h"
#include "pyleaf.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */


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
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(**selfbnp);
  TraceSquare(initiallygenerated);
  TraceFunctionParam("%u",defender);
  TraceFunctionParamListEnd();

  if (encore())
    result = true;
  else
  {
    Side const attacker = advers(defender);
    square curr_square = **selfbnp;

    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(attacker);

    while (!result && curr_square!=initsquare)
    {
      if (curr_square!=initiallygenerated)
      {
        piece p = e[curr_square];
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
        result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the side at move must end in 1.
 * @return true iff side_at_move can end in 1 move
 */
static boolean is_end_in_1_forced(Side defender, slice_index leaf)
{
  boolean is_defender_immobile = true;
  boolean escape_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",defender);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[leaf].u.leaf.goal);

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
        escape_found = leaf_is_goal_reached(defender,leaf)!=goal_reached;
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

    active_slice[nbply+1] = leaf;
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
        if (leaf_is_goal_reached(defender,leaf)!=goal_reached)
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

/* Write the key
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_forced_root_write_key(slice_index leaf, attack_type type)
{
  write_attack(type);
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return whether starter has won
 */
has_starter_won_result_type leaf_forced_has_starter_won(slice_index leaf)
{
  boolean result;
  Side const defender = slices[leaf].starter;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  result = (is_end_in_1_forced(defender,leaf)
            ? starter_has_won
            : starter_has_not_won);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_starter_won_result_type,result,"");
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param leaf identifies leaf
 * @return true iff the defender wins
 */
boolean leaf_forced_does_defender_win(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].starter;

  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = !is_end_in_1_forced(defender,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_forced_has_non_starter_solved(slice_index leaf)
{
  Side const defender = slices[leaf].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  TraceValue("%u\n",defender);

  result = leaf_is_goal_reached(defender,leaf)==goal_reached;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side const defender = slices[leaf].starter;
  boolean final_move_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",defender);
  TraceFunctionParamListEnd();

  output_start_leaf_variation_level();

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,defender);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(defender,leaf)==goal_reached)
    {
      final_move_found = true;
      write_final_defense(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  finply();

  output_end_leaf_variation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",final_move_found);
  TraceFunctionResultEnd();
  return final_move_found;
}

/* Determine and write the postkey play after the move that has just
 * been played in the current ply.
 * We have already determined that >=1 move reaching the goal is forced
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve_postkey(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = solve_final_move(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution of a leaf slice at root level.
 * @param leaf identifies leaf slice
 * @return true iff >=1 key was found and written
 */
boolean leaf_forced_root_solve(slice_index leaf)
{
  Side const defender = slices[leaf].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  init_output(leaf);

  if (is_end_in_1_forced(defender,leaf))
  {
    result = true;
    output_start_postkey_level();
    solve_final_move(leaf);
    output_end_postkey_level();
  }

  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
leaf_forced_root_find_refutations(table refutations,
                                  slice_index leaf)
{
  Side const defender = slices[leaf].starter;
  quantity_of_refutations_type result = attacker_has_reached_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",defender);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[leaf].u.leaf.goal);

  if (!echecc(nbply,advers(defender)))
  {
    if (defender==Black ? flagblackmummer : flagwhitemummer)
    {
      move_generation_mode = move_generation_optimized_by_killer_move;
      genmove(defender);
      move_generation_mode = move_generation_optimized_by_killer_move;

      while (table_length(refutations)<=max_nr_refutations
             && encore())
      {
        if (jouecoup(nbply,first_play))
          switch (leaf_is_goal_reached(defender,leaf))
          {
            case goal_reached:
              if (result==attacker_has_reached_deadend)
                result = found_no_refutation;
              break;

            case goal_not_reached:
              if (!echecc(nbply,defender))
              {
                append_to_top_table();
                coupfort();
                result = found_refutations;
              }
              break;

            case goal_not_reached_selfcheck:
              /* nothing */
              break;

            default:
              assert(0);
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

      active_slice[nbply+1] = leaf;
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

      while (table_length(refutations)<=max_nr_refutations
             && selflastencore(&selfbnp,initiallygenerated,defender))
      {
        if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
          switch (leaf_is_goal_reached(defender,leaf))
          {
            case goal_reached:
              if (result==attacker_has_reached_deadend)
                result = found_no_refutation;
              break;

            case goal_not_reached:
              if (!echecc(nbply,defender))
              {
                append_to_top_table();
                coupfort();
                result = found_refutations;
              }
              break;

            case goal_not_reached_selfcheck:
              /* nothing */
              break;

            default:
              assert(0);
          }

        repcoup();
      }

      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_root_solve_postkey(table refutations, slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = solve_final_move(leaf);
  write_refutations(refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  slices[leaf].starter = Black;
  TraceValue("->%u\n",slices[leaf].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
