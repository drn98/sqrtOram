#pragma once
typedef struct
{ int size,indexn,rep;
  int *content,*indices,*outputs;
} TestSqrtAccessIO;

void testSqrtAccess(void* vargs);
