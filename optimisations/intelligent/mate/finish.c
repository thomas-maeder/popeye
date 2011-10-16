#include "optimisations/intelligent/mate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "solving/legal_move_finder.h"
#include "optimisations/intelligent/mate/place_white_piece.h"
#include "optimisations/intelligent/mate/place_black_piece.h"
#include "optimisations/intelligent/mate/pin_black_piece.h"
#include "optimisations/intelligent/mate/intercept_checks.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static boolean exists_redundant_white_piece(void)
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* check for redundant white pieces */
  for (bnp = boardnum; !result && *bnp!=initsquare; bnp++)
  {
    square const sq = *bnp;
    if (sq!=king_square[White] && e[sq]>obs)
    {
      piece const p = e[sq];
      Flags const sp = spec[sq];

      /* remove piece */
      e[sq] = vide;
      spec[sq] = EmptySpec;

      result = slice_has_solution(slices[current_start_slice].u.intelligent_mate_filter.goal_tester_fork)==has_solution;

      /* restore piece */
      e[sq] = p;
      spec[sq] = sp;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean neutralise_guarding_pieces(stip_length_type n)
{
  square trouble;
  square trto;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  init_legal_move_finder();
  result = slice_has_solution(slices[current_start_slice].u.intelligent_mate_filter.fork)==has_solution;
  trouble = legal_move_finder_departure;
  trto = legal_move_finder_arrival;
  fini_legal_move_finder();

  if (result)
  {
    assert(trouble!=initsquare);

    intelligent_mate_pin_black_piece(n,trouble);

    if (is_rider(abs(e[trouble])))
    {
      int const dir = CheckDirQueen[trto-trouble];

      square sq;
      for (sq = trouble+dir; sq!=trto; sq+=dir)
        if (nr_reasons_for_staying_empty[sq]==0)
        {
          intelligent_mate_place_any_black_piece_on(n,sq);
          intelligent_mate_place_any_white_piece_on(n,sq);
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void fix_white_king_on_diagram_square(stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0
      && !would_white_king_guard_from(king_diagram_square))
  {
    king_square[White] = king_diagram_square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    intelligent_mate_finish(n,echecc(nbply,White));

    white[index_of_king].usage = piece_is_unused;
    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_test_target_position(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));
  if (!neutralise_guarding_pieces(n))
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].diagram_square!=square_e1
        && Nr_remaining_white_moves==0)
      fix_white_king_on_diagram_square(n);
    else if (!exists_redundant_white_piece())
    {
      /* Nail white king to diagram square if no white move remains; we can't do
       * this with the other white or black pieces because they might be
       * captured in the solution */
        solve_target_position(n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_finish(stip_length_type n,
                             unsigned int nr_of_checks_to_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_checks_to_white);
  TraceFunctionParamListEnd();

  if (nr_of_checks_to_white>0)
    intelligent_mate_intercept_checks(n);
  else
    intelligent_mate_test_target_position(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
