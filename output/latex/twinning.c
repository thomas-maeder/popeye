#include "output/latex/twinning.h"
#include "output/latex/latex.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "input/plaintext/stipulation.h"
#include "options/options.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "solving/machinery/twin.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char twinning[1532];

static move_effect_journal_index_type last_horizon;

void LaTeXWriteOptions(void)
{
  if (OptFlag[duplex])
  {
    strcat(twinning, OptTab[duplex]);
    strcat(twinning, "{\\newline}");
  }
  else if (OptFlag[halfduplex])
  {
    strcat(twinning, OptTab[halfduplex]);
    strcat(twinning, "{\\newline}");
  }
  if (OptFlag[quodlibet])
  {
    strcat(twinning, OptTab[quodlibet]);
    strcat(twinning, "{\\newline}");
  }
}

void LaTeXFlushTwinning(FILE *file)
{
  if (twinning[0]!='\0')
  {
    /* remove the last "{\\newline} */
    twinning[strlen(twinning)-10]= '\0';

    WriteUserInputElement(file,"twins",twinning);

    twinning[0] = 0;
  }
}

static void BeginTwinning(unsigned int twin_number)
{
  int const len = strlen(twinning);
  if (twin_number-twin_a<='z'-'a')
    sprintf(twinning+len, "%c) ", 'a'+twin_number-twin_a);
  else
    sprintf(twinning+len, "z%u) ", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void EndTwinning(void)
{
  strcat(twinning,"{\\newline}");
}

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
      strcat(twinning,CondLine);
      break;

    case condition_subsequent:
      strcat(twinning, ", ");
      strcat(twinning,CondLine);
      break;

    case condition_end:
      break;
  }
}

static void WritePieceCreation(move_effect_journal_index_type base,
                               move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int const len = strlen(twinning);

  sprintf(twinning+len,
          "%s\\%c%s %c%c",
          find_removal(base,curr,entry->u.piece_addition.added.on) ? "" : "+",
          is_piece_neutral(entry->u.piece_addition.added.flags) ? 'n' : (TSTFLAG(entry->u.piece_addition.added.flags, White) ? 'w' : 's'),
          LaTeXWalk(entry->u.piece_addition.added.walk),
          'a'-nr_of_slack_files_left_of_board+entry->u.piece_addition.added.on%onerow,
          '1'-nr_of_slack_rows_below_board+entry->u.piece_addition.added.on/onerow);
}

static boolean WritePieceRemoval(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int len = strlen(twinning);

  if (find_creation(curr+1,entry->u.piece_removal.on))
    return false;
  else
  {
    strcat(twinning, " --");
    strcat(twinning,
           is_piece_neutral(entry->u.piece_removal.flags) ? "\\n" : (TSTFLAG(entry->u.piece_removal.flags, White) ? "\\w" : "\\s"));
    strcat(twinning,LaTeXWalk(entry->u.piece_removal.walk));
    len = strlen(twinning);
    sprintf(twinning+len, " %c%c",
            'a'-nr_files_on_board+entry->u.piece_removal.on%onerow,
            '1'-nr_rows_on_board+entry->u.piece_removal.on/onerow);
    return true;
  }
}

static void WritePieceMovement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int len = strlen(twinning);

  sprintf(twinning+len,
          "\\%c%s %c%c",
          is_piece_neutral(entry->u.piece_movement.movingspec) ? 'n' : (TSTFLAG(entry->u.piece_movement.movingspec, White) ? 'w' : 's'),
          LaTeXWalk(entry->u.piece_movement.moving),
          'a'-nr_of_slack_files_left_of_board+entry->u.piece_movement.from%onerow,
          '1'-nr_of_slack_rows_below_board+entry->u.piece_movement.from/onerow);

  strcat(twinning, "{\\ra}");

  len = strlen(twinning);
  sprintf(twinning+len, "%c%c",
          'a'-nr_files_on_board+entry->u.piece_movement.to%onerow,
          '1'-nr_rows_on_board+entry->u.piece_movement.to/onerow);
}

static void WritePieceExchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int len = strlen(twinning);

  sprintf(twinning+len,"\\%c%s %c%c",
          is_piece_neutral(entry->u.piece_exchange.fromflags) ? 'n' : (TSTFLAG(entry->u.piece_exchange.fromflags, White) ? 'w' : 's'),
          LaTeXWalk(get_walk_of_piece_on_square(entry->u.piece_exchange.to)),
          'a'-nr_of_slack_files_left_of_board+entry->u.piece_exchange.from%onerow,
          '1'-nr_of_slack_rows_below_board+entry->u.piece_exchange.from/onerow);

  strcat(twinning, "{\\lra}");

  len = strlen(twinning);
  sprintf(twinning+len, "\\%c%s %c%c",
          is_piece_neutral(entry->u.piece_exchange.toflags) ? 'n' : (TSTFLAG(entry->u.piece_exchange.toflags, White) ? 'w' : 's'),
          LaTeXWalk(get_walk_of_piece_on_square(entry->u.piece_exchange.from)),
          'a'-nr_files_on_board+entry->u.piece_exchange.to%onerow,
          '1'-nr_rows_on_board+entry->u.piece_exchange.to/onerow);
}

static void WriteBoardTransformation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int const len = strlen(twinning);

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      sprintf(twinning+len, "%s $%s^\\circ$", TwinningTab[TwinningRotate], "90");
      break;
    case rot180:
      sprintf(twinning+len, "%s $%s^\\circ$", TwinningTab[TwinningRotate], "180");
      break;
    case rot270:
      sprintf(twinning+len, "%s $%s^\\circ$", TwinningTab[TwinningRotate], "270");
      break;
    case mirra1h1:
      sprintf(twinning+len, "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1h1]);
      break;
    case mirra1a8:
      sprintf(twinning+len, "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1a8]);
      break;
    case mirra1h8:
      sprintf(twinning+len, "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1h8]);
      break;
    case mirra8h1:
      sprintf(twinning+len, "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora8h1]);
      break;

    default:
      assert(0);
      break;
  }
}

static void WriteShift(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int const len = strlen(twinning);

  sprintf(twinning+len, "%s %c%c$\\Rightarrow$%c%c",
          TwinningTab[TwinningShift],
          'a'-nr_files_on_board+entry->u.twinning_shift.from%onerow,
          '1'-nr_rows_on_board+entry->u.twinning_shift.from/onerow,
          'a'-nr_files_on_board+entry->u.twinning_shift.to%onerow,
          '1'-nr_rows_on_board+entry->u.twinning_shift.to/onerow);
}

static void WriteStipulation(move_effect_journal_index_type curr)
{
  strcat(twinning, AlphaStip);
  if (OptFlag[solapparent])
    strcat(twinning, "*");

  if (OptFlag[whitetoplay])
  {
    char temp[10];        /* increased due to buffer overflow */
    sprintf(temp, " %c{\\ra}",tolower(*PieSpTab[White]));
    strcat(twinning, temp);
  }
}

static void WritePolish(move_effect_journal_index_type curr)
{
  strcat(twinning, TwinningTab[TwinningPolish]);
}

static void WriteSubstitute(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];
  int const len = strlen(twinning);

  sprintf(twinning+len,"{\\w%s} $\\Rightarrow$ \\w%s",
          LaTeXWalk(entry->u.piece_change.from),
          LaTeXWalk(entry->u.piece_change.to));
}

static void WriteTwinLetterToSolution(unsigned int twin_number, FILE *file)
{
  if (twin_number-twin_a<='z'-'a')
    fprintf(file, "%c)", 'a'+twin_number-twin_a);
  else
    fprintf(file, "z%u)", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void WriteTwinning(unsigned int twin_number, boolean continued)
{
  move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
  move_effect_journal_index_type const base = continued ? last_horizon : move_effect_journal_base[ply_twinning];
  move_effect_journal_index_type curr;
  boolean written_on_last_entry = false;

  assert(base<=top);

  if (base<top)
  {
    if (continued)
      strcat(twinning, "+");

    BeginTwinning(twin_number);

    for (curr = base; curr!=top; ++curr)
    {
      if (written_on_last_entry)
      {
        strcat(twinning, ", ");
        written_on_last_entry = false;
      }

      switch (move_effect_journal[curr].type)
      {
        case move_effect_piece_creation:
          WritePieceCreation(base,curr);
          written_on_last_entry = true;
          break;

        case move_effect_piece_removal:
          if (WritePieceRemoval(curr))
            written_on_last_entry = true;
          break;

        case move_effect_piece_movement:
          WritePieceMovement(curr);
          written_on_last_entry = true;
          break;

        case move_effect_piece_exchange:
          WritePieceExchange(curr);
          written_on_last_entry = true;
          break;

        case move_effect_board_transformation:
          WriteBoardTransformation(curr);
          written_on_last_entry = true;
          break;

        case move_effect_twinning_shift:
          WriteShift(curr);
          written_on_last_entry = true;
          break;

        case move_effect_input_condition:
          WriteConditions(0,&WriteCondition);
          written_on_last_entry = true;
          break;

        case move_effect_input_stipulation:
        case move_effect_input_sstipulation:
          WriteStipulation(curr);
          written_on_last_entry = true;
          break;

        case move_effect_twinning_polish:
          WritePolish(curr);
          written_on_last_entry = true;
          break;

        case move_effect_twinning_substitute:
          WriteSubstitute(curr);
          written_on_last_entry = true;
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

    EndTwinning();
  }

  last_horizon = top;
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
void output_latex_write_twinning(slice_index si)
{
  slice_index const file_owner = SLICE_NEXT2(si);
  FILE * const file = SLICE_U(file_owner).writer.file;
  unsigned int const twin_number = SLICE_U(si).twinning_handler.twin_number;
  boolean const continued = SLICE_U(si).twinning_handler.continued;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteTwinning(twin_number,continued);
  WriteTwinLetterToSolution(twin_number,file);

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
      break;

    default:
    {
      slice_index const file_owner = SLICE_NEXT2(si);
      slice_index const prototype = alloc_output_latex_writer(STOutputLaTeXTwinningWriter,file_owner);
      SLICE_U(prototype).twinning_handler.twin_number = event-twin_regular;
      SLICE_U(prototype).twinning_handler.continued = continued;
      slice_insertion_insert(si,&prototype,1);
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with slices that write the twinning in
 * LaTeX
 */
void output_latex_twinning_writer_builder_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

slice_index output_latex_alloc_twin_intro_writer_builder(slice_index file_owner)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",file_owner);
  TraceFunctionParamListEnd();

  result = alloc_output_latex_writer(STOutputLaTeXTwinningWriterBuilder,file_owner);
  SLICE_U(result).twinning_event_handler.handler = &handle_twinning_event;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
