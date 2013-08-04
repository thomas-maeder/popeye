#include "pieces/walks/radial.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void genleapleap(vec_index_type kanf, vec_index_type kend,
                        int hurdletype, boolean leaf)
{
  vec_index_type k;

  for (k= kanf; k<=kend; ++k)
  {
    square const sq_hurdle = curr_generation->departure+vec[k];
    if ((hurdletype==0 && piece_belongs_to_opponent(sq_hurdle))
        || (hurdletype==1 && get_walk_of_piece_on_square(sq_hurdle)>=King))
    {
      vec_index_type k1;
      for (k1= kanf; k1 <= kend; k1++)
      {
        curr_generation->arrival = (leaf ? curr_generation->departure : sq_hurdle) + vec[k1];
        if (curr_generation->arrival!=sq_hurdle
            && (is_square_empty(curr_generation->arrival)
                || piece_belongs_to_opponent(curr_generation->arrival)))
          push_move();
      }
    }
  }
}

static void genqlinesradial(int hurdletype, boolean leaf)
{
  numecoup const save_current_move = current_move[nbply]-1;

  genleapleap(vec_rook_start, vec_rook_end, hurdletype, leaf);
  genleapleap(vec_dabbaba_start, vec_dabbaba_end, hurdletype, leaf);
  genleapleap(vec_leap03_start, vec_leap03_end, hurdletype, leaf);
  genleapleap(vec_leap04_start, vec_leap04_end, hurdletype, leaf);
  genleapleap(vec_leap05_start, vec_leap05_end, hurdletype, leaf);
  genleapleap(vec_leap06_start, vec_leap06_end, hurdletype, leaf);
  genleapleap(vec_leap07_start, vec_leap07_end, hurdletype, leaf);
  genleapleap(vec_bishop_start, vec_bishop_end, hurdletype, leaf);
  genleapleap(vec_alfil_start, vec_alfil_end, hurdletype, leaf);
  genleapleap(vec_leap33_start, vec_leap33_end, hurdletype, leaf);
  genleapleap(vec_leap44_start, vec_leap44_end, hurdletype, leaf);
  genleapleap(vec_leap55_start, vec_leap55_end, hurdletype, leaf);
  genleapleap(vec_leap66_start, vec_leap66_end, hurdletype, leaf);
  genleapleap(vec_leap77_start, vec_leap77_end, hurdletype, leaf);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void genradial(int hurdletype, boolean leaf)
{
  numecoup const save_current_move = current_move[nbply]-1;

  genleapleap(vec_rook_start, vec_rook_end, hurdletype, leaf);
  genleapleap(vec_dabbaba_start, vec_dabbaba_end, hurdletype, leaf);
  genleapleap(vec_leap03_start, vec_leap03_end, hurdletype, leaf);
  genleapleap(vec_leap04_start, vec_leap04_end, hurdletype, leaf);
  genleapleap(vec_bucephale_start, vec_bucephale_end, hurdletype, leaf);
  genleapleap(vec_leap06_start, vec_leap06_end, hurdletype, leaf);
  genleapleap(vec_leap07_start, vec_leap07_end, hurdletype, leaf);
  genleapleap(vec_bishop_start, vec_bishop_end, hurdletype, leaf);
  genleapleap(vec_knight_start, vec_knight_end, hurdletype, leaf);
  genleapleap(vec_chameau_start, vec_chameau_end, hurdletype, leaf);
  genleapleap(vec_girafe_start, vec_girafe_end, hurdletype, leaf);
  genleapleap(vec_leap15_start, vec_leap15_end, hurdletype, leaf);
  genleapleap(vec_leap16_start, vec_leap16_end, hurdletype, leaf);
  genleapleap(vec_rccinq_start, vec_rccinq_end, hurdletype, leaf);
  genleapleap(vec_alfil_start, vec_alfil_end, hurdletype, leaf);
  genleapleap(vec_zebre_start, vec_zebre_end, hurdletype, leaf);
  genleapleap(vec_leap24_start, vec_leap24_end, hurdletype, leaf);
  genleapleap(vec_leap25_start, vec_leap25_end, hurdletype, leaf);
  genleapleap(vec_leap26_start, vec_leap26_end, hurdletype, leaf);
  genleapleap(vec_leap27_start, vec_leap27_end, hurdletype, leaf);
  genleapleap(vec_leap33_start, vec_leap33_end, hurdletype, leaf);
  genleapleap(vec_leap35_start, vec_leap35_end, hurdletype, leaf);
  genleapleap(vec_leap36_start, vec_leap36_end, hurdletype, leaf);
  genleapleap(vec_leap37_start, vec_rccinq_end, hurdletype, leaf);
  genleapleap(vec_leap44_start, vec_leap44_end, hurdletype, leaf);
  genleapleap(vec_leap45_start, vec_leap45_end, hurdletype, leaf);
  genleapleap(vec_leap46_start, vec_leap46_end, hurdletype, leaf);
  genleapleap(vec_leap47_start, vec_leap47_end, hurdletype, leaf);
  genleapleap(vec_leap56_start, vec_leap56_end, hurdletype, leaf);
  genleapleap(vec_leap57_start, vec_leap57_end, hurdletype, leaf);
  genleapleap(vec_leap66_start, vec_leap66_end, hurdletype, leaf);
  genleapleap(vec_leap67_start, vec_leap67_end, hurdletype, leaf);
  genleapleap(vec_leap77_start, vec_leap77_end, hurdletype, leaf);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Generate moves for a radial knight
 */
void radialknight_generate_moves(void)
{
  genradial(0, false);
}

/* Generate moves for a radial knight
 */
void treehopper_generate_moves(void)
{
  genqlinesradial(1, false);
}

/* Generate moves for a radial knight
 */
void greater_treehopper_generate_moves(void)
{
  genradial(1, false);
}

/* Generate moves for a radial knight
 */
void leafhopper_generate_moves(void)
{
  genqlinesradial(1, true);
}

/* Generate moves for a radial knight
 */
void greater_leafhopper_generate_moves(void)
{
  genradial(1, true);
}
