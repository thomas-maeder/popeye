#include "conditions/singlebox/type2.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "conditions/singlebox/type1.h"
#include "debugging/trace.h"

#include <assert.h>

/* Does the current position contain an illegal latent white pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_white_pawn(void)
{
  boolean result = false;
  square const next_latent_white = next_latent_pawn(initsquare,White);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (next_latent_white!=initsquare)
  {
    piece p;
    for (p = db; p<=fb; ++p)
      if (nbpiece[p]<nr_piece(game_array)[p])
      {
        result = true;
        break;
      }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Does the current position contain an illegal latent black pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_black_pawn(void)
{
  boolean result = false;
  square const next_latent_black = next_latent_pawn(initsquare,Black);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (next_latent_black!=initsquare)
  {
    piece p;
    for (p = dn; p>=fn; --p)
      if (nbpiece[p]<nr_piece(game_array)[p])
      {
        result = true;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_last_move_illegal(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (singlebox_type1_illegal())
    result = true;
  else if (singlebox_illegal_latent_white_pawn()
           || singlebox_illegal_latent_black_pawn())
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STSingleBoxType2LegalityTester);
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
void stip_insert_singlebox_type2(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
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
stip_length_type singlebox_type2_legality_tester_attack(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = n+2;
  else
    result = attack(next,n);

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
stip_length_type singlebox_type2_legality_tester_defend(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = slack_length-1;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
