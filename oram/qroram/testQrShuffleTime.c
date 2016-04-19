#include"qroram.h"
#include<stdlib.h>
#include<stdio.h>

int main(int argc,char* argv[])
{
  int size;
  if(argc<2 || sscanf(argv[1],"%d",&size)!=1)
  { fprintf(stderr,"Usage: %s <size>\n",argv[0]);
    return 1;
  }
  QroramTable* tbl = qroramTable(size);
  int *schedT,*schedD;
  int i,schedSize = qroramShuffTimes(tbl,&schedT,&schedD,size);
  for(i=0;i<schedSize;++i)
    printf("%d %d\n",schedT[i],schedD[i]);
  free(schedT);
  free(schedD);
  qroramTableRelease(tbl);
  return 0;
}

