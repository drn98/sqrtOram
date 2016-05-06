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

int
cmdFindFirst(const char* search,const char* alts[],int n)
{
  int i;
  for(i=0;i<n;++i) if(strcmp(search,alts[i])==0) return i;
  return -1;
}

bool
isPrefix(const char* prefix,const char* str)
{ int i;
  for(i=0;;++i)
  { if(str[i]==0) return prefix[i]==0;
    if(prefix[i]==0) return true;
    if(str[i]!=prefix[i]) return false;
  }
}
bool
checkPrefix(const char* alts[],int n)
{ int i,j;
  for(i=0;i<n;++i) for(j=0;j<n;++j) if(i!=j) if(isPrefix(alts[i],alts[j]))
    return false;
  return true;
}

int
cmdFindFirstPrefix(const char* search,const char* alts[],int n)
{
  assert(!checkPrefix(alts,n));
  int i,res=-1,n;
  for(i=0;i<n;++i) if(isPrefix(search,alts[i]))
  { if(res==-1) res=i;
    else return -1;
  }
  return i;
}

