#include "output/plaintext/twinning.h"
#include "output/plaintext/protocol.h"
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

/* remember how far we written the twinning
 * for a continued twin, this is where we start writing
 */
static move_effect_journal_index_type water_line;

static boolean find_removal(move_effect_journal_index_type top,
                            square on)
{
  move_effect_journal_index_type const base = twin_is_continued ? water_line : move_effect_journal_base[ply_twinning];
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

static void WriteCondition(FILE *file, char const CondLine[], condition_rank rank)
{
  switch (rank)
  {
    case condition_first:
      protocol_printf("%s",CondLine);
      break;

    case condition_subsequent:
      protocol_printf("\n   %s",CondLine);
      break;

    case condition_end:
      break;
  }
}

static void WritePieceCreation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_removal(curr,entry->u.piece_addition.on))
    protocol_putchar('+');

  WriteSpec1(entry->u.piece_addition.flags,
             entry->u.piece_addition.walk,
             true);
  WritePiece1(entry->u.piece_addition.walk);
  WriteSquare1(entry->u.piece_addition.on);
  protocol_printf("%s","  ");
}

static void WritePieceRemoval(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_creation(curr+1,entry->u.piece_removal.on))
  {
    protocol_printf("%s","-");
    WriteSpec1(entry->u.piece_removal.flags,
               entry->u.piece_removal.walk,
               true);
    WritePiece1(entry->u.piece_removal.walk);
    WriteSquare1(entry->u.piece_removal.on);
    protocol_printf("%s","  ");
  }
}

static void WritePieceMovement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec1(entry->u.piece_movement.movingspec,
             entry->u.piece_movement.moving,
             true);
  WritePiece1(entry->u.piece_movement.moving);
  WriteSquare1(entry->u.piece_movement.from);
  protocol_printf("%s","-->");
  WriteSquare1(entry->u.piece_movement.to);
  protocol_printf("%s","  ");
}

static void WritePieceExchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec1(entry->u.piece_exchange.fromflags,
             get_walk_of_piece_on_square(entry->u.piece_exchange.to),
             true);
  WritePiece1(get_walk_of_piece_on_square(entry->u.piece_exchange.to));
  WriteSquare1(entry->u.piece_exchange.from);
  protocol_printf("%s","<-->");
  WriteSpec1(entry->u.piece_exchange.toflags,
             get_walk_of_piece_on_square(entry->u.piece_exchange.from),
             true);
  WritePiece1(get_walk_of_piece_on_square(entry->u.piece_exchange.from));
  WriteSquare1(entry->u.piece_exchange.to);
  protocol_printf("%s","  ");
}

static void WriteBoardTransformation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      protocol_printf("%s",TwinningTab[TwinningRotate]);
      protocol_printf("%s"," 90");
      break;
    case rot180:
      protocol_printf("%s",TwinningTab[TwinningRotate]);
      protocol_printf("%s"," 180");
      break;
    case rot270:
      protocol_printf("%s",TwinningTab[TwinningRotate]);
      protocol_printf("%s"," 270");
      break;
    case mirra1h1:
      protocol_printf("%s",TwinningTab[TwinningMirror]);
      protocol_printf("%s"," ");
      protocol_printf("%s",TwinningMirrorTab[TwinningMirrora1h1]);
      break;
    case mirra1a8:
      protocol_printf("%s",TwinningTab[TwinningMirror]);
      protocol_printf("%s"," ");
      protocol_printf("%s",TwinningMirrorTab[TwinningMirrora1a8]);
      break;
    case mirra1h8:
      protocol_printf("%s",TwinningTab[TwinningMirror]);
      protocol_printf("%s"," ");
      protocol_printf("%s",TwinningMirrorTab[TwinningMirrora1h8]);
      break;
    case mirra8h1:
      protocol_printf("%s",TwinningTab[TwinningMirror]);
      protocol_printf("%s"," ");
      protocol_printf("%s",TwinningMirrorTab[TwinningMirrora8h1]);
      break;

    default:
      assert(0);
      break;
  }

  protocol_printf("%s","  ");
}

static void WriteShift(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  protocol_printf("%s",TwinningTab[TwinningShift]);
  protocol_printf("%s"," ");
  WriteSquare1(entry->u.twinning_shift.from);
  protocol_printf("%s"," ==> ");
  WriteSquare1(entry->u.twinning_shift.to);
  protocol_printf("%s","  ");
}

static void WriteStipulation(move_effect_journal_index_type curr)
{
  protocol_printf("%s",AlphaStip);
  protocol_printf("%s","  ");
}

static void WritePolish(move_effect_journal_index_type curr)
{
  protocol_printf("%s",TwinningTab[TwinningPolish]);
  protocol_printf("%s","  ");
}

static void WriteSubstitute(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WritePiece1(entry->u.piece_change.from);
  protocol_printf("%s"," ==> ");
  WritePiece1(entry->u.piece_change.to);
  protocol_printf("%s","  ");
}

static void WriteTwinLetter(void)
{
  if (twin_is_continued)
    protocol_putchar('+');

  if (twin_number-twin_a<='z'-'a')
    protocol_printf("%c) ", 'a'+twin_number-twin_a);
  else
    protocol_printf("z%u) ", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void WriteTwinning(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  move_effect_journal_index_type const base = twin_is_continued ? water_line : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;

  assert(base<=top);

  for (curr = base; curr!=top; ++curr)
  {
    move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

    switch (entry->type)
    {
      case move_effect_piece_creation:
        WritePieceCreation(curr);
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
        protocol_printf("%s","  ");
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

      default:
        assert(0);
        break;
    }
  }
}

static void WriteIntro(void)
{
  switch (twin_stage)
  {
    case twin_original_position_no_twins:
      break;

    case twin_zeroposition:
      Message(NewLine);
      protocol_printf("%s",TokenTab[ZeroPosition]);
      Message(NewLine);
      Message(NewLine);
      break;

    case twin_initial:
      Message(NewLine);
      WriteTwinLetter();
      WriteTwinning();
      Message(NewLine);
      break;

    case twin_regular:
    case twin_last:
      WriteTwinLetter();
      WriteTwinning();
      Message(NewLine);
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

  WriteIntro();

  water_line = move_effect_journal_base[ply_twinning+1];

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
