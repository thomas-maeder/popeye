#include "pieces/walks/pawns/combined.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a Dragon
 * @param sq_departure common departure square of the generated moves
 */
void dragon_generate_moves(square sq_departure)
{
  pawn_generate_moves(sq_departure);
  leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
}

/* Generate moves for a Gryphon
 * @param sq_departure common departure square of the generated moves
 */
void gryphon_generate_moves(square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],sq_departure);

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
  }

  rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
}

/* Generate moves for a Ship
 * @param sq_departure common departure square of the generated moves
 */
void ship_generate_moves(square sq_departure)
{
  if (pawn_get_no_capture_length(trait[nbply],sq_departure)>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_capture_move(sq_departure,dir_forward+dir_left);
    pawns_generate_capture_move(sq_departure,dir_forward+dir_right);
  }

  rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
}
