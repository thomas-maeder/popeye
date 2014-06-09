#include "output/plaintext/twinning.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/stipulation.h"
#include "solving/move_effect_journal.h"
#include "debugging/assert.h"

#include <stdio.h>
#include <string.h>

static move_effect_journal_index_type last_horizon;

void WriteTwinNumber(unsigned int TwinNumber)
{
  if (TwinNumber-1<='z'-'a')
    sprintf(GlobalStr, "%c) ", 'a'+TwinNumber-1);
  else
    sprintf(GlobalStr, "z%u) ", (unsigned int)(TwinNumber-1-('z'-'a')));

  StdString(GlobalStr);
}

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

static void WriteCondition(char const CondLine[], boolean is_first)
{
  if (is_first)
    StdString(CondLine);
  else
  {
    StdString("\n   ");
    StdString(CondLine);
  }
}

static void WritePieceCreation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_removal(curr,entry->u.piece_addition.on))
    StdChar('+');

  WriteSpec(entry->u.piece_addition.flags,
            entry->u.piece_addition.walk,
            true);
  WritePiece(entry->u.piece_addition.walk);
  WriteSquare(entry->u.piece_addition.on);
  StdString("  ");
}

static void WritePieceRemoval(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  if (!find_creation(curr+1,entry->u.piece_removal.on))
  {
    StdString("-");
    WriteSpec(entry->u.piece_removal.flags,
              entry->u.piece_removal.walk,
              true);
    WritePiece(entry->u.piece_removal.walk);
    WriteSquare(entry->u.piece_removal.on);
    StdString("  ");
  }
}

static void WritePieceMovement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(entry->u.piece_movement.movingspec,
            entry->u.piece_movement.moving,
            true);
  WritePiece(entry->u.piece_movement.moving);
  WriteSquare(entry->u.piece_movement.from);
  StdString("-->");
  WriteSquare(entry->u.piece_movement.to);
  StdString("  ");
}

static void WritePieceExchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WriteSpec(entry->u.piece_exchange.fromflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.to),
            true);
  WritePiece(get_walk_of_piece_on_square(entry->u.piece_exchange.to));
  WriteSquare(entry->u.piece_exchange.from);
  StdString("<-->");
  WriteSpec(entry->u.piece_exchange.toflags,
            get_walk_of_piece_on_square(entry->u.piece_exchange.from),
            true);
  WritePiece(get_walk_of_piece_on_square(entry->u.piece_exchange.from));
  WriteSquare(entry->u.piece_exchange.to);
  StdString("  ");
}

static void WriteBoardTransformation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      StdString(TwinningTab[TwinningRotate]);
      StdString(" 90");
      break;
    case rot180:
      StdString(TwinningTab[TwinningRotate]);
      StdString(" 180");
      break;
    case rot270:
      StdString(TwinningTab[TwinningRotate]);
      StdString(" 270");
      break;
    case mirra1h1:
      StdString(TwinningTab[TwinningMirror]);
      StdString(" ");
      StdString(TwinningMirrorTab[TwinningMirrora1h1]);
      break;
    case mirra1a8:
      StdString(TwinningTab[TwinningMirror]);
      StdString(" ");
      StdString(TwinningMirrorTab[TwinningMirrora1a8]);
      break;
    case mirra1h8:
      StdString(TwinningTab[TwinningMirror]);
      StdString(" ");
      StdString(TwinningMirrorTab[TwinningMirrora1h8]);
      break;
    case mirra8h1:
      StdString(TwinningTab[TwinningMirror]);
      StdString(" ");
      StdString(TwinningMirrorTab[TwinningMirrora8h1]);
      break;

    default:
      assert(0);
      break;
  }

  StdString("  ");
}

static void WriteShift(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  StdString(TwinningTab[TwinningShift]);
  StdString(" ");
  WriteSquare(entry->u.twinning_shift.from);
  StdString(" ==> ");
  WriteSquare(entry->u.twinning_shift.to);
  StdString("  ");
}

static void WriteStipulation(move_effect_journal_index_type curr)
{
  StdString(AlphaStip);
  StdString("  ");
}

static void WritePolish(move_effect_journal_index_type curr)
{
  StdString(TwinningTab[TwinningPolish]);
  StdString("  ");
}

static void WriteSubstitute(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  WritePiece(entry->u.piece_change.from);
  StdString(" ==> ");
  WritePiece(entry->u.piece_change.to);
  StdString("  ");
}

void WriteTwinning(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  move_effect_journal_index_type const base = twin_is_continued ? last_horizon : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;

  assert(base<=top);

  if (twin_is_continued)
    StdChar('+');

  WriteTwinNumber(TwinNumber);

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
        WriteConditions(&WriteCondition);
        StdString("  ");
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

  Message(NewLine);

  last_horizon = top;
}
