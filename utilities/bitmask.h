#if !defined(UTILITIES_BITMASK_H)
#define UTILITIES_BITMASK_H

/* Functionality for bitmask fiddling
 */

#define BIT(pos)                (1<<(pos))
#define TSTFLAG(bits,pos)       (((bits)&BIT(pos))!=0)
#define CLRFLAG(bits,pos)       ((bits)&= ~BIT(pos))
#define SETFLAG(bits,pos)       ((bits)|= BIT(pos))
#define CLEARFL(bits)           ((bits)=0)

#define TSTFLAGMASK(bits,mask)  ((bits)&(mask))
#define CLRFLAGMASK(bits,mask)  ((bits) &= ~(mask))
#define SETFLAGMASK(bits,mask)  ((bits) |= (mask))

#endif
