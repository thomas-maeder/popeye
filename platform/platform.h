#if !defined(PLATFORM_PLATFORM_H)
#define PLATFORM_PLATFORM_H

/* Guess the "bitness" of the platform
 * @return 32 if we run on a 32bit platform etc.
 */
unsigned int platform_guess_bitness(void);

#endif
