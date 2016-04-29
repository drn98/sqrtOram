#pragma once
#include<stdbool.h>
typedef struct
{ int size;
  int *content,*indices,*outputs;
  enum { oramTypeLin,oramTypeSqrt,oramTypeCkt } type;
  bool detailedOut;
} TestOramAccessIO;

void testOramAccess(void* vargs);
void exchangeInputs(TestOramAccessIO* args,int accn,int n);
