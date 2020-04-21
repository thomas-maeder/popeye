#if !defined(POSITION_COLOR_H)
#define POSITION_COLOR_H

#include "position/color.enum.h"

#define COLOURFLAGS     (BIT(colour_black)+BIT(colour_white)+BIT(colour_neutral))
#define SETCOLOUR(a,b)  ((a)=((a)&~COLOURFLAGS)+((b)&COLOURFLAGS))

#endif
