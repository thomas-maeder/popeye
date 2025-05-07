#if !defined(DHT_VALUE_INCLUDED)
#define DHT_VALUE_INCLUDED

/* This is dhtvalue.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
#include <stdio.h>
#ifdef __cplusplus
#    if __cplusplus >= 201103L
#        include <cstdint>
#    else
#        include <climits>
#    endif
#    include <csignal>
#elif defined(__STDC_VERSION__)
#    if __STDC_VERSION__ >= 199901L
#        include <stdint.h>
#    else
#        include <limits.h>
#    endif
#    include <signal.h>
#endif

#if defined(FXF)
#include "fxf.h"
#define DHTVALUE_ALLOC(size, type)	fxfAlloc(size, type)
#define DHTVALUE_ALLOC_RAW(size, alignment)	fxfAllocRaw(size, alignment)
#define DHTVALUE_FREE(ptr, size)	fxfFree(ptr, size)
#else
#include <stdlib.h>
#define	DHTVALUE_ALLOC(size, type)	((type *) malloc(size))
#define DHTVALUE_ALLOC_RAW(size, alignment)	malloc(size)
#define DHTVALUE_FREE(ptr, size)	free(ptr)
#endif /*FXF*/

#if !defined(LOCAL)
#define LOCAL	static
#endif

#if !defined(NO_REGISTER)
#define REGISTER_SIMPLE
#define REGISTER_STRING
#define REGISTER_COMPACT
#define REGISTER_MEMORY
#define REGISTER_BCMEM
#endif /*NO_REGISTER*/

typedef enum {
	dhtOkStatus, dhtFailedStatus
} dhtStatus;

/* Here we define some value types used throughout the source */
typedef enum {
	dhtSimpleValue,
	dhtStringValue,
	dhtCompactMemoryValue,
	dhtMemoryValue,
	dhtBCMemValue,
	dhtUser1Value,
	dhtUser2Value,
	dhtUser3Value,
	dhtUser4Value,
	/* dhtNewValue */
	dhtValueTypeCnt
} dhtValueType;

extern char const *dhtValueTypeToString[dhtValueTypeCnt];

typedef union {
#if 0
// HACK: This is meant to be completely generic, but Popeye doesn't require such genericity.
#    ifdef __cplusplus
#        if __cplusplus >= 201103L
    ::std::uintmax_t unsigned_integer;
    ::std::intmax_t signed_integer;
#        elif defined(LLONG_MAX)
    unsigned long long int unsigned_integer;
    long long int signed_integer;
#        else
    unsigned long int unsigned_integer;
    long int signed_integer;
#        endif
     ::std::sig_atomic_t atomic_integer;
#    else
#        if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    uintmax_t unsigned_integer;
    intmax_t signed_integer;
#        else
#            if defined(LLONG_MAX)
    unsigned long long int unsigned_integer;
    long long int signed_integer;
#            else
    unsigned long int unsigned_integer;
    long int signed_integer;
#            endif
#        endif
    sig_atomic_t atomic_integer;
#    endif
    void (*function_pointer)(void);
#    ifdef DHTVALUE_NEEDS_FLOATING_POINT
    long double floating_point;
#    endif
#else
/* HACK: We'll use a simpler dhtValue for Popeye that only includes the types
   Popeye needs (as well as other types that shouldn't affect sizeof(dhtValue)
   or the required alignment).  This maybe reduces the total size, and it may
   also make the allocator happier when it comes to alignment requirements. */
    unsigned int unsigned_integer;
    int signed_integer;
#endif
#ifdef __cplusplus
    bool boolean;
#else
#    if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    _Bool boolean;
#    else
    int boolean; // What else?
#    endif
#endif
    const volatile void * object_pointer;
    char character;
    unsigned char buffer[1]; /* treat as having sizeof(dhtValue) elements */
} dhtValue;

typedef struct {
	dhtValue value;
} dhtKey;

typedef unsigned long dhtHashValue;

typedef struct {
	dhtHashValue	(*Hash)(dhtKey);
	int				(*Equal)(dhtKey, dhtKey);
	int				(*Dup)(dhtValue, dhtValue *); // should return 0 on success (and store the copied value at the second argument) and nonzero on error
	void			(*Free)(dhtValue);
	void			(*Dump)(dhtValue, FILE *);
} dhtValueProcedures;

#if defined(REGISTER_SIMPLE)
/* dhtSimple verifies equality by comparing unsigned_integer members;
   keys should be stored to unsigned_integer members, cast if needed */
extern dhtValueProcedures dhtSimpleProcs;
#endif /*REGISTER_SIMPLE*/
#if defined(REGISTER_STRING)
extern dhtValueProcedures dhtStringProcs;
#endif /*REGISTER_STRING*/
#if defined(REGISTER_COMPACT)
extern dhtValueProcedures dhtCompactMemoryProcs;
#endif /*REGISTER_COMPACT*/
#if defined(REGISTER_MEMORY)
extern dhtValueProcedures dhtMemoryProcs;
#endif /*REGISTER_MEMORY*/
#if defined(REGISTER_BCMEM)
extern dhtValueProcedures dhtBCMemoryProcs;
#endif /*REGISTER_BCMEM*/

extern dhtValueProcedures *dhtProcedures[dhtValueTypeCnt];

dhtStatus dhtRegisterValue(dhtValueType, char const *, dhtValueProcedures *);

#endif /*DHT_VALUE_INCLUDED*/
