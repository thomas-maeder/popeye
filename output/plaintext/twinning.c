#include "output/plaintext/twinning.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/stipulation.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "debugging/assert.h"

#include <stdio.h>
#include <string.h>

static move_effect_journal_index_type last_horizon;

static boolean find_removal(move_effect_journal_index_type top,
                            square on)
{
  move_effect_journal_index_type const base = twin_is_continued ? last_horizon : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;
  for (curr = base; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_removal
        && move_effect_journal[curr].u.piece_addition.on==on)
      return true;

  return false;
}

static boolean find_creation(move_effect_journal_index_type curr,
                             square on)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  for (; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_creation
        && move_effect_journal[curr].u.piece_addition.on==on)
      return true;

  return false;
}

static void WriteCondition(FILE *file, char const CondLine[], boolean is_first)
{
  if (is_first)
    fputs(CondLine,file);
  else
    fprintf(file,"\n   %s",CondLine);
}

static void WritePieceCreation(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_removal(curr,entry->u.piece_addition.on))
    fputc('+',file);

  WriteSpec(file,
            entry->u.piece_addition.flags,
            entry->u.piece_addition.walk,
            true);
  WritePiece(file,entry->u.piece_addition.walk);
  WriteSquare(file,entry->u.piece_addition.on);
  fputs("  ",file);
}

static void WritePieceRemoval(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_creation(curr+1,entry->u.piece_removal.on))
  {
    fputs("-",file);
    WriteSpec(file,
              entry->u.piece_removal.flags,
              entry->u.piece_removal.walk,
              true);
    WritePiece(file,entry->u.piece_removal.walk);
    WriteSquare(file,entry->u.piece_removal.on);
    fputs("  ",file);
  }
}

static void WritePieceMovement(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(file,
            entry->u.piece_movement.movingspec,
            entry->u.piece_movement.moving,
            true);
  WritePiece(file,entry->u.piece_movement.moving);
  WriteSquare(file,entry->u.piece_movement.from);
  fputs("-->",file);
  WriteSquare(file,entry->u.piece_movement.to);
  fputs("  ",file);
}

static void WritePieceExchange(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(file,
            entry->u.piece_exchange.fromflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.to),
            true);
  WritePiece(file,get_walk_of_piece_on_square(entry->u.piece_exchange.to));
  WriteSquare(file,entry->u.piece_exchange.from);
  fputs("<-->",file);
  WriteSpec(file,
            entry->u.piece_exchange.toflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.from),
            true);
  WritePiece(file,get_walk_of_piece_on_square(entry->u.piece_exchange.from));
  WriteSquare(file,entry->u.piece_exchange.to);
  fputs("  ",file);
}

static void WriteBoardTransformation(FILE *file,
                                     move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      fputs(TwinningTab[TwinningRotate],file);
      fputs(" 90",file);
      break;
    case rot180:
      fputs(TwinningTab[TwinningRotate],file);
      fputs(" 180",file);
      break;
    case rot270:
      fputs(TwinningTab[TwinningRotate],file);
      fputs(" 270",file);
      break;
    case mirra1h1:
      fputs(TwinningTab[TwinningMirror],file);
      fputs(" ",file);
      fputs(TwinningMirrorTab[TwinningMirrora1h1],file);
      break;
    case mirra1a8:
      fputs(TwinningTab[TwinningMirror],file);
      fputs(" ",file);
      fputs(TwinningMirrorTab[TwinningMirrora1a8],file);
      break;
    case mirra1h8:
      fputs(TwinningTab[TwinningMirror],file);
      fputs(" ",file);
      fputs(TwinningMirrorTab[TwinningMirrora1h8],file);
      break;
    case mirra8h1:
      fputs(TwinningTab[TwinningMirror],file);
      fputs(" ",file);
      fputs(TwinningMirrorTab[TwinningMirrora8h1],file);
      break;

    default:
      assert(0);
      break;
  }

  fputs("  ",file);
}

static void WriteShift(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  fputs(TwinningTab[TwinningShift],file);
  fputs(" ",file);
  WriteSquare(file,entry->u.twinning_shift.from);
  fputs(" ==> ",file);
  WriteSquare(file,entry->u.twinning_shift.to);
  fputs("  ",file);
}

static void WriteStipulation(FILE *file, move_effect_journal_index_type curr)
{
  fputs(AlphaStip,file);
  fputs("  ",file);
}

static void WritePolish(FILE *file, move_effect_journal_index_type curr)
{
  fputs(TwinningTab[TwinningPolish],file);
  fputs("  ",file);
}

static void WriteSubstitute(FILE *file, move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WritePiece(file,entry->u.piece_change.from);
  fputs(" ==> ",file);
  WritePiece(file,entry->u.piece_change.to);
  fputs("  ",file);
}

static void WriteTwinLetter(FILE *file)
{
  if (twin_is_continued)
    fputc('+',file);

  if (twin_number-twin_a<='z'-'a')
    fprintf(file,"%c) ", 'a'+twin_number-twin_a);
  else
    fprintf(file,"z%u) ", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void WriteTwinning(FILE *file)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  move_effect_journal_index_type const base = twin_is_continued ? last_horizon : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;

  assert(base<=top);

  for (curr = base; curr!=top; ++curr)
  {
    move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

    switch (entry->type)
    {
      case move_effect_piece_creation:
        WritePieceCreation(file,curr);
        break;

      case move_effect_piece_removal:
        WritePieceRemoval(file,curr);
        break;

      case move_effect_piece_movement:
        WritePieceMovement(file,curr);
        break;

      case move_effect_piece_exchange:
        WritePieceExchange(file,curr);
        break;

      case move_effect_board_transformation:
        WriteBoardTransformation(file,curr);
        break;

      case move_effect_twinning_shift:
        WriteShift(file,curr);
        break;

      case move_effect_input_condition:
        WriteConditions(file,&WriteCondition);
        fputs("  ",file);
        break;

      case move_effect_input_stipulation:
      case move_effect_input_sstipulation:
        WriteStipulation(file,curr);
        break;

      case move_effect_twinning_polish:
        WritePolish(file,curr);
        break;

      case move_effect_twinning_substitute:
        WriteSubstitute(file,curr);
        break;

      case move_effect_king_square_movement:
        /* the search for royals leaves its traces in the twinning ply */
      case move_effect_remember_volcanic:
        /* Forsberg twinning */
        break;

      default:
        assert(0);
        break;
    }
  }

  last_horizon = top;
}

static void WriteIntro(FILE *file)
{
  switch (twin_stage)
  {
    case twin_original_position_no_twins:
      break;

    case twin_zeroposition:
      Message2(file,NewLine);
      fputs(TokenTab[ZeroPosition],file);
      Message2(file,NewLine);
      Message2(file,NewLine);
      break;

    case twin_initial:
      Message2(file,NewLine);
      WriteTwinLetter(file);
      WriteTwinning(file);
      Message2(file,NewLine);
      break;

    case twin_regular:
    case twin_last:
      WriteTwinLetter(file);
      WriteTwinning(file);
      Message2(file,NewLine);
      break;

    default:
      assert(0);
      break;
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
void output_plaintext_write_twinning(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TraceFile)
    WriteIntro(TraceFile);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
