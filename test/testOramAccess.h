#pragma once
#include<stdbool.h>
#include<util.h>
typedef struct
{ int size;
  int *content,*indices,*outputs;
  OramType type;
  bool detailedOut;
} TestOramAccessIO;

void testOramAccess(void* vargs);
void exchangeInputs(TestOramAccessIO* args,int accn,int n);
