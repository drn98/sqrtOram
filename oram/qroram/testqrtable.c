#include"qroram.h"
#include<stdlib.h>
#include<stdio.h>

void stormOut(const char* myname)
{
  fprintf(stderr,"Usage: %s <oram-sizes ...>\n",myname);
  exit(1);
}

int parseParams(int** sizes,int argc,char* argv[])
{
  int i,rv=argc-1;
  *sizes=malloc(sizeof(int)*rv);
  for(i=0;i<rv;++i)
    if((sscanf(argv[i+1],"%d",(*sizes)+i))!=1)
    { free(*sizes);
      stormOut(argv[0]);
    }
  return rv;
}

int max(const int x[],int n)
{
  int i,rv=x[0];
  for(i=1;i<n;++i) if(rv<x[i]) rv=x[i];
  return rv;
}

int main(int argc,char* argv[])
{
  if(argc<2) stormOut(argv[0]);
  int i,*sizes;
  int inputCount = parseParams(&sizes,argc,argv);
  int maxsize = max(sizes,inputCount);
  QroramTable* tbl = qroramTable(maxsize);
  printf("Periods:");
  for(i=0;i<inputCount;++i) 
    printf(" %d",qroramBestPeriod(tbl,sizes[i]));
  printf("\n");
  //printf("Debug:\n");
  //qroramDebugInfo(tbl);
  qroramTableRelease(tbl);
  free(sizes);
  return 0;
}

