#include "republican.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pylang.h"
#include "stipulation/has_solution_type.h"
#include "solving/battle_play/attack_play.h"
#include "pymsg.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/temporary_hacks.h"

#include <assert.h>
#include <string.h>

/* TODO make static */
boolean is_republican_suspended;

static pilecase republican_king_placement;

static Goal republican_goal = { no_goal, initsquare };

/* TODO implement independently from SuperCirce et al.
 */

/* Find a square for the opposite king
 * @param camp side looking for a square for the opposite king
 */
static void find_mate_square(Side side)
{
  Side const other_side = advers(side);
  piece const king_type = other_side==Black ? roin : roib;
  assert(republican_goal.type==goal_mate);

  king_square[other_side] = ++super[nbply];
  ++nbpiece[king_type];
  while (king_square[other_side]<=square_h8)
  {
    if (e[king_square[other_side]]==vide)
    {
      e[king_square[other_side]]= king_type;
      if (attack(slices[temporary_hack_mate_tester[other_side]].next2,length_unspecified)==has_solution)
        return;
      e[king_square[other_side]]= vide;
    }

    king_square[other_side] = ++super[nbply];
  }

  --nbpiece[king_type];
  king_square[other_side] = initsquare;
}

/* Perform the necessary verification steps for solving a Republican
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean republican_verifie_position(slice_index si)
{
  if (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction])
  {
    VerifieMsg(IncompatibleRoyalSettings);
    return false;
  }
  else if (CondFlag[masand]) /* TODO what else should we prohibit here? */
  {
    VerifieMsg(NoRepublicanWithConditionsDependingOnCheck);
    return false;
  }
  else if (supergenre)
  {
    VerifieMsg(SuperCirceAndOthers);
    return false;
  }
  else
  {
    Goal const goal = find_unique_goal(si);
    if (goal.type==no_goal)
    {
      VerifieMsg(StipNotSupported);
      return false;
    }
    else
    {
      republican_goal = goal;
      OptFlag[sansrn] = true;
      OptFlag[sansrb] = true;
      optim_neutralretractable = false;
      add_ortho_mating_moves_generation_obstacle();
      is_republican_suspended = false;
      jouegenre = true;
      supergenre = true;
      return true;
    }
  }
}

/* Write the Republican Chess diagram caption
 * @param CondLine string containing the condition caption
 * @param lineLength size of array CondLine
 */
void republican_write_diagram_caption(char CondLine[], size_t lineLength)
{
  strncat(CondLine, "    ", lineLength);
  if (RepublicanType==republican_type1)
    strncat(CondLine, VariantTypeString[UserLanguage][Type1], lineLength);
  if (RepublicanType==republican_type2)
    strncat(CondLine, VariantTypeString[UserLanguage][Type2], lineLength);
}

/* Place the opposite king as part of playing a move
 * @param jt are we playing the move for the first time?
 * @param moving side at the move
 * @param ply_id id of ply in which the move is played
 */
void republican_place_king(joue_type jt, Side moving, ply ply_id)
{

  if (jt==replay)
  {
    if (republican_king_placement[ply_id]!=initsquare)
    {
      Side const not_moving = advers(moving);
      piece const king_type = not_moving==White ? roib : roin;
      king_square[not_moving] = republican_king_placement[ply_id];
      e[king_square[not_moving]] = king_type;
      ++nbpiece[king_type];
    }
  }
  else if (is_republican_suspended)
  {
    republican_king_placement[ply_id] = initsquare;
    super[ply_id] = square_h8+1;
  }
  else
  {
    is_republican_suspended = true;
    find_mate_square(moving);
    republican_king_placement[ply_id] = (super[ply_id]<=square_h8
                                         ? super[ply_id]
                                         : initsquare);
    if (RepublicanType==republican_type1)
    {
      /* In type 1, Republican chess is suspended (and hence
       * play is over) once a king is inserted. */
      if (republican_king_placement[ply_id]==initsquare)
        is_republican_suspended = false;
    }
    else
      /* In type 2, on the other hand, Republican chess is
       * continued, and the side just "mated" can attempt to
       * defend against the mate by inserting the opposite
       * king. */
      is_republican_suspended = false;
  }
}

/* Unplace the opposite king as part of taking back a move
 */
void republican_unplace_king(void)
{
  square const sq = republican_king_placement[nbply];
  if (sq!=initsquare)
  {
    e[sq] = vide;
    if (sq==king_square[Black])
    {
      king_square[Black] = initsquare;
      nbpiece[roin]--;
    }
    if (sq==king_square[White])
    {
      king_square[White] = initsquare;
      nbpiece[roib]--;
    }

    if (RepublicanType==republican_type1)
      /* Republican chess was suspended when the move was played. */
      is_republican_suspended = false;
  }
}

/* Advance the square where to place the opposite king as part of
 * taking back a move
 */
boolean republican_advance_king_square(void)
{
  boolean const result = super[nbply]<=square_h8;
  if (!result)
    super[nbply] = superbas;
  return result;
}

/* Save the Republican Chess part of the current move in a play
 * @param ply_id identifies ply of move to be saved
 * @param mov address of structure where to save the move
 */
void republican_current(ply ply_id, coup *move)
{
  move->repub_k = republican_king_placement[ply_id];
}

/* Compare the Republican Chess parts of two saved moves
 * @param move1 address of 1st saved move
 * @param move2 address of 2nd saved move
 * @return true iff the Republican Chess parts are equal
 */
boolean republican_moves_equal(coup const *move1, coup const *move2)
{
  return move1->repub_k==move2->repub_k;
}

/* Write how the opposite king is placed as part of a move
 * @param move address of move being written
 */
void write_republican_king_placement(coup const *mov)
{
  if (mov->repub_k<=square_h8 && mov->repub_k>=square_a1)
  {
    Flags ren_spec = mov->ren_spec;
    SETFLAG(ren_spec,advers(mov->tr));
    StdString("[+");
    WriteSpec(ren_spec, true);
    WritePiece(roib);
    WriteSquare(mov->repub_k);
    StdChar(']');
  }
}
