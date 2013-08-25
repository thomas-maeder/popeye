#if !defined(INPUT_PLAINTEXT_PROBLEM_H)
#define INPUT_PLAINTEXT_PROBLEM_H

extern char ActAuthor[256];
extern char ActOrigin[256];
extern char ActTitle[256];
extern char ActAward[256];
extern char ActStip[37];

/* Iterate over the problems read from standard input or the input
 * file indicated in the command line options
 */
void iterate_problems(void);

#endif
