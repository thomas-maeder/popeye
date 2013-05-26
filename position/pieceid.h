#if !defined(POSITION_PIECEID_H)
#define POSITION_PIECEID_H

#include "position/board.h"

enum
{
  NullPieceId = 0,
  MinPieceId = 1,
  MaxPieceId = 63
};

typedef unsigned int        PieceIdType;

extern PieceIdType currPieceId;

#define PieceIdOffset       (PieSpCount+1)
#define PieSpMask           ((1<<PieceIdOffset)-1)
#define PieceIdMask         (~PieSpMask)
#define SetPieceId(spec,id) ((spec) = ((id)<<PieceIdOffset) | ((spec)&PieSpMask))
#define GetPieceId(spec)    ((spec) >> PieceIdOffset)
#define ClearPieceId(spec)  SetPieceId(spec,NullPieceId)

extern square PiecePositionsInDiagram[MaxPieceId+1];

#define GetPositionInDiagram(spec)     PiecePositionsInDiagram[GetPieceId(spec)]
#define SavePositionInDiagram(spec,sq) (PiecePositionsInDiagram[GetPieceId(spec)] = (sq))
#define ClearPositionInDiagram(spec)   SavePositionInDiagram(spec,initsquare)

#endif
