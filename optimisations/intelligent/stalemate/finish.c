#include "optimisations/intelligent/stalemate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "optimisations/intelligent/stalemate/intercept_checks_to_white.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "options/maxsolutions/maxsolutions.h"
#include "trace.h"

#include <assert.h>

void intelligent_stalemate_test_target_position(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,Black));
  assert(!echecc(nbply,White));
  if (!max_nr_solutions_found_in_phase())
  {
    if (!intelligent_stalemate_immobilise_black(n))
      intelligent_stalemate_deal_with_unused_pieces(n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
