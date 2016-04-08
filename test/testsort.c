#include"sort.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void showUsage(int argc,char* argv[])
{
  fprintf(stderr,"Usage: %s <size>\n",argc>0?argv[0]:"testsort");
  exit(1);
}

void cmpswap(void* v,size_t i,size_t j)
{
  int* d = v;
  if(d[i]>d[j]) { int x=d[i]; d[i]=d[j]; d[j]=x; }
}

int main(int argc,char* argv[])
{
  size_t n,i;
  if(argc<2) showUsage(argc,argv);
  if(sscanf(argv[1],"%zd",&n)!=1) showUsage(argc,argv);
  int *a = malloc(sizeof(int)*n);
  srand(time(0));
  for(i=0;i<n;++i) a[i]=rand();
  batcherSort(a,0,n,cmpswap);
  for(i=1;i<n;++i) if(a[i-1]>a[i]) break;
  if(i==n) printf("Checks passed\n");
  else printf("Checks failed\n");
  free(a);
  return 0;
}
