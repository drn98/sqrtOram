#pragma once
#include<stdbool.h>

typedef struct
{ int rounds,batchsz;
  bool useShare;
} TimeCopyIO;

void timeCopy(void* varg);
