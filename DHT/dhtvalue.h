#if !defined(DHT_VALUE_INCLUDED)
#define DHT_VALUE_INCLUDED

/* This is dhtvalue.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */
#include <stdio.h>

#if defined(FXF)
#include "fxf.h"
#else
#define	fxfAlloc(x)		malloc(x)
#define fxfFree(x,n)		free(x)
#endif /*FXF*/

#if !defined(LOCAL)
#define LOCAL	static
#endif

#if defined(GDATA)
#define DATA  /* TODO: Maybe we can/should put "static" here. */
#else
#define DATA	extern
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

DATA char const *dhtValueTypeToString[dhtValueTypeCnt]
#if defined(GDATA)
	= { "dhtSimpleValue",
	    "dhtStringValue",
	    "dhtCompactMemoryValue",
	    "dhtMemoryValue",
	    "dhtByteCountedMemoryValue",
	    "dhtUser1Value",
	    "dhtUser2Value",
	    "dhtUser3Value",
	    "dhtUser4Value"
	    /* , "dhtNewValue" */
	  }
#endif
;

typedef void *dhtValue;
typedef void const *dhtConstValue;
typedef unsigned long dhtHashValue;

typedef struct {
  dhtHashValue (*Hash)(dhtConstValue);
	int		(*Equal)(dhtConstValue, dhtConstValue);
	dhtConstValue	(*Dup)(dhtConstValue);
	void		(*Free)(dhtValue);
	void		(*Dump)(dhtConstValue, FILE *);
} dhtValueProcedures;

#if defined(REGISTER_SIMPLE)
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

DATA dhtValueProcedures *dhtProcedures[dhtValueTypeCnt]
#if defined(GDATA)
	= {
#if defined(REGISTER_SIMPLE)
		&dhtSimpleProcs,
#endif /*REGISTER_SIMPLE*/
#if defined(REGISTER_STRING)
		&dhtStringProcs,
#endif /*REGISTER_STRING*/
#if defined(REGISTER_COMPACT)
		&dhtCompactMemoryProcs,
#endif /*REGISTER_COMPACT*/
#if defined(REGISTER_MEMORY)
		&dhtMemoryProcs,
#endif /*REGISTER_MEMORY*/
#if defined(REGISTER_BCMEM)
		&dhtBCMemoryProcs,
#endif /*REGISTER_BCMEM*/
	  }
#endif /*GDATA*/
;

dhtStatus dhtRegisterValue(dhtValueType, char const *, dhtValueProcedures *);

#endif /*DHT_VALUE_INCLUDED*/
