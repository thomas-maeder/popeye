#include "position/board.h"

#include "debugging/assert.h"

/* NOTE: Code in input/plaintext/condition.c (at least) assumes that
         getBoardFileLabel(unsigned int) and getBoardRowLabel(unsigned int)
         return distinct outputs of tolower(int) (declared in ctype.h). */

/* Declaration of the symbols we'll use to identify files on the board
 */
static board_label_type const BOARD_FILE_LABELS[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                     'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

/* Declaration of the symbols we'll use to identify rows on the board
 */
static board_label_type const BOARD_ROW_LABELS[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

board_label_type getBoardFileLabel(unsigned int const index) /* index should be in [0, nr_files_on_board-1] */
{
  {
    enum
    {
      ensure_we_have_enough_board_file_labels = 1/(nr_files_on_board <= ((sizeof BOARD_FILE_LABELS)/(sizeof *BOARD_FILE_LABELS)))
    };
  }
  assert(index < nr_files_on_board);
  return BOARD_FILE_LABELS[index];
}

board_label_type getBoardRowLabel(unsigned int const index) /* index should be in [0, nr_rows_on_board-1] */
{
  {
    enum
    {
      ensure_we_have_enough_board_row_labels = 1/(nr_rows_on_board <= ((sizeof BOARD_ROW_LABELS)/(sizeof *BOARD_ROW_LABELS)))
    };
  }
  assert(index < nr_rows_on_board);
  return BOARD_ROW_LABELS[index];
}

unsigned int getBoardFileIndex(board_label_type const label) /* returns nr_files_on_board if label is invalid */
{
  {
    enum
    {
      ensure_we_have_enough_board_file_labels = 1/(nr_files_on_board <= ((sizeof BOARD_FILE_LABELS)/(sizeof *BOARD_FILE_LABELS)))
    };
  }
  unsigned int index;
  for (index = 0; ((index < nr_files_on_board) && (BOARD_FILE_LABELS[index] != label)); ++index)
    ; /* do nothing */
  return index;
}

unsigned int getBoardRowIndex(board_label_type const label) /* returns nr_rows_on_board if label is invalid */
{
  {
    enum
    {
      ensure_we_have_enough_board_row_labels = 1/(nr_rows_on_board <= ((sizeof BOARD_ROW_LABELS)/(sizeof *BOARD_ROW_LABELS)))
    };
  }
  unsigned int index;
  for (index = 0; ((index < nr_rows_on_board) && (BOARD_ROW_LABELS[index] != label)); ++index)
    ; /* do nothing */
  return index;
}

/* 0 terminated sequence of the effective squares of the board
 */
square const boardnum[65] = {
  /* first   rank */      square_a1, 201, 202, 203, 204, 205, 206, 207,
  /* second  rank */      224, 225, 226, 227, 228, 229, 230, 231,
  /* third   rank */      248, 249, 250, 251, 252, 253, 254, 255,
  /* fourth  rank */      272, 273, 274, 275, 276, 277, 278, 279,
  /* fifth   rank */      296, 297, 298, 299, 300, 301, 302, 303,
  /* sixth   rank */      320, 321, 322, 323, 324, 325, 326, 327,
  /* seventh rank */      344, 345, 346, 347, 348, 349, 350, 351,
  /* eighth  rank */      368, 369, 370, 371, 372, 373, 374, square_h8,
  /* end marker   */    0};

SquareFlags zzzan[(one_row + 1) + (square_h8 - square_a1 + 1) + (one_row + 1)];

int zzzao[square_h8 - square_a1 + 1];

/* Calculate a square transformation
 * @param sq square to be reflected
 * @param transformation transformation to be performed
 * @return transformed value of sq
 */
square transformSquare(square sq, SquareTransformation transformation)
{
  square ret = sq;

  switch (transformation)
  {
    case rot90:
      ret = onerow*(sq%onerow)-sq/onerow+(onerow-1);
      break;

    case rot180:
      ret = square_h8+square_a1 - sq;
      break;

    case rot270:
      ret = -onerow*(sq%onerow)+sq/onerow-(onerow-1)+square_h8+square_a1;
      break;

    case mirra1a8:
      ret = sq%onerow+onerow*((onerow-1)-sq/onerow);
      break;

    case mirra1h1:
      ret = ((onerow-1)-sq%onerow)+onerow*(sq/onerow);
      break;

    case mirra8h1:
      ret = onerow*(sq%onerow)+sq/onerow;
      break;

    case mirra1h8:
      ret = ((onerow-1)-sq/onerow)+onerow*((onerow-1)-sq%onerow);
      break;

    default:
      assert(0);
      break;
  }

  return ret;
}
