#include<stdio.h>
#include"timecopy.h"
#include<obliv.h>
#include"util.h"

int main(int argc,char* argv[])
{
  if(argc<6)
  {
    fprintf(stderr,"Usage: %s <remote-server|--> <port> <yao|share> <batch> <round>\n",
            argv[0]);
    return 1;
  }
  ProtocolDesc pd;
  TimeCopyIO io;
  io.useShare = (argv[3][0]!='y');
  sscanf(argv[4],"%d",&io.batchsz);
  sscanf(argv[5],"%d",&io.rounds);
  const char* server=argv[1];
  connectOrDie(&pd,server,argv[2]);
  setCurrentParty(&pd,strcmp(server,"--")==0?1:2);
  execYaoProtocol(&pd,timeCopy,&io);

}
