			/* PYDATA.C     */
/*************************** MODIFICATIONS *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91     SB
**
** 2008/02/25 SE   New piece type: Magic
**
**************************** INFORMATION END **************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(macintosh)        /* is always defined on macintosh's  SB */
#       define SEGMIO
#       include "platform/unix/mac.h"
#endif

#include "py.h"
#include "pyproc.h"
#define EXTERN
#include "pydata.h"

killer_state const null_killer_state = {
  false,
  {
    initsquare,
    initsquare,
    initsquare
  },
  initsquare
};

change_rec* colour_change_stack_limit = &colour_change_stack[colour_change_stack_size];
change_rec* push_colour_change_stack_limit = &push_colour_change_stack[push_colour_change_stack_size];

static int comparePieceNames(void const * param1, void const * param2)
{
  PieceChar const *name1 = (PieceChar const *)param1;
  PieceChar const *name2 = (PieceChar const *)param2;
  int result = (*name1)[0]-(*name2)[0];
  if (result==0)
    result = (*name1)[1]-(*name2)[1];
  return result;
}

static void enforce_piecename_uniqueness_one_language(Language language)
{
  PieNam name_index;
  PieTable piece_names_sorted;
  unsigned int nr_names = sizeof piece_names_sorted / sizeof piece_names_sorted[0];

  memcpy(piece_names_sorted, PieNamString[language], sizeof piece_names_sorted);
  qsort(piece_names_sorted,
        nr_names,
        sizeof piece_names_sorted[0],
        &comparePieceNames);

  /* hunter names are initialised to two blanks; qsort moves them to the
   * beginning */
  for (name_index = maxnrhuntertypes; name_index<nr_names-1; ++name_index)
  {
    assert(piece_names_sorted[name_index][0]!=piece_names_sorted[name_index+1][0]
           || piece_names_sorted[name_index][1]!=piece_names_sorted[name_index+1][1]);
  }
}

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void)
{
  Language language;

  for (language = 0; language!=LanguageCount; ++language)
    enforce_piecename_uniqueness_one_language(language);
}
