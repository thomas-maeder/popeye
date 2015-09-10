#if !defined(INPUT_PLAINTEXT_PROBLEM_H)
#define INPUT_PLAINTEXT_PROBLEM_H

#include "input/plaintext/token.h"
#include "stipulation/stipulation.h"

extern char ActAuthor[256];
extern char ActOrigin[256];
extern char ActTitle[256];
extern char ActAward[256];
extern char ActStip[37];

/* Handle (read, solve, write) the current problem
 * @return the input token that ends the problem (NextProblem or EndProblem)
 */
char *input_plaintext_problem_handle(char *tok, slice_index start);

#endif
