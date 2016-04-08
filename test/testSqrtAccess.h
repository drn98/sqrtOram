#pragma once
typedef struct
{ int size;
  int *content,*indices,*outputs;
} TestSqrtAccessIO;

void testSqrtAccess(void* vargs);
