#include "conditions/republican.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pylang.h"
#include "stipulation/has_solution_type.h"
#include "solving/battle_play/attack_play.h"
#include "pymsg.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"

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
 * @param moving side at the move
 */
static void republican_place_king_first_play(Side moving)
{

  if (is_republican_suspended)
  {
    republican_king_placement[nbply] = initsquare;
    super[nbply] = square_h8+1;
  }
  else
  {
    is_republican_suspended = true;
    find_mate_square(moving);
    republican_king_placement[nbply] = (super[nbply]<=square_h8
                                        ? super[nbply]
                                        : initsquare);
    if (RepublicanType==republican_type1)
    {
      /* In type 1, Republican chess is suspended (and hence
       * play is over) once a king is inserted. */
      if (republican_king_placement[nbply]==initsquare)
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

/* Place the opposite king as part of playing a move
 * @param moving side at the move
 */
static void republican_place_king_replay(Side moving)
{
  if (republican_king_placement[nbply]!=initsquare)
  {
    Side const not_moving = advers(moving);
    piece const king_type = not_moving==White ? roib : roin;
    king_square[not_moving] = republican_king_placement[nbply];
    e[king_square[not_moving]] = king_type;
    ++nbpiece[king_type];
  }
}

/* Unplace the opposite king as part of taking back a move
 */
static void republican_unplace_king(void)
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

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STRepublicanKingPlacer);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_replay(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STRepublicanKingPlacerReplay);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_republican_king_placers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move_replay);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type republican_king_placer_attack(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  republican_place_king_first_play(slices[si].starter);
  result = attack(slices[si].next1,n);
  republican_unplace_king();

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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type republican_king_placer_defend(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  republican_place_king_first_play(slices[si].starter);
  result = defend(slices[si].next1,n);
  republican_unplace_king();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type republican_king_placer_replay_attack(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  republican_place_king_replay(slices[si].starter);
  result = attack(slices[si].next1,n);
  republican_unplace_king();

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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type republican_king_placer_replay_defend(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  republican_place_king_replay(slices[si].starter);
  result = defend(slices[si].next1,n);
  republican_unplace_king();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
