#pragma once

typedef struct
{
  int *data,n;
  bool *isdata,*iserase;
  int outBlank,outData;
} PropagateArgs;

// Both parties should allocate arrays for result
void testPropagate(void* args);
