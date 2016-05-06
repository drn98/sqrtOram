#pragma once
#include<obliv.h>

void connectOrDie(ProtocolDesc* pd,const char* server,const char* port);
double wallClock();

int cmdFindFirst(const char* search,const char* alts[],int n);
int cmdFindFirstPrefix(const char* search,const char* alts[],int n);

typedef enum
{ oramTypeNone=-1,
  oramTypeLin,
  oramTypeSqrt,
  oramTypeCkt
} OramType;

static inline OramType
oramTypeFromString(const char* s)
{ const char* types[] = {"linear","sqrt","ckt"};
  return cmdFindFirstPrefix(s,types,sizeof(types)/sizeof(*types));
}
