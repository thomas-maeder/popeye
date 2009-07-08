#if !defined(MAXMEM_H)
#define MAXMEM_H

/* Initialize maxmem module.
 * To be called before any other function of this module.
 */
void initMaxmem(void);

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
void readMaxmem(char const *commandLineValue);

/* Allocate memory for the hash table, based on the -maxmem command
 * line value (if any) and information retrieved from the operating
 * system.
 */
void dimensionHashtable(void);

/* Retrieve a human-readable indication of the maximal amount of
 * memory used by the hash table.
 * Only invoke after dimensionHashtable().
 * @return maximal amount of hash table memory, in human readable format
 */
char const *maxmemString(void);

/* Make a guess for a reasonable amount of memory for the hashtable.
 * Implemented separately for each platform.
 * Invoked by maxmem module; don't invoke directly.
 * @return number of kilo-bytes to be used
 */
unsigned long guessReasonableMaxmemory(void);

#endif
