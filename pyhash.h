/******************** MODIFICATIONS to pyhash.h **************************
**
** Date       Who  What
** 
** 1994            Original
** 
**************************** End of List ******************************/ 

#if !defined(PYHASH_H)
#define PYHASH_H

#include "DHT/dhtbcmem.h"
#include "py.h"
#include "pytable.h"

/* typedefs */
typedef enum {
	SerNoSucc,
	hash_help_insufficient_nr_half_moves,
	DirSucc,
	DirNoSucc,
    nr_hashwhat
} hashwhat;

typedef unsigned char byte;

enum
{
  hashbuf_length = 256
};

typedef union
{
    BCMemValue cmv;
    byte buffer[hashbuf_length];
} HashBuffer;

extern HashBuffer hashBuffers[maxply+1];

extern boolean isHashBufferValid[maxply+1];

void validateHashBuffer(void);

/* if (guard) invalidate hash buffer for ply nbply
 * (making guard a parameter prevents it from being evaluated
 * #if !defined(NDEBUG))
 * @param guard guard
 */
void invalidateHashBuffer(void);

/* exported functions */
void check_hash_assumptions(void);

void hash_reset_derivations(void);

void hash_slice_is_derived_from(slice_index derived, slice_index base);

/* Allocate memory for the hash table. If the requested amount of
 * memory isn't available, reduce the amount until allocation
 * succeeds. 
 * @param nr_kilos number of kilo-bytes to allocate
 * @return number of kilo-bytes actually allocated
 */
unsigned long allochash(unsigned long nr_kilos);

void inithash (void);
void closehash (void);
void HashStats(unsigned int level, char *trailer);

typedef unsigned int hash_value_type;

boolean inhash(slice_index si,
               hashwhat what,
               hash_value_type val);
void addtohash(slice_index si,
               hashwhat what,
               hash_value_type val);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

/* Allocate a STHelpHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STHelpHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_hashed_slice(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void hashed_help_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n);

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean hashed_help_must_starter_resign_hashed(slice_index si);


#endif
