#include"benchSqrtAccess.h"
#include"util.h"
#include<stdio.h>
#include<stdlib.h>
#include<obliv.h>

void showUsage(const char* exec)
{
  fprintf(stderr,"Usage: %s -- <port> <oram-size> <block-size>\n"
                 "   or: %s <remote-addr> <port> <oram-size> <access-count>\n",
                 exec,exec);
  exit(1);
}
int main(int argc,char* argv[])
{
  ProtocolDesc pd;
  TestSqrtAccessIO io;
  int i,me;
  if(argc<4) showUsage(argv[0]);
  else if(strcmp(argv[1],"--")==0)
  { if(argc<5) showUsage(argv[0]);
    if(sscanf(argv[4],"%d",&io.rep)!=1) showUsage(argv[0]);
    if(sscanf(argv[3],"%d",&io.size)!=1) showUsage(argv[0]);
    io.content=malloc(sizeof(int)*io.size);
    for(i=0;i<io.size;++i) io.content[i]=i+2;
    me=1;
  }else
  { if(sscanf(argv[4],"%d",&io.indexn)!=1) showUsage(argv[0]);
    if(sscanf(argv[3],"%d",&io.size)!=1) showUsage(argv[0]);
    io.indices=malloc(sizeof(int)*io.indexn);
    for(i=0;i<io.indexn;++i) io.indices[i]=i%io.size;
    me=2;
  }
  connectOrDie(&pd,argv[1],argv[2]);
  setCurrentParty(&pd,me);
  execYaoProtocol(&pd,testSqrtAccess,&io);
  if(me==2) free(io.indices);
  for(i=0;i<io.indexn;++i) if(io.outputs[i]!=i%io.size+2) break;
  if(i<io.indexn)
  { for(i=0;i<io.indexn;++i) printf("%d ",io.outputs[i]);
    printf("\n");
  }
  free(io.outputs);
  if(me==1) free(io.content);
  cleanupProtocol(&pd);
  return 0;
}
