#include "output/plaintext/twinning.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/plaintext.h"
#include "input/plaintext/stipulation.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "solving/duplex.h"
#include "stipulation/pipe.h"
#include "debugging/assert.h"

#include <stdio.h>
#include <string.h>

/* remember how far we written the twinning
 * for a continued twin, this is where we start writing
 */
static move_effect_journal_index_type water_line;

static boolean find_removal(move_effect_journal_index_type base,
                            move_effect_journal_index_type top,
                            square on)
{
  move_effect_journal_index_type curr;
  for (curr = base; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_removal
        && move_effect_journal[curr].u.piece_addition.added.on==on)
      return true;

  return false;
}

static boolean find_creation(move_effect_journal_index_type curr,
                             square on)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  for (; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_creation
        && move_effect_journal[curr].u.piece_addition.added.on==on)
      return true;

  return false;
}

static void WriteCondition(FILE *file, char const CondLine[], condition_rank rank)
{
  switch (rank)
  {
    case condition_first:
      protocol_fprintf(stdout,"%s",CondLine);
      break;

    case condition_subsequent:
      protocol_fprintf(stdout,"\n   %s",CondLine);
      break;

    case condition_end:
      break;
  }
}

static void WritePieceCreation(move_effect_journal_index_type base,
                               move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_removal(base,curr,entry->u.piece_addition.added.on))
    protocol_fputc('+',stdout);

  WriteSpec(&output_plaintext_engine,
            stdout,
            entry->u.piece_addition.added.flags,
            entry->u.piece_addition.added.walk,
            true);
  WriteWalk(&output_plaintext_engine,stdout,entry->u.piece_addition.added.walk);
  WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_addition.added.on);
  protocol_fprintf(stdout,"%s","  ");
}

static void WritePieceRemoval(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_creation(curr+1,entry->u.piece_removal.on))
  {
    protocol_fprintf(stdout,"%s","-");
    WriteSpec(&output_plaintext_engine,
              stdout,
              entry->u.piece_removal.flags,
              entry->u.piece_removal.walk,
              true);
    WriteWalk(&output_plaintext_engine,stdout,entry->u.piece_removal.walk);
    WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_removal.on);
    protocol_fprintf(stdout,"%s","  ");
  }
}

static void WritePieceMovement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(&output_plaintext_engine,
            stdout,
            entry->u.piece_movement.movingspec,
            entry->u.piece_movement.moving,
            true);
  WriteWalk(&output_plaintext_engine,stdout,entry->u.piece_movement.moving);
  WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_movement.from);
  protocol_fprintf(stdout,"%s","-->");
  WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_movement.to);
  protocol_fprintf(stdout,"%s","  ");
}

static void WritePieceExchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(&output_plaintext_engine,
            stdout,
            entry->u.piece_exchange.fromflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.to),
            true);
  WriteWalk(&output_plaintext_engine,stdout,get_walk_of_piece_on_square(entry->u.piece_exchange.to));
  WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_exchange.from);
  protocol_fprintf(stdout,"%s","<-->");
  WriteSpec(&output_plaintext_engine,
            stdout,
            entry->u.piece_exchange.toflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.from),
            true);
  WriteWalk(&output_plaintext_engine,stdout,get_walk_of_piece_on_square(entry->u.piece_exchange.from));
  WriteSquare(&output_plaintext_engine,stdout,entry->u.piece_exchange.to);
  protocol_fprintf(stdout,"%s","  ");
}

static void WriteBoardTransformation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningRotate]);
      protocol_fprintf(stdout,"%s"," 90");
      break;
    case rot180:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningRotate]);
      protocol_fprintf(stdout,"%s"," 180");
      break;
    case rot270:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningRotate]);
      protocol_fprintf(stdout,"%s"," 270");
      break;
    case mirra1h1:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningMirror]);
      protocol_fprintf(stdout,"%s"," ");
      protocol_fprintf(stdout,"%s",TwinningMirrorTab[TwinningMirrora1h1]);
      break;
    case mirra1a8:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningMirror]);
      protocol_fprintf(stdout,"%s"," ");
      protocol_fprintf(stdout,"%s",TwinningMirrorTab[TwinningMirrora1a8]);
      break;
    case mirra1h8:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningMirror]);
      protocol_fprintf(stdout,"%s"," ");
      protocol_fprintf(stdout,"%s",TwinningMirrorTab[TwinningMirrora1h8]);
      break;
    case mirra8h1:
      protocol_fprintf(stdout,"%s",TwinningTab[TwinningMirror]);
      protocol_fprintf(stdout,"%s"," ");
      protocol_fprintf(stdout,"%s",TwinningMirrorTab[TwinningMirrora8h1]);
      break;

    default:
      assert(0);
      break;
  }

  protocol_fprintf(stdout,"%s","  ");
}

static void WriteShift(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  protocol_fprintf(stdout,"%s",TwinningTab[TwinningShift]);
  protocol_fprintf(stdout,"%s"," ");
  WriteSquare(&output_plaintext_engine,stdout,entry->u.twinning_shift.from);
  protocol_fprintf(stdout,"%s"," ==> ");
  WriteSquare(&output_plaintext_engine,stdout,entry->u.twinning_shift.to);
  protocol_fprintf(stdout,"%s","  ");
}

static void WriteStipulation(move_effect_journal_index_type curr)
{
  protocol_fprintf(stdout,"%s",AlphaStip);
  protocol_fprintf(stdout,"%s","  ");
}

static void WritePolish(move_effect_journal_index_type curr)
{
  protocol_fprintf(stdout,"%s",TwinningTab[TwinningPolish]);
  protocol_fprintf(stdout,"%s","  ");
}

static void WriteSubstitute(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteWalk(&output_plaintext_engine,stdout,entry->u.piece_change.from);
  protocol_fprintf(stdout,"%s"," ==> ");
  WriteWalk(&output_plaintext_engine,stdout,entry->u.piece_change.to);
  protocol_fprintf(stdout,"%s","  ");
}

static void WriteTwinLetter(unsigned int twin_number, boolean continued)
{
  if (continued)
    protocol_fputc('+',stdout);

  if (twin_number-twin_a<='z'-'a')
    protocol_fprintf(stdout,"%c) ", 'a'+twin_number-twin_a);
  else
    protocol_fprintf(stdout,"z%u) ", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void WriteTwinning(boolean continued)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  move_effect_journal_index_type const base = continued ? water_line : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;

  assert(base<=top);

  for (curr = base; curr!=top; ++curr)
  {
    move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

    switch (entry->type)
    {
      case move_effect_piece_creation:
        WritePieceCreation(base,curr);
        break;

      case move_effect_piece_removal:
        WritePieceRemoval(curr);
        break;

      case move_effect_piece_movement:
        WritePieceMovement(curr);
        break;

      case move_effect_piece_exchange:
        WritePieceExchange(curr);
        break;

      case move_effect_board_transformation:
        WriteBoardTransformation(curr);
        break;

      case move_effect_twinning_shift:
        WriteShift(curr);
        break;

      case move_effect_input_condition:
        WriteConditions(0,&WriteCondition);
        protocol_fprintf(stdout,"%s","  ");
        break;

      case move_effect_input_stipulation:
      case move_effect_input_sstipulation:
        WriteStipulation(curr);
        break;

      case move_effect_twinning_polish:
        WritePolish(curr);
        break;

      case move_effect_twinning_substitute:
        WriteSubstitute(curr);
        break;

      case move_effect_king_square_movement:
        /* the search for royals leaves its traces in the twinning ply */
      case move_effect_remember_volcanic:
        /* Forsberg twinning */
        break;
      case move_effect_snapshot_proofgame_target_position:
      case move_effect_atob_reset_position_for_target:
        break;

      default:
        assert(0);
        break;
    }
  }
}

/* Try to solve in solve_nr_remaining half-moves.
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
void output_plaintext_write_zeroposition_intro(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_plaintext_message(NewLine);
  protocol_fprintf(stdout,"%s",EndTwinTokenTab[ZeroPosition]);
  output_plaintext_message(NewLine);
  output_plaintext_message(NewLine);

  water_line = move_effect_journal_base[ply_twinning+1];

  pipe_solve_delegate(si);

  /* only write the twinning once per twin (and not twice in a duplex) */
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
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
void output_plaintext_write_twin_intro(slice_index si)
{
  unsigned int const twin_number = SLICE_U(si).twinning_handler.twin_number;
  boolean const continued = SLICE_U(si).twinning_handler.continued;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_plaintext_message(NewLine);
  WriteTwinLetter(twin_number,continued);
  WriteTwinning(continued);
  output_plaintext_message(NewLine);

  water_line = move_effect_journal_base[ply_twinning+1];

  pipe_solve_delegate(si);

  /* only write the twinning once per twin (and not twice in a duplex) */
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void handle_twinning_event(slice_index si,
                                  twinning_event_type event,
                                  boolean continued)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",event);
  TraceFunctionParam("%u",continued);
  TraceFunctionParamListEnd();

  switch (event)
  {
    case twin_zeroposition:
    {
      slice_index const prototype = alloc_pipe(STOutputPlaintextZeropositionIntroWriter);
      slice_insertion_insert(si,&prototype,1);
      break;
    }

    default:
    {
      slice_index const prototype = alloc_pipe(STOutputPlaintextTwinIntroWriter);
      SLICE_U(prototype).twinning_handler.twin_number = event-twin_regular;
      SLICE_U(prototype).twinning_handler.continued = continued;
      slice_insertion_insert(si,&prototype,1);
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
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
void output_plaintext_twin_intro_writer_builder_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

slice_index output_plaintext_alloc_twin_intro_writer_builder(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextTwinIntroWriterBuilder),
  SLICE_U(result).twinning_event_handler.handler = &handle_twinning_event;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
