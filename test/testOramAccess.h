#pragma once
typedef struct
{ int size;
  int *content,*indices,*outputs;
  enum { oramTypeLin,oramTypeSqrt } type;
} TestOramAccessIO;

void testOramAccess(void* vargs);
