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
#include "pydirect.h"

/* typedefs */
typedef enum {
	hash_series_insufficient_nr_half_moves,
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

/* Allocate a STDirectHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STDirectHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_directhashed_slice(slice_index si);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type direct_hashed_solve_continuations_in_n(table continuations,
                                                        slice_index si,
                                                        stip_length_type n);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type direct_hashed_solve_threats(table threats,
                                             slice_index si,
                                             stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_hashed_are_threats_refuted_in_n(table threats,
                                               stip_length_type len_threat,
                                               slice_index si,
                                               stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type direct_hashed_has_solution_in_n(slice_index si,
                                                  stip_length_type n);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_hashed_solve(slice_index si);

/* Allocate a STHelpHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STHelpHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_helphashed_slice(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_help_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n);

/* Allocate a STSeriesHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STSeriesHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_serieshashed_slice(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_series_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_series_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_series_solve_continuations_in_n(table continuations,
                                            slice_index si,
                                            stip_length_type n);

#endif
