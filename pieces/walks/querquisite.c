#include "pieces/walks/querquisite.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

/* Generate moves for an Querquisite
 */
void querquisite_generate_moves(void)
{
  switch (curr_generation->departure%onerow - nr_of_slack_files_left_of_board)
  {
    case file_rook_queenside:
    case file_rook_kingside:
      rider_generate_moves(vec_rook_start,vec_rook_end);
      break;
    case file_bishop_queenside:
    case file_bishop_kingside:
      rider_generate_moves(vec_bishop_start,vec_bishop_end);
      break;
    case file_queen:
      rider_generate_moves(vec_queen_start,vec_queen_end);
      break;
    case file_knight_queenside:
    case file_knight_kingside:
      leaper_generate_moves(vec_knight_start,vec_knight_end);
      break;
    case file_king:
      leaper_generate_moves(vec_queen_start,vec_queen_end);
      break;
  }
}
