#include "position/topology.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

BoardTopology board_topology = TOPOLOGY_STANDARD;

/* Twist phases for non-orientable topologies (Phase 3+).
 * Unused for Phase 1 (vertical cylinder) but declared for forward compatibility. */
static int twist_phase_file = 0;
static int twist_phase_rank = 0;

void topology_reset_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  twist_phase_file = 0;
  twist_phase_rank = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Decompose a direction vector into file and rank deltas.
 *
 * The padded board uses onerow (=24) as the row stride. A vector v
 * encodes df + dr*onerow. However, C's % and / operators on the raw
 * sum sq+v do NOT correctly separate df and dr when df is negative,
 * because the "borrow" between file and rank distorts the result.
 *
 * Example: vec=22 (=24-2) means 1 rank up, 2 files left.
 *   Naive: 22%24=22, 22/24=0 → wrong (df=22, dr=0)
 *   Correct: dr=1, df=-2
 *
 * Solution: round v/onerow to the nearest integer, then compute
 * df = v - dr*onerow.
 */
static void decompose_vector(numvec v, int *df, int *dr)
{
  int r;
  if (v >= 0)
    r = (v + onerow/2) / onerow;
  else
    r = (v - onerow/2) / onerow;
  *dr = r;
  *df = v - r * onerow;
}

square topology_step(square sq, numvec dir)
{
  square result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  assert(board_topology != TOPOLOGY_STANDARD);

  {
    /* Extract padded file and rank from sq */
    int const f = sq % onerow;
    int const r = sq / onerow;
    int df, dr;

    decompose_vector(dir, &df, &dr);

    {
      int nf = f + df;
      int nr = r + dr;

      switch (board_topology)
      {
        case TOPOLOGY_VERTICAL_CYLINDER:
          /* Wrap file: a-file (file 0) connects to h-file (file 7) */
          while (nf < (int)left_file)
            nf += nr_files_on_board;
          while (nf > (int)right_file)
            nf -= nr_files_on_board;
          /* Rank is NOT wrapped — top/bottom edges remain */
          break;

        default:
          assert(0);
          break;
      }

      /* Check if the (possibly wrapped) position is on the board */
      if (nf >= (int)left_file && nf <= (int)right_file
          && nr >= (int)bottom_row && nr <= (int)top_row)
        result = nr * onerow + nf;
      else
        result = square_a1 - 1;  /* sentinel: a blocked slack square */
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}
