#include"util.h"
#include<stdio.h>
#include<time.h>

double wallClock()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

void connectOrDie(ProtocolDesc* pd,const char* server,const char* port)
{
  if(strcmp("--",server)==0) // I *am* the server
  { if(protocolAcceptTcp2P(pd,port)!=0)
    { fprintf(stderr,"TCP accept failed\n");
      exit(1);
    }
    setCurrentParty(pd,1); 
  }else
  { if(protocolConnectTcp2P(pd,server,port)!=0)
    { fprintf(stderr,"TCP connect failed\n");
      exit(1);
    }
    setCurrentParty(pd,2);
  }
}
