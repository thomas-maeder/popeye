#include "optimisations/intelligent/mate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "solving/legal_move_finder.h"
#include "stipulation/temporary_hacks.h"
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

      result = (echecc(nbply,Black)
                && slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution);

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

static void neutralise_guarding_pieces(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       unsigned int max_nr_allowed_captures_by_white,
                                       unsigned int max_nr_allowed_captures_by_black,
                                       stip_length_type n)
{
  square trouble = initsquare;
  square trto = initsquare;
#if !defined(NDEBUG)
  has_solution_type search_result;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  init_legal_move_finder();

#if !defined(NDEBUG)
  search_result =
#endif
  slice_has_solution(slices[temporary_hack_legal_move_finder[Black]].u.fork.fork);
  assert(search_result==has_solution);
  assert(legal_move_finder_departure!=initsquare);
  trouble = legal_move_finder_departure;
  trto = legal_move_finder_arrival;

  fini_legal_move_finder();

  intelligent_mate_pin_black_piece(nr_remaining_white_moves,
                                   nr_remaining_black_moves,
                                   max_nr_allowed_captures_by_white,
                                   max_nr_allowed_captures_by_black,
                                   n,
                                   trouble);

  if (is_rider(abs(e[trouble])))
  {
    int const dir = CheckDirQueen[trto-trouble];

    square sq;
    for (sq = trouble+dir; sq!=trto; sq+=dir)
      if (nr_reasons_for_staying_empty[sq]==0)
      {
        intelligent_mate_place_any_black_piece_on(nr_remaining_white_moves,
                                                  nr_remaining_black_moves,
                                                  max_nr_allowed_captures_by_white,
                                                  max_nr_allowed_captures_by_black,
                                                  n,
                                                  sq);
        intelligent_mate_place_any_white_piece_on(nr_remaining_white_moves,
                                                  nr_remaining_black_moves,
                                                  max_nr_allowed_captures_by_white,
                                                  max_nr_allowed_captures_by_black,
                                                  n,
                                                  sq);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_test_target_position(unsigned int nr_remaining_white_moves,
                                           unsigned int nr_remaining_black_moves,
                                           unsigned int max_nr_allowed_captures_by_white,
                                           unsigned int max_nr_allowed_captures_by_black,
                                           stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));
  if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
  {
    /* avoid duplicate test of the same target position (modulo redundant pieces
     * and unused white king) */
    if (!exists_redundant_white_piece()
        && !(white[index_of_king].usage==piece_is_unused
             && white[index_of_king].diagram_square!=square_e1
             && nr_remaining_white_moves==0))
      solve_target_position(n);
  }
  else
    neutralise_guarding_pieces(nr_remaining_white_moves,
                               nr_remaining_black_moves,
                               max_nr_allowed_captures_by_white,
                               max_nr_allowed_captures_by_black,
                               n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_finish(unsigned int nr_remaining_white_moves,
                             unsigned int nr_remaining_black_moves,
                             unsigned int max_nr_allowed_captures_by_white,
                             unsigned int max_nr_allowed_captures_by_black,
                             stip_length_type n,
                             unsigned int nr_of_checks_to_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_checks_to_white);
  TraceFunctionParamListEnd();

  if (nr_of_checks_to_white>0)
    intelligent_mate_intercept_checks(nr_remaining_white_moves,
                                      nr_remaining_black_moves,
                                      max_nr_allowed_captures_by_white,
                                      max_nr_allowed_captures_by_black,
                                      n);
  else
    intelligent_mate_test_target_position(nr_remaining_white_moves,
                                          nr_remaining_black_moves,
                                          max_nr_allowed_captures_by_white,
                                          max_nr_allowed_captures_by_black,
                                          n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
