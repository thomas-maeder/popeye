#include "output/latex/twinning.h"
#include "output/latex/latex.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/stipulation.h"
#include "output/plaintext/sstipulation.h"
#include "input/plaintext/stipulation.h"
#include "options/options.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "solving/machinery/twin.h"
#include "platform/tmpfile.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *twinning = 0;
static int twinning_pos = 0;

static move_effect_journal_index_type last_horizon;

void LaTeXWriteOptions(void)
{
  if (twinning==0)
    twinning = platform_open_tmpfile();

  if (twinning==0)
    perror("error opening tmpfile for LaTeX twinning by stipulation");

  if (twinning!=0)
  {
    if (OptFlag[duplex])
    {
      twinning_pos += fprintf(twinning, "%s", OptTab[duplex]);
      twinning_pos += fprintf(twinning, "%s", "{\\newline}");
    }
    else if (OptFlag[halfduplex])
    {
      twinning_pos += fprintf(twinning, "%s", OptTab[halfduplex]);
      twinning_pos += fprintf(twinning, "%s", "{\\newline}");
    }
    if (OptFlag[quodlibet])
    {
      twinning_pos += fprintf(twinning, "%s", OptTab[quodlibet]);
      twinning_pos += fprintf(twinning, "%s", "{\\newline}");
    }
  }
}

void LaTeXFlushTwinning(FILE *file)
{
  if (twinning==0)
    fprintf(file," \\%s{??}%%\n","twins");
  else
  {
    if (twinning_pos>=10)
    {
      /* remove the last "{\\newline} */
      twinning_pos -= 10;

      rewind(twinning);

      fprintf(file," \\%s{","twins");
      LaTeXCopyFile(twinning,file,twinning_pos);
      fputs("}%\n",file);
    }

    rewind(twinning);
    twinning_pos = 0;
  }
}

void LaTeXShutdownTwinning(void)
{
  if (twinning!=0)
  {
    platform_close_tmpfile(twinning);
    twinning = 0;
  }
}

static void BeginTwinning(unsigned int twin_number)
{
  if (twin_number-twin_a<='z'-'a')
    twinning_pos += fprintf(twinning, "%c) ", 'a'+twin_number-twin_a);
  else
    twinning_pos += fprintf(twinning, "z%u) ", (unsigned int)(twin_number-twin_a-('z'-'a')));
}

static void EndTwinning(void)
{
  twinning_pos += fprintf(twinning,"%s","{\\newline}");
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
      twinning_pos += fprintf(twinning, "%s", CondLine);
      break;

    case condition_subsequent:
      twinning_pos += fprintf(twinning, "%s", ", ");
      twinning_pos += fprintf(twinning, "%s", CondLine);
      break;

    case condition_end:
      break;
  }
}

void output_latex_twinning_writer_builder_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WritePieceCreation(move_effect_journal_index_type base,
                               move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  twinning_pos += fprintf(twinning,
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

  if (find_creation(curr+1,entry->u.piece_removal.on))
    return false;
  else
  {
    twinning_pos += fprintf(twinning, "%s", " --");
    twinning_pos += fprintf(twinning, "%s",
           is_piece_neutral(entry->u.piece_removal.flags) ? "\\n" : (TSTFLAG(entry->u.piece_removal.flags, White) ? "\\w" : "\\s"));
    twinning_pos += fprintf(twinning, "%s", LaTeXWalk(entry->u.piece_removal.walk));
    twinning_pos += fprintf(twinning, " %c%c",
            'a'-nr_files_on_board+entry->u.piece_removal.on%onerow,
            '1'-nr_rows_on_board+entry->u.piece_removal.on/onerow);
    return true;
  }
}

static void WritePieceMovement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  twinning_pos += fprintf(twinning,
                          "\\%c%s %c%c",
                          is_piece_neutral(entry->u.piece_movement.movingspec) ? 'n' : (TSTFLAG(entry->u.piece_movement.movingspec, White) ? 'w' : 's'),
                              LaTeXWalk(entry->u.piece_movement.moving),
                              'a'-nr_of_slack_files_left_of_board+entry->u.piece_movement.from%onerow,
                              '1'-nr_of_slack_rows_below_board+entry->u.piece_movement.from/onerow);

  twinning_pos += fprintf(twinning, "%s", "{\\ra}");

  twinning_pos += fprintf(twinning, "%c%c",
                          'a'-nr_files_on_board+entry->u.piece_movement.to%onerow,
                          '1'-nr_rows_on_board+entry->u.piece_movement.to/onerow);
}

static void WritePieceExchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  twinning_pos += fprintf(twinning, "\\%c%s %c%c",
          is_piece_neutral(entry->u.piece_exchange.fromflags) ? 'n' : (TSTFLAG(entry->u.piece_exchange.fromflags, White) ? 'w' : 's'),
          LaTeXWalk(get_walk_of_piece_on_square(entry->u.piece_exchange.to)),
          'a'-nr_of_slack_files_left_of_board+entry->u.piece_exchange.from%onerow,
          '1'-nr_of_slack_rows_below_board+entry->u.piece_exchange.from/onerow);

  twinning_pos += fprintf(twinning, "%s", "{\\lra}");

  twinning_pos += fprintf(twinning,  "\\%c%s %c%c",
          is_piece_neutral(entry->u.piece_exchange.toflags) ? 'n' : (TSTFLAG(entry->u.piece_exchange.toflags, White) ? 'w' : 's'),
          LaTeXWalk(get_walk_of_piece_on_square(entry->u.piece_exchange.from)),
          'a'-nr_files_on_board+entry->u.piece_exchange.to%onerow,
          '1'-nr_rows_on_board+entry->u.piece_exchange.to/onerow);
}

static void WriteBoardTransformation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  switch (entry->u.board_transformation.transformation)
  {
    case rot90:
      twinning_pos += fprintf(twinning,  "%s $%s^\\circ$", TwinningTab[TwinningRotate], "90");
      break;
    case rot180:
      twinning_pos += fprintf(twinning,  "%s $%s^\\circ$", TwinningTab[TwinningRotate], "180");
      break;
    case rot270:
      twinning_pos += fprintf(twinning,  "%s $%s^\\circ$", TwinningTab[TwinningRotate], "270");
      break;
    case mirra1h1:
      twinning_pos += fprintf(twinning,  "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1h1]);
      break;
    case mirra1a8:
      twinning_pos += fprintf(twinning,  "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1a8]);
      break;
    case mirra1h8:
      twinning_pos += fprintf(twinning,  "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora1h8]);
      break;
    case mirra8h1:
      twinning_pos += fprintf(twinning,  "%s %s", TwinningTab[TwinningMirror], TwinningMirrorTab[TwinningMirrora8h1]);
      break;

    default:
      assert(0);
      break;
  }
}

static void WriteShift(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  twinning_pos += fprintf(twinning,  "%s %c%c$\\Rightarrow$%c%c",
          TwinningTab[TwinningShift],
          'a'-nr_files_on_board+entry->u.twinning_shift.from%onerow,
          '1'-nr_rows_on_board+entry->u.twinning_shift.from/onerow,
          'a'-nr_files_on_board+entry->u.twinning_shift.to%onerow,
          '1'-nr_rows_on_board+entry->u.twinning_shift.to/onerow);
}

static void WriteStip(move_effect_journal_index_type curr)
{
  slice_index const stipulation = move_effect_journal[curr].u.input_stipulation.stipulation;

  twinning_pos += WriteStipulation(twinning,stipulation);

  if (OptFlag[solapparent])
    twinning_pos += fprintf(twinning, "%s", "*");

  if (OptFlag[whitetoplay])
  {
    char temp[10];        /* increased due to buffer overflow */
    sprintf(temp, " %c{\\ra}",tolower(*PieSpTab[White]));
    twinning_pos += fprintf(twinning, "%s", temp);
  }
}

static void WriteSStip(move_effect_journal_index_type curr)
{
  slice_index const stipulation = move_effect_journal[curr].u.input_stipulation.stipulation;

  twinning_pos += WriteSStipulation(twinning,stipulation);

  if (OptFlag[solapparent])
    twinning_pos += fprintf(twinning, "%s", "*");

  if (OptFlag[whitetoplay])
  {
    char temp[10];        /* increased due to buffer overflow */
    sprintf(temp, " %c{\\ra}",tolower(*PieSpTab[White]));
    twinning_pos += fprintf(twinning, "%s", temp);
  }
}

static void WritePolish(move_effect_journal_index_type curr)
{
  twinning_pos += fprintf(twinning, "%s", TwinningTab[TwinningPolish]);
}

static void WriteSubstitute(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type const *entry = &move_effect_journal[curr];

  twinning_pos += fprintf(twinning, "{\\w%s} $\\Rightarrow$ \\w%s",
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
  if (twinning!=0)
  {
    move_effect_journal_index_type const top = move_effect_journal_base[ply_twinning+1];
    move_effect_journal_index_type const base = continued ? last_horizon : move_effect_journal_base[ply_twinning];
    move_effect_journal_index_type curr;
    boolean written_on_last_entry = false;

    assert(base<=top);

    if (base<top)
    {
      if (continued)
        twinning_pos += fprintf(twinning, "%s", "+");

      BeginTwinning(twin_number);

      for (curr = base; curr!=top; ++curr)
        if (move_effect_journal[curr].type!=move_effect_walk_change)
        {
          if (written_on_last_entry)
          {
            twinning_pos += fprintf(twinning, "%s", ", ");
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
              WriteStip(curr);
              written_on_last_entry = true;
              break;
            case move_effect_input_sstipulation:
              WriteSStip(curr);
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
            case move_effect_remove_stipulation:
            case move_effect_hunter_type_definition:
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
