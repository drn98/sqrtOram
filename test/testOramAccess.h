#pragma once
typedef struct
{ int size;
  int *content,*indices,*outputs;
  enum { oramTypeLin,oramTypeSqrt,oramTypeCkt } type;
} TestOramAccessIO;

void testOramAccess(void* vargs);
