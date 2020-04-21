#if !defined(DHTMEM_INCLUDED)
#define DHTMEM_INCLUDED
/* This is dhtmem.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */

/* This is nearly the same as the CompactMemVal
 * but you will need two malloc-calls to duplicate
 * this. But can be more efficient, if no data in the
 * hash-table is needed.
 */
typedef struct MemVal {
	unsigned long	Leng;
	unsigned char	*Data;
} MemVal;
#define NilMemVal	(MemVal *)0
#define NewMemVal	(MemVal *)fxfAlloc(sizeof(MemVal))
#define FreeMemVal(v)	fxfFree(v, sizeof(MemVal))
#define DeleteMemVal(v)	if (((MemVal const *)(v))!=NilMemVal) fxfFree(((MemVal const *)(v))->Data, ((MemVal const *)(v))->Leng), FreeMemVal(v)

#endif /*DHTMEM_INCLUDED*/
