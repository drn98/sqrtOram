#pragma once

typedef struct
{ int *data,*indices,*outputs;
  int n,axc;
} SearchIO;

void searchProto(void* args);
