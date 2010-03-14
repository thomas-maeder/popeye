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

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int leaf_forced_count_refutations(slice_index leaf,
                                           unsigned int max_result)
{
  Side const defender = slices[leaf].starter;
  boolean is_defender_immobile = true;
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",max_result);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[leaf].u.leaf.goal);

  if (defender==Black ? flagblackmummer : flagwhitemummer)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    genmove(defender);

    while (result<=max_result && encore())
    {
      if (jouecoup(nbply,first_play)
          && !echecc(nbply,defender))
      {
        is_defender_immobile = false;
        /* TODO this checks for echecc(nbply,defender) again (in most cases
         * anyway); optimise? */
        if (leaf_is_goal_reached(defender,leaf)!=goal_reached)
        {
          coupfort();
          ++result;
        }
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
    while (result<=max_result
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
          ++result;
          coupfort();
        }
      }
      repcoup();
    }
    finply();
  }

  if (is_defender_immobile)
    result = max_result+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and find final moves
 * @param leaf slice index
 */
static void solve_final_move(slice_index leaf)
{
  Side const defender = slices[leaf].starter;

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
      write_final_defense(slices[leaf].u.leaf.goal);

    repcoup();
  }

  finply();

  output_end_leaf_variation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_forced_defend(slice_index leaf)
{
  boolean result = false;
  Side const defender = slices[leaf].starter;

  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (is_end_in_1_forced(defender,leaf))
  {
    result = false;
    write_attack(attack_regular);
    solve_final_move(leaf);
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
boolean leaf_forced_has_solution(slice_index leaf)
{
  boolean result;
  Side const defender = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = is_end_in_1_forced(defender,leaf);

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

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf)
{
  Side const defender = slices[leaf].starter;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (is_end_in_1_forced(defender,leaf))
  {
    result = true;

    output_start_postkey_level();
    solve_final_move(leaf);
    output_end_postkey_level();
  }
  else
    result = false;

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
    write_end_of_solution_phase();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_forced_root_defend(slice_index leaf,
                                unsigned int max_number_refutations)
{
  Side const defender = slices[leaf].starter;
  boolean result = true;
  table const refutations = allocate_table();
  boolean have_we_reached_goal = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[leaf].u.leaf.goal);

  move_generation_mode = move_generation_optimized_by_killer_move;
  genmove(defender);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (leaf_is_goal_reached(defender,leaf))
      {
        case goal_reached:
          have_we_reached_goal = true;
          break;

        case goal_not_reached:
          if (!echecc(nbply,defender))
          {
            append_to_top_table();
            coupfort();
          }
          break;

        case goal_not_reached_selfcheck:
          /* nothing */
          break;

        default:
          assert(0);
      }

    repcoup();

    if (table_length(refutations)>max_number_refutations)
      break;
  }

  finply();

  if (table_length(refutations)==0)
  {
    if (have_we_reached_goal)
    {
      result = false;
      write_attack(attack_key);
      solve_final_move(leaf);
      write_end_of_solution();
    }
    else
      /* defender immobile and goal not stalemate */
      result = true;
  }
  else if (table_length(refutations)<=max_number_refutations)
  {
    result = true;
    write_attack(attack_try);
    solve_final_move(leaf);
    write_refutations(refutations);
    write_end_of_solution();
  }

  free_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_forced_detect_starter(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
