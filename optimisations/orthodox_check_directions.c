#include "optimisations/orthodox_check_directions.h"
#include "position/position.h"
#include "solving/pipe.h"

static numvec ortho_opt[4][2*(square_h8-square_a1)+1];

static numvec const * const check_dir_impl[4] = {
    ortho_opt[Queen-Queen]+(square_h8-square_a1),
    ortho_opt[Knight-Queen]+(square_h8-square_a1),
    ortho_opt[Rook-Queen]+(square_h8-square_a1),
    ortho_opt[Bishop-Queen]+(square_h8-square_a1)
};

numvec const * CheckDir(piece_walk_type t)
{
    return check_dir_impl[t-Queen];
}

static void InitCheckDir(void)
{
  vec_index_type i;
  unsigned int j;

  {
    STATIC_ASSERT(Queen<Rook, "Queen must be < Rook.");
    STATIC_ASSERT((Rook-Queen)<4, "Rook - Queen must be < 4.");
    STATIC_ASSERT(Queen<Bishop, "Queen must be < Bishop.");
    STATIC_ASSERT((Bishop-Queen)<4, "Bishop - Queen must be < 4.");
    STATIC_ASSERT(Queen<Knight, "Queen must be < Knight.");
    STATIC_ASSERT((Knight-Queen)<4, "Knight - Queen must < 4.");
  }

  for (i = 0; i<=2*(square_h8-square_a1); i++)
  {
    ortho_opt[Queen-Queen][i] = 0;
    ortho_opt[Rook-Queen][i] = 0;
    ortho_opt[Bishop-Queen][i] = 0;
    ortho_opt[Knight-Queen][i] = 0;
  }

  for (i = vec_knight_start; i <= vec_knight_end; i++)
    ortho_opt[Knight-Queen][(square_h8-square_a1)+vec[i]] = vec[i];

  for (i = vec_rook_start; i<=vec_rook_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Rook-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }

  for (i = vec_bishop_start; i<=vec_bishop_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Bishop-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }
}

void check_dir_initialiser_solve(slice_index si)
{
  InitCheckDir();
  pipe_solve_delegate(si);
}
