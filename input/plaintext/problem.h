#if !defined(INPUT_PLAINTEXT_PROBLEM_H)
#define INPUT_PLAINTEXT_PROBLEM_H

#include "stipulation/stipulation.h"

extern char ActAuthor[256];
extern char ActOrigin[256];
extern char ActTitle[256];
extern char ActAward[256];

/* Handle (read, solve, write) the current problem
 */
void input_plaintext_problem_handle(slice_index start);

#endif
