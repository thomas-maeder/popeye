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
#else
#define	fxfAlloc(x)		malloc(x)
#define fxfFree(x,n)		free(x)
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
#ifdef __cplusplus
#    if __cplusplus >= 201103L
    ::std::uintmax_t unsigned_integer;
    ::std::intmax_t signed_integer;
#    elif defined(LLONG_MAX)
    unsigned long long int unsigned_integer;
    long long int signed_integer;
#    else
    unsigned long int unsigned_integer;
    long int signed_integer;
#    endif
    bool boolean;
     ::std::sig_atomic_t atomic_integer;
#else
#    if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    uintmax_t unsigned_integer;
    intmax_t signed_integer;
    _Bool boolean;
#    else
#        if defined(LLONG_MAX)
    unsigned long long int unsigned_integer;
    long long int signed_integer;
#        else
    unsigned long int unsigned_integer;
    long int signed_integer;
#        endif
    int boolean; // What else?
#    endif
    sig_atomic_t atomic_integer;
#endif
    const volatile void * object_pointer;
    char character;
    void (*function_pointer)(void);
#ifdef DHTVALUE_NEEDS_FLOATING_POINT
    long double floating_point;
#endif
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
