/* This is dhtvalue.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
#include "dhtvalue.h"
#include "dht.h"

char const *dhtValueTypeToString[dhtValueTypeCnt]
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
;

dhtValueProcedures *dhtProcedures[dhtValueTypeCnt]
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
;

dhtStatus dhtRegisterValue(dhtValueType t, char const *ts, dhtValueProcedures *proc) {
    if (t >= dhtValueTypeCnt) {
    	sprintf(dhtError,
	  "dhtRegisterValue: dhtValueType > dhtValueTypeCount (=%d).",
	  dhtValueTypeCnt);
    	return dhtFailedStatus;
    }
    dhtProcedures[t]= proc;
    if (ts)
	dhtValueTypeToString[t]= ts;
    return dhtOkStatus;
}
