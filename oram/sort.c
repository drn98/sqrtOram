#include"sort.h"

// Copied and modified from the Obliv-C source repository.
// Uses indices rather than actual data in cmpswap, since we are working on
// multiple arrays quite frequently.
void batcherSwap(void* data,
    size_t step,size_t st,size_t en,
    void (*cmpswap)(void*,size_t,size_t))
{
  size_t i;
  for(i=st;i+step<en;i+=step*2) cmpswap(data,i,i+step);
}
void batcherMerge
   (void* data,size_t step,size_t st,size_t mid,size_t en,
    void (*cmpswap)(void*,size_t,size_t))
{
  if(en-st<=step) return;
  batcherMerge(data,2*step,st,     mid,en,cmpswap);
  batcherMerge(data,2*step,st+step,mid,en,cmpswap);
  size_t half = (mid-st+step-1)/step;
  batcherSwap(data,step,st+step*(half%2==0),en,cmpswap);
}

void batcherSort(void* data,size_t st,size_t en,
    void (*cmpswap)(void*,size_t,size_t))
{
  if(en<=1+st) return;
  size_t mid = (st+en)/2;
  batcherSort(data,st,mid,cmpswap);
  batcherSort(data,mid,en,cmpswap);
  batcherMerge(data,1,st,mid,en,cmpswap);
}
