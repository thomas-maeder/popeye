/********************* MODIFICATIONS to pyproof.c ***********************
 **
 ** Date       Who  What
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Change for take&make
 **
 ** 2007/05/14 SE   Change for annan
 **
 ** 2008/01/01 SE   Bug fix: Circe Assassin + proof game (reported P.Raican)
 **
 ** 2008/01/01 SE   Bug fix: Circe Parrain + proof game (reported P.Raican)
 **
 **************************** End of List ******************************/

#if defined(__TURBOC__)
# include <mem.h>
#endif
#include "solving/proofgames.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "solving/observation.h"
#include "platform/maxtime.h"
#include "position/position.h"
#include "pieces/pieces.h"
#include "pieces/walks/walks.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

boolean change_moving_piece;

position proofgames_start_position;
position proofgames_target_position;

/* an array to store the position */
static struct
{
    piece_walk_type type;
    Flags spec;
    square pos;
} target_pieces[nr_squares_on_board];

static unsigned int ProofNbrAllPieces;

static void override_standard_walk(square s, Side side, piece_walk_type orthodox_walk)
{
  piece_walk_type const overriding_walk = standard_walks[orthodox_walk];

  --proofgames_start_position.number_of_pieces[side][proofgames_start_position.board[s]];
  proofgames_start_position.board[s] = overriding_walk;
  ++proofgames_start_position.number_of_pieces[side][overriding_walk];
}

void ProofSaveStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  proofgames_start_position = being_solved;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved = proofgames_start_position;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreTargetPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved = proofgames_target_position;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean compareProofPieces(void)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<ProofNbrAllPieces; ++i)
    if (target_pieces[i].type!=get_walk_of_piece_on_square(target_pieces[i].pos)
        || target_pieces[i].spec!=(being_solved.spec[target_pieces[i].pos]&PieSpMask))
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean compareProofNbrPiece(void)
{
  boolean result = true;
  piece_walk_type const last_piece = piece_walk_may_exist_fairy ? nr_piece_walks-1 : Bishop;
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = King; p<=last_piece; ++p)
    if (proofgames_target_position.number_of_pieces[White][p]!=being_solved.number_of_pieces[White][p]
        || proofgames_target_position.number_of_pieces[Black][p]!=being_solved.number_of_pieces[Black][p])
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean compareImitators(void)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<being_solved.number_of_imitators; ++imi_idx)
      if (proofgames_target_position.isquare[imi_idx]!=being_solved.isquare[imi_idx])
      {
        result = false;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* function that compares the current position with the desired one
 * and returns true if they are identical. Otherwise it returns false.
 */
boolean ProofIdentical(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = compareProofPieces() && compareProofNbrPiece() && compareImitators();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void initialise_target_pieces_cache(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ProofNbrAllPieces = 0;

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const square_i = boardnum[i];
    piece_walk_type const p = proofgames_target_position.board[square_i];
    if (p!=Empty && p!=Invalid)
    {
      target_pieces[ProofNbrAllPieces].type = p;
      target_pieces[ProofNbrAllPieces].spec = proofgames_target_position.spec[square_i]&PieSpMask;
      target_pieces[ProofNbrAllPieces].pos = square_i;
      ++ProofNbrAllPieces;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Take a snapshot of the current (target) position
 * @param reason reason for taking the snapshot
 */
static void move_effect_journal_do_snapshot_proofgame_target_position(move_effect_reason_type reason)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_snapshot_proofgame_target_position,reason);
  proofgames_target_position = being_solved;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Restore the current position to the target position of a proofgame or A=>B stipulation
 * @param entry address of move effect journal entry that represents taking the
 *              restored snapshot
 */
static void move_effect_journal_undo_snapshot_proofgame_target_position(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved = proofgames_target_position;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_do_atob_reset_position_for_target(move_effect_reason_type reason)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_atob_reset_position_for_target,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  ProofSaveStartPosition();

  {
    int i;
    for (i = 0; i<nr_squares_on_board; i++)
      empty_square(boardnum[i]);
    for (i = 0; i<maxinum; i++)
      being_solved.isquare[i] = initsquare;
  }

  entry->u.reset_position.currPieceId = being_solved.currPieceId;
  being_solved.currPieceId = NullPieceId;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_atob_reset_position_for_target(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved.currPieceId = entry->u.reset_position.currPieceId;

  ProofRestoreStartPosition();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void proof_initialise_start_position(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  proofgames_start_position = game_array;

  override_standard_walk(square_e1,White,King);
  override_standard_walk(square_d1,White,Queen);
  override_standard_walk(square_a1,White,Rook);
  override_standard_walk(square_h1,White,Rook);
  override_standard_walk(square_c1,White,Bishop);
  override_standard_walk(square_f1,White,Bishop);
  override_standard_walk(square_b1,White,Knight);
  override_standard_walk(square_g1,White,Knight);
  override_standard_walk(square_a2,White,Pawn);
  override_standard_walk(square_b2,White,Pawn);
  override_standard_walk(square_c2,White,Pawn);
  override_standard_walk(square_d2,White,Pawn);
  override_standard_walk(square_e2,White,Pawn);
  override_standard_walk(square_f2,White,Pawn);
  override_standard_walk(square_g2,White,Pawn);
  override_standard_walk(square_h2,White,Pawn);

  override_standard_walk(square_e8,Black,King);
  override_standard_walk(square_d8,Black,Queen);
  override_standard_walk(square_a8,Black,Rook);
  override_standard_walk(square_h8,Black,Rook);
  override_standard_walk(square_c8,Black,Bishop);
  override_standard_walk(square_f8,Black,Bishop);
  override_standard_walk(square_b8,Black,Knight);
  override_standard_walk(square_g8,Black,Knight);
  override_standard_walk(square_a7,Black,Pawn);
  override_standard_walk(square_b7,Black,Pawn);
  override_standard_walk(square_c7,Black,Pawn);
  override_standard_walk(square_d7,Black,Pawn);
  override_standard_walk(square_e7,Black,Pawn);
  override_standard_walk(square_f7,Black,Pawn);
  override_standard_walk(square_g7,Black,Pawn);
  override_standard_walk(square_h7,Black,Pawn);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* solve a proofgame stipulation
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void proof_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we don't redo saving the target position */
  move_effect_journal_set_effect_doers(move_effect_snapshot_proofgame_target_position,
                                       &move_effect_journal_undo_snapshot_proofgame_target_position,
                                       0);
  move_effect_journal_do_snapshot_proofgame_target_position(move_effect_reason_diagram_setup);

  being_solved = proofgames_start_position;

  initialise_target_pieces_cache();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* verify that the proof or A=>B goal is unique
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void proof_verify_unique_goal_solve(slice_index si)
{
  if (find_unique_goal(si).type==no_goal)
    output_plaintext_verifie_message(MultipleGoalsWithProofGameNotAcceptable);
  else
    pipe_solve_delegate(si);
}

/* Instrument the input machinery with a proof games type
 * @param start start slice of input machinery
 */
void input_instrument_proof(slice_index start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  if (input_is_instrumented_with_proof(start))
    output_plaintext_input_error_message(InconsistentProofTarget);
  else
  {
    slice_index const prototypes[] = {
        alloc_pipe(STProofSolver),
        alloc_pipe(STPiecesCounter),
        alloc_pipe(STRoyalsLocator),
        alloc_pipe(STPiecesFlagsInitialiser)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(start,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void report_instrumented(slice_index si, stip_structure_traversal *st)
{
  boolean * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Has the input branch already been instrumend with a proofgame solver?
 * @param start entry into input branch
 * @return true iff input branch has already been instrumend
 */
boolean input_is_instrumented_with_proof(slice_index start)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,STProofSolverBuilder,&report_instrumented);
  stip_structure_traversal_override_single(&st,STAToBSolverBuilder,&report_instrumented);
  stip_structure_traversal_override_single(&st,STStartOfCurrentTwin,&stip_structure_visitor_noop);
  stip_traverse_structure(start,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
