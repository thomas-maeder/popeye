/* This is dhtvalue.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */
#define GDATA
#include "dhtvalue.h"
#undef	GDATA
#include "dht.h"

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
