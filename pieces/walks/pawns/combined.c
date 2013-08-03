#include "pieces/walks/pawns/combined.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a Dragon
 */
void dragon_generate_moves(void)
{
  pawn_generate_moves();
  leaper_generate_moves(vec_knight_start,vec_knight_end);
}

/* Generate moves for a Gryphon
 */
void gryphon_generate_moves(void)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],
                                                                    curr_generation->departure);

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(dir_forward,no_capture_length);
  }

  rider_generate_moves(vec_bishop_start,vec_bishop_end);
}

/* Generate moves for a Ship
 */
void ship_generate_moves(void)
{
  if (pawn_get_no_capture_length(trait[nbply],curr_generation->departure)>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_capture_move(dir_forward+dir_left);
    pawns_generate_capture_move(dir_forward+dir_right);
  }

  rider_generate_moves(vec_rook_start,vec_rook_end);
}
