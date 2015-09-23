#include "output/plaintext/position.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/message.h"
#include "output/plaintext/stdio.h"
#include "output/output.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "conditions/grid.h"
#include "conditions/imitator.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/hunters.h"
#include "position/position.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "solving/proofgames.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <string.h>

enum
{
  fileWidth = 4,
  border_width = 5,
  board_width = fileWidth*nr_files_on_board + border_width
};

static boolean is_square_occupied_by_imitator(position const *pos, square s)
{
  boolean result = false;
  unsigned int imi_idx;

  for (imi_idx = 0; imi_idx<pos->number_of_imitators; ++imi_idx)
    if (s==pos->isquare[imi_idx])
    {
      result = true;
      break;
    }

  return result;
}

static void WriteCastlingMutuallyExclusive(void)
{
  /* no need to test in [Black] - information is redundant */
  if (castling_mutual_exclusive[White][queenside_castling-min_castling]!=0
      || castling_mutual_exclusive[White][kingside_castling-min_castling]!=0)
  {
    protocol_fprintf(stdout,"%s",OptTab[mutuallyexclusivecastling]);

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &ra_cancastle))
    {
      protocol_fputc(' ',stdout);
      WriteSquare(&output_plaintext_engine,stdout,square_a1);
      WriteSquare(&output_plaintext_engine,stdout,square_a8);
    }

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &rh_cancastle))
    {
      protocol_fputc(' ',stdout);
      WriteSquare(&output_plaintext_engine,stdout,square_a1);
      WriteSquare(&output_plaintext_engine,stdout,square_h8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &ra_cancastle))
    {
      protocol_fputc(' ',stdout);
      WriteSquare(&output_plaintext_engine,stdout,square_h1);
      WriteSquare(&output_plaintext_engine,stdout,square_a8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &rh_cancastle))
    {
      protocol_fputc(' ',stdout);
      WriteSquare(&output_plaintext_engine,stdout,square_h1);
      WriteSquare(&output_plaintext_engine,stdout,square_h8);
    }

    protocol_fputc('\n',stdout);
  }
}

static void WriteGrid(void)
{
  square square, square_a;
  int row, column;
  char    HLine[40];

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c                                   %c\n";
  static char BlankL[]="|                                   |\n";

  protocol_fputc('\n',stdout);
  protocol_fprintf(stdout,"%s",BorderL);
  protocol_fprintf(stdout,"%s",BlankL);

  for (row=0, square_a = square_a8;
       row<nr_rows_on_board;
       row++, square_a += dir_down) {
    char const *digits="87654321";
    sprintf(HLine, HorizL, digits[row], digits[row]);

    for (column=0, square= square_a;
         column<nr_files_on_board;
         column++, square += dir_right)
    {
      char g = (GridNum(square))%100;
      HLine[fileWidth*column+3]= g>9 ? (g/10)+'0' : ' ';
      HLine[fileWidth*column+4]= (g%10)+'0';
    }

    protocol_fprintf(stdout,"%s",HLine);
    protocol_fprintf(stdout,"%s",BlankL);
  }

  protocol_fprintf(stdout,"%s",BorderL);
}

static void WritePiecesWithAttribute(position const *pos, piece_flag_type sp)
{
  char squares[3*nr_rows_on_board*nr_files_on_board+1] = "";
  square square_a = square_a8;
  unsigned int row;

  for (row = 1; row<=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int column;
    square square = square_a;

    for (column = 1; column<=nr_files_on_board; ++column, square += dir_right)
      if (TSTFLAG(pos->spec[square],sp))
        AppendSquare(squares,square);
  }

  protocol_fprintf_c(stdout,board_width,"%s%s\n",PieSpTab[sp-nr_sides],squares);
}

static void WriteNonRoyalAttributedPieces(position const *pos)
{
  piece_flag_type sp;

  for (sp = Royal+1; sp<nr_piece_flags; ++sp)
    if (!TSTFLAG(all_pieces_flags,sp) && TSTFLAG(some_pieces_flags,sp))
      WritePiecesWithAttribute(pos,sp);
}

static void WriteRoyalPiecePositions(position const *pos)
{
  char squares[3*nr_rows_on_board*nr_files_on_board+1] = "";
  unsigned int nr_royals = 0;

  square square_a = square_a8;
  unsigned int row;

  for (row = 0; row!=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int column;
    square square = square_a;

    for (column = 0; column!=nr_files_on_board; ++column, square += dir_right)
      if (TSTFLAG(pos->spec[square],Royal)
          && !is_king(pos->board[square]))
      {
        AppendSquare(squares,square);
        ++nr_royals;
      }
  }

  if (nr_royals>0)
    protocol_fprintf_c(stdout,board_width,"%s%s\n",PieSpTab[Royal-nr_sides],squares);
}

static void DoPieceCounts(position const *pos,
                          unsigned piece_per_colour[nr_colours])
{
  square square_a = square_a8;
  unsigned int row;

  for (row = 0; row!=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int column;
    square square = square_a;

    for (column = 0; column!=nr_files_on_board; ++column, square += dir_right)
    {
      if (is_piece_neutral(pos->spec[square]))
        ++piece_per_colour[colour_neutral];
      else if (TSTFLAG(pos->spec[square],Black))
        ++piece_per_colour[colour_black];
      else if (TSTFLAG(pos->spec[square],White))
        ++piece_per_colour[colour_white];
    }
  }
}

static int indentation = 0;

static void WritePieceCounts(position const *pos, unsigned int indentation)
{
  unsigned piece_per_colour[nr_colours] = { 0 };

  DoPieceCounts(pos,piece_per_colour);

  {
    char const *format = piece_per_colour[colour_neutral]>0 ? "%d + %d + %dn\n" : "%d + %d\n";
    int const width = nr_files_on_board*fileWidth+4-indentation;
    protocol_fprintf_r(stdout,width,format,
                       piece_per_colour[colour_white],
                       piece_per_colour[colour_black],
                       piece_per_colour[colour_neutral]);
  }
}

static int WriteStipulation(void)
{
  char StipLower[sizeof AlphaStip];
  char *stip = AlphaStip;
  char *lower = StipLower;

  while (*stip)
    *lower++ = tolower(*stip++);
  *lower = 0;

  return protocol_fprintf(stdout,"  %s",StipLower);
}

typedef enum
{
  sstip_write_state_begin,
  sstip_write_state_branch,
  sstip_write_state_end
} sstip_write_state;

typedef enum
{
  sstip_play_attack,
  sstip_play_defense,
  sstip_play_help,
  sstip_play_series,

  sstip_nr_plays,
  sstip_play_unknown = sstip_nr_plays
} sstip_play_type;

typedef struct
{
    FILE *file;
    int nr_chars_written;
    sstip_write_state state;
    sstip_play_type play;
} sstip_write_type;

static void sstip_write_move_inverter(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","-");
  stip_traverse_structure_children(si,st);
}

static void sstip_write_attack(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_play_type const save_play = sstip_write->play;

  if (sstip_write->state==sstip_write_state_begin)
  {
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%u",SLICE_U(si).branch.length);
    if (SLICE_U(si).branch.min_length>1)
      sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,":%u",SLICE_U(si).branch.min_length+1);
  }

  sstip_write->play = sstip_play_attack;
  stip_traverse_structure_children(si,st);
  sstip_write->play = save_play;
}

static void sstip_write_attack_played(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  assert(sstip_write->play==sstip_play_attack);

  if (sstip_write->state==sstip_write_state_begin)
  {
    sstip_write->state = sstip_write_state_branch;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_begin;
  }
  else
  {
    assert(sstip_write->state==sstip_write_state_branch);
    sstip_write->state = sstip_write_state_end;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_branch;
  }
}

static void sstip_write_defense(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_play_type const save_play = sstip_write->play;

  if (sstip_write->state==sstip_write_state_begin)
  {
    if (branch_find_slice(STPlaySuppressor,si,st->context)!=no_slice)
      sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","/");
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%u",SLICE_U(si).branch.length);
    if (SLICE_U(si).branch.min_length>1)
      sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,":%u",SLICE_U(si).branch.min_length);
  }

  sstip_write->play = sstip_play_defense;
  stip_traverse_structure_children(si,st);
  sstip_write->play = save_play;
}

static void sstip_write_defense_played(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  assert(sstip_write->play==sstip_play_defense);

  if (sstip_write->state==sstip_write_state_begin)
  {
    sstip_write->state = sstip_write_state_branch;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_begin;
  }
  else
  {
    assert(sstip_write->state==sstip_write_state_branch);
    sstip_write->state = sstip_write_state_end;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_branch;
  }
}

static void sstip_write_help(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  if (sstip_write->state==sstip_write_state_begin)
  {
    sstip_play_type const save_play = sstip_write->play;
    slice_index const leg2 = branch_find_slice(STReadyForHelpMove,si,st->context);
    if (leg2==si)
    {
      sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%u",(SLICE_U(si).branch.length+1)/2);
      if (SLICE_U(si).branch.min_length>1)
        sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,":%u",(SLICE_U(si).branch.min_length+1)/2);
      sstip_write->play = sstip_play_series;
    }
    else
    {
      sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%u",SLICE_U(si).branch.length);
      if (SLICE_U(si).branch.min_length>1)
        sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,":%u",SLICE_U(si).branch.min_length);
      sstip_write->play = sstip_play_help;
    }

    stip_traverse_structure_children(si,st);
    sstip_write->play = save_play;
  }
  else
    stip_traverse_structure_children(si,st);
}

static void sstip_write_help_played(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  if (sstip_write->state==sstip_write_state_begin)
  {
    sstip_write->state = sstip_write->play==sstip_play_help ? sstip_write_state_branch : sstip_write_state_end;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_begin;
  }
  else if (sstip_write->state==sstip_write_state_branch)
  {
    sstip_write->state = sstip_write_state_end;
    stip_traverse_structure_children(si,st);
    sstip_write->state = sstip_write_state_branch;
  }
}

static void sstip_write_move(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  char const play2char[sstip_nr_plays] = { 'a','d','h','s' };

  assert(sstip_write->play<sstip_nr_plays);
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,
                                                    "%c",
                                                    play2char[sstip_write->play]);

  stip_traverse_structure_children(si,st);
}

static void sstip_write_end_of_branch(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write_state const save_state = sstip_write->state;
  sstip_play_type const save_play = sstip_write->play;

  if (sstip_write->state==sstip_write_state_begin)
  {
    /* don't write the nested branch at the beginning */
    stip_traverse_structure_children_pipe(si,st);

    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","[");
    sstip_write->state = sstip_write_state_begin;
    sstip_write->play = sstip_play_unknown;
    stip_traverse_structure_next_branch(si,st);
    sstip_write->play = save_play;
    sstip_write->state = save_state;
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","]");
  }
  else
  {
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","[");
    sstip_write->state = sstip_write_state_begin;
    sstip_write->play = sstip_play_unknown;
    stip_traverse_structure_next_branch(si,st);
    sstip_write->play = save_play;
    sstip_write->state = save_state;
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","]");

    stip_traverse_structure_children_pipe(si,st);
  }
}

static void sstip_write_end_of_branch_forced(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write_state const save_state = sstip_write->state;
  sstip_play_type const save_play = sstip_write->play;

  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","[>");
  sstip_write->state = sstip_write_state_begin;
  sstip_write->play = sstip_play_unknown;
  stip_traverse_structure_next_branch(si,st);
  sstip_write->play = save_play;
  sstip_write->state = save_state;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","]");

  stip_traverse_structure_children_pipe(si,st);
}

static void sstip_write_constraint_tester(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write_state const save_state = sstip_write->state;
  sstip_play_type const save_play = sstip_write->play;

  if (sstip_write->state==sstip_write_state_begin)
  {
    /* don't write the constraint at the beginning */
    stip_traverse_structure_children_pipe(si,st);

    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","{");
    sstip_write->state = sstip_write_state_begin;
    sstip_write->play = sstip_play_unknown;
    stip_traverse_structure_next_branch(si,st);
    sstip_write->play = save_play;
    sstip_write->state = save_state;
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","}");
  }
  else
  {
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","{");
    sstip_write->state = sstip_write_state_begin;
    sstip_write->play = sstip_play_unknown;
    stip_traverse_structure_next_branch(si,st);
    sstip_write->play = save_play;
    sstip_write->state = save_state;
    sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","}");

    stip_traverse_structure_children_pipe(si,st);
  }
}

static void sstip_write_goal_reached_tester(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write_state const save_state = sstip_write->state;
  sstip_play_type const save_play = sstip_write->play;

  sstip_write->state = sstip_write_state_begin;
  sstip_write->play = sstip_play_unknown;
  stip_traverse_structure_next_branch(si,st);
  sstip_write->play = save_play;
  sstip_write->state = save_state;
}

static void sstip_write_mate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","#");
}

static void sstip_write_stalemate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","=");
}

static void sstip_write_doublestalemate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","==");
}

static void sstip_write_immobile(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","#=");
}

static void sstip_write_check(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","+");
}

static void sstip_write_target(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  square const s = SLICE_U(si).goal_handler.goal.target;

  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","z");
  /* TODO avoid duplication with WriteSquare() */
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%c",('a' - nr_files_on_board + s%onerow));
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%c",('1' - nr_rows_on_board + s/onerow));
}

static void sstip_write_capture(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","x");
}

static void sstip_write_castling(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","00");
}

static void sstip_write_steingewinn(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","%");
}

static void sstip_write_enpassant(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","ep");
}

static void sstip_write_doublemate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","##");
}

static void sstip_write_countermate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","##!");
}

static void sstip_write_autostalemate(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","!=");
}

static void sstip_write_circuit(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","ct");
}

static void sstip_write_exchange(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","<>");
}

static void sstip_write_any(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","~");
}

static void sstip_write_dia(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","dia");
}

static void sstip_write_atob(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","a=>b");
}

static void sstip_write_chess81(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","c81");
}

static void sstip_write_kiss(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","k");
}

static void sstip_write_if_then_else(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write_state const save_state = sstip_write->state;
  stip_traversal_context_type const save_context = st->context;
  sstip_play_type const save_play = sstip_write->play;

  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","?");
  sstip_write->state = sstip_write_state_begin;
  sstip_write->play = sstip_play_unknown;
  st->context = stip_traversal_context_intro;
  stip_traverse_structure(SLICE_U(si).if_then_else.condition,st);
  st->context = save_context;
  sstip_write->play = save_play;
  sstip_write->state = save_state;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","?");

  stip_traverse_structure_binary_operand1(si,st);
  stip_traverse_structure_binary_operand2(si,st);
}

static void sstip_write_not(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","!");
  stip_traverse_structure_children_pipe(si,st);
}

static void sstip_write_and(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  stip_traverse_structure_binary_operand1(si,st);
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","&");
  stip_traverse_structure_binary_operand2(si,st);
}

static void sstip_write_or(slice_index si, stip_structure_traversal *st)
{
  sstip_write_type * const sstip_write = st->param;

  stip_traverse_structure_binary_operand1(si,st);
  sstip_write->nr_chars_written += protocol_fprintf(sstip_write->file,"%s","|");
  stip_traverse_structure_binary_operand2(si,st);
}

structure_traversers_visitor const sstip_visitors[] = {
    { STMoveInverter,  &sstip_write_move_inverter },
    { STReadyForAttack,  &sstip_write_attack  },
    { STAttackPlayed, &sstip_write_attack_played },
    { STReadyForDefense, &sstip_write_defense },
    { STDefensePlayed, &sstip_write_defense_played },
    { STReadyForHelpMove, &sstip_write_help },
    { STHelpMovePlayed, &sstip_write_help_played },
    { STMove, &sstip_write_move },
    { STEndOfBranch, &sstip_write_end_of_branch },
    { STEndOfBranchGoal, &sstip_write_end_of_branch },
    { STEndOfBranchGoalImmobile, &sstip_write_end_of_branch },
    { STEndOfBranchForced, &sstip_write_end_of_branch_forced },
    { STConstraintTester, &sstip_write_constraint_tester },
    { STGoalConstraintTester, &sstip_write_constraint_tester },
    { STGoalReachedTester, &sstip_write_goal_reached_tester },
    { STGoalMateReachedTester, &sstip_write_mate },
    { STGoalStalemateReachedTester, &sstip_write_stalemate },
    { STGoalDoubleStalemateReachedTester, &sstip_write_doublestalemate },
    { STGoalImmobileReachedTester, &sstip_write_immobile },
    { STGoalCheckReachedTester, &sstip_write_check },
    { STGoalTargetReachedTester, &sstip_write_target },
    { STGoalCaptureReachedTester, &sstip_write_capture },
    { STGoalCastlingReachedTester, &sstip_write_castling },
    { STGoalSteingewinnReachedTester, &sstip_write_steingewinn },
    { STGoalEnpassantReachedTester, &sstip_write_enpassant },
    { STGoalDoubleMateReachedTester, &sstip_write_doublemate },
    { STGoalCounterMateReachedTester, &sstip_write_countermate },
    { STGoalAutoStalemateReachedTester, &sstip_write_autostalemate },
    { STGoalCircuitReachedTester, &sstip_write_circuit },
    { STGoalExchangeReachedTester, &sstip_write_exchange },
    { STGoalAnyReachedTester, &sstip_write_any },
    { STGoalProofgameReachedTester, &sstip_write_dia },
    { STGoalAToBReachedTester, &sstip_write_atob },
    { STGoalChess81ReachedTester, &sstip_write_chess81 },
    { STGoalKissReachedTester, &sstip_write_kiss },
    { STIfThenElse, &sstip_write_if_then_else },
    { STNot, &sstip_write_not },
    { STAnd, &sstip_write_and },
    { STOr, &sstip_write_or }
};
enum { nr_sstip_visitors = sizeof sstip_visitors / sizeof sstip_visitors[0]};


static int WriteSStipulation(slice_index si)
{
  slice_index const stipulation = SLICE_NEXT2(si);
  Side const starter = SLICE_STARTER(stipulation);
  sstip_write_type sstip_write = { stdout, 0, sstip_write_state_begin, sstip_play_unknown };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(stipulation);

  sstip_write.nr_chars_written += protocol_fprintf(stdout,"  %s ",ColourTab[starter]);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&sstip_write);
    stip_structure_traversal_override(&st,sstip_visitors,nr_sstip_visitors);
    stip_traverse_structure(stipulation,&st);
  }


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",sstip_write.nr_chars_written);
  TraceFunctionResultEnd();
  return sstip_write.nr_chars_written;
}

static int WriteOptions(position const *pos)
{
  int result = 0;

  TraceValue("%u\n",indentation);

  if (OptFlag[solmenaces])
  {
    result += protocol_fprintf(stdout, "/%u", get_max_threat_length());
    if (OptFlag[solflights])
      result += protocol_fprintf(stdout, "/%d", get_max_flights());
  }
  else if (OptFlag[solflights])
    result += protocol_fprintf(stdout, "//%d", get_max_flights());

  if (OptFlag[nontrivial])
    result += protocol_fprintf(stdout,";%d,%u",
                              max_nr_nontrivial,
                              get_min_length_nontrivial());

  return result;
}

static char *WriteWalkRtoL(char *pos, piece_walk_type walk)
{
  pos[0] = PieceTab[walk][1];
  if (pos[0]!=' ')
  {
    pos[0] = toupper(pos[0]);
    --pos;
  }

  pos[0] = toupper(PieceTab[walk][0]);
  --pos;

  return pos;
}

static void WriteRegularCells(position const *pos, square square_a)
{
  unsigned int column;
  square square;

  for (column = 0,  square = square_a;
      column!=nr_files_on_board;
       ++column, square += dir_right)
  {
    char cell[fileWidth+1];
    char *pos_in_cell = cell + (sizeof cell)/2;

    snprintf(cell, sizeof cell, "%*c", fileWidth, ' ');

    if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    {
      if (is_on_board(square+dir_left)
          && GridLegal(square, square+dir_left))
        cell[0] = '|';
    }

    if (is_square_occupied_by_imitator(pos,square))
      pos_in_cell[0] = 'I';
    else if (pos->board[square]==Invalid)
      pos_in_cell[0] = ' ';
    else if (pos->board[square]==Empty)
      pos_in_cell[0] = '.';
    else
    {
      piece_walk_type const walk = pos->board[square];
      if (walk<Hunter0 || walk>=Hunter0+max_nr_hunter_walks)
        pos_in_cell = WriteWalkRtoL(pos_in_cell,walk);
      else
      {
        unsigned int const hunterIndex = walk-Hunter0;
        assert(hunterIndex<max_nr_hunter_walks);

        pos_in_cell[1] = '/';
        pos_in_cell = WriteWalkRtoL(pos_in_cell,huntertypes[hunterIndex].away);
      }

      if (is_piece_neutral(pos->spec[square]))
        pos_in_cell[0] = '=';
      else if (TSTFLAG(pos->spec[square],Black))
        pos_in_cell[0] = '-';
    }

    protocol_fprintf(stdout,"%s",cell);
  }
}

static void WriteBaseCells(position const *pos, square square_a)
{
  unsigned int column;
  square square;

  for (column = 0, square = square_a;
      column!=nr_files_on_board;
       ++column, square += dir_right)
  {
    piece_walk_type const walk = pos->board[square];

    char cell[fileWidth+1];
    char *pos_in_cell = cell + (sizeof cell)/2;

    snprintf(cell, sizeof cell, "%*c", fileWidth, ' ');

    if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    {
      if (is_on_board(square+dir_down)
          && GridLegal(square,square+dir_down))
      {
        pos_in_cell[-1] = '-';
        pos_in_cell[0] = '-';
        pos_in_cell[+1] = '-';
      }
    }

    if (Hunter0<=walk && walk<Hunter0+max_nr_hunter_walks)
    {
      unsigned int const hunterIndex = walk-Hunter0;
      WriteWalkRtoL(pos_in_cell,huntertypes[hunterIndex].home);
    }

    protocol_fprintf(stdout,"%s",cell);
  }
}

static void WriteBorder(void)
{
  unsigned int column;
  char letter;

  assert(nr_files_on_board <= 'z'-'a');

  protocol_fprintf(stdout,"%s","+--");

  for (column = 0, letter = 'a'; column!=nr_files_on_board; ++column, ++letter)
  {
    char cell[fileWidth+1];
    snprintf(cell, sizeof cell, "-%c--", letter);
    protocol_fprintf(stdout,"%s",cell);
  }

  protocol_fprintf(stdout,"%s","-+\n");
}

static void WriteBlankLine(void)
{
  unsigned int column;

  protocol_fprintf(stdout,"%s","| ");
  protocol_fprintf(stdout,"%s"," ");

  for (column = 0; column!=nr_files_on_board; ++column)
    protocol_fprintf(stdout,"%s","    ");

  protocol_fprintf(stdout,"%s"," |\n");
}

void WriteBoard(position const *pos)
{
  unsigned int row;
  square square_a;

  assert(nr_rows_on_board<10);

  protocol_fputc('\n',stdout);
  WriteBorder();
  WriteBlankLine();

  for (row = 0, square_a = square_a8;
       row!=nr_rows_on_board;
       ++row, square_a += dir_down)
  {
    protocol_fprintf(stdout,"%d ",nr_rows_on_board-row);
    WriteRegularCells(pos,square_a);
    protocol_fprintf(stdout,"  %d", nr_rows_on_board-row);
    protocol_fputc('\n',stdout);

    protocol_fprintf(stdout,"%s","| ");
    WriteBaseCells(pos,square_a);
    protocol_fprintf(stdout,"%s","  |\n");
  }

  WriteBorder();
}

static void WriteMeta(void)
{
  protocol_fprintf(stdout,"%s","\n");
  protocol_fputs_c_multi(stdout,board_width,ActAuthor);
  protocol_fputs_c_multi(stdout,board_width,ActOrigin);
  protocol_fputs_c_multi(stdout,board_width,ActAward);
  protocol_fputs_c_multi(stdout,board_width,ActTitle);
}

static void WriteCondition(FILE* dummy, char const CondLine[], condition_rank rank)
{
  if (rank!=condition_end)
    protocol_fprintf_c(stdout,board_width,"%s\n",CondLine);
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
void output_plaintext_write_meta(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteMeta();

  pipe_solve_delegate(si);

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
void output_plaintext_write_board(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteBoard(slices[si].u.position_handler.position);
  indentation = 0;

  pipe_solve_delegate(si);

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
void output_plaintext_write_piece_counts(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WritePieceCounts(slices[si].u.position_handler.position,indentation);

  pipe_solve_delegate(si);

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
void output_plaintext_write_royal_piece_positions(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteRoyalPiecePositions(slices[si].u.position_handler.position);

  pipe_solve_delegate(si);

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
void output_plaintext_write_stipulation_options(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  indentation += WriteOptions(&being_solved);

  pipe_solve_delegate(si);

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
void output_plaintext_write_stipulation(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  indentation += WriteStipulation();

  pipe_solve_delegate(si);

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
void output_plaintext_write_sstipulation(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  indentation += WriteSStipulation(si);

  pipe_solve_delegate(si);

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
void output_plaintext_write_non_royal_attributes(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteNonRoyalAttributedPieces(slices[si].u.position_handler.position);

  pipe_solve_delegate(si);

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
void output_plaintext_write_conditions(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteConditions(0,&WriteCondition);

  pipe_solve_delegate(si);

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
void output_plaintext_write_mutually_exclusive_castlings(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteCastlingMutuallyExclusive();

  pipe_solve_delegate(si);

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
void output_plaintext_write_duplex(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  protocol_fprintf_c(stdout,board_width,"%s\n",OptTab[duplex]);

  pipe_solve_delegate(si);

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
void output_plaintext_write_halfduplex(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  protocol_fprintf_c(stdout,board_width,"%s\n",OptTab[halfduplex]);

  pipe_solve_delegate(si);

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
void output_plaintext_write_quodlibet(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  protocol_fprintf_c(stdout,board_width,"%s\n",OptTab[quodlibet]);

  pipe_solve_delegate(si);

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
void output_plaintext_write_grid(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  WriteGrid();

  pipe_solve_delegate(si);

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
void output_plaintext_write_atob_intra(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  protocol_fputc('\n',stdout);
  protocol_fprintf_c(stdout,board_width,"=> (%s ->)\n",ColourTab[SLICE_STARTER(si)]);
  protocol_fputc('\n',stdout);

  pipe_solve_delegate(si);

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
void output_plaintext_end_of_position_writers_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  protocol_fflush(stdout);

  pipe_solve_delegate(si);

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index alloc_position_handler(slice_type type, position const *pos)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.position_handler.position = pos;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void output_plaintext_build_position_writers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(find_unique_goal(si).type!=goal_proofgame);
  assert(find_unique_goal(si).type!=goal_atob);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STOutputPlainTextMetaWriter),
        alloc_position_handler(STOutputPlainTextBoardWriter,&being_solved),
        alloc_pipe(STOutputPlainTextStipulationOptionsWriter),
        alloc_position_handler(STOutputPlainTextPieceCountsWriter,&being_solved),
        alloc_position_handler(STOutputPlainTextRoyalPiecePositionsWriter,&being_solved),
        alloc_position_handler(STOutputPlainTextNonRoyalAttributesWriter,&being_solved),
        alloc_pipe(STOutputPlainTextConditionsWriter),
        alloc_pipe(STOutputPlainTextEndOfPositionWriters)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_plaintext_build_proof_position_writers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STOutputPlainTextMetaWriter),
        alloc_pipe(STOutputPlainTextStartOfTargetWriter),
        alloc_position_handler(STOutputPlainTextBoardWriter,&proofgames_target_position),
        alloc_pipe(STOutputPlainTextStipulationOptionsWriter),
        alloc_position_handler(STOutputPlainTextPieceCountsWriter,&proofgames_target_position),
        alloc_position_handler(STOutputPlainTextRoyalPiecePositionsWriter,&proofgames_target_position),
        alloc_position_handler(STOutputPlainTextNonRoyalAttributesWriter,&proofgames_target_position),
        alloc_pipe(STOutputPlainTextConditionsWriter),
        alloc_pipe(STOutputPlainTextEndOfPositionWriters)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index alloc_atob_intra_writer(Side starter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlainTextAToBIntraWriter);
  SLICE_STARTER(result) = starter;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void output_plaintext_build_atob_start_position_writers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_position_handler(STOutputPlainTextBoardWriter,&being_solved),
        alloc_position_handler(STOutputPlainTextPieceCountsWriter,&being_solved),
        alloc_position_handler(STOutputPlainTextRoyalPiecePositionsWriter,&being_solved),
        alloc_position_handler(STOutputPlainTextNonRoyalAttributesWriter,&being_solved),
        alloc_atob_intra_writer(SLICE_STARTER(si))
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean output_plaintext_are_there_position_writers(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const writer = branch_find_slice(STOutputPlainTextMetaWriter,
                                                 si,
                                                 stip_traversal_context_intro);
    result = writer!=no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_plaintext_remove_position_writers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_by_function(&st,
                                                  slice_function_output_plaintext_position_writer,
                                                  &remove_writer);
    stip_structure_traversal_override_single(&st,
                                             STPiecesCounter,
                                             &stip_structure_visitor_noop);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_plaintext_option_noboard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_plaintext_remove_position_writers(si);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
