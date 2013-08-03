#include "pieces/walks/pawns/berolina.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/pawn.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a Berolina pawn
 */
void berolina_pawn_generate_moves(void)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],
                                                                    curr_generation->departure);

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    pawns_generate_capture_move(dir_forward);
    pawns_generate_nocapture_moves(dir_forward+dir_left,no_capture_length);
    pawns_generate_nocapture_moves(dir_forward+dir_right,no_capture_length);
  }
}
