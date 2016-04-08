#include<obliv.h>
#include<stdio.h>
#include"util.h" // This is just stupid. Fix this to not include path
                 // to oblivcc test folder in Makefile

void testShuffle(void*);

void showUsage(const char* exec)
{
  fprintf(stderr,"Parameters missing\n");
  fprintf(stderr,"Usage: %s <remote-server|--> <port> n\n",exec);
  exit(1);
}
// Compile with -DPERM_STORE_YAO for pure yao protocol, no shares
// Compile with -DUSE_SHARES for pure share shuffling
// Use both for pure shares, along with yao-based index handling
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  int i,me,n;
  if(argc<4) showUsage(argv[0]);
  else sscanf(argv[3],"%d",&n);
  connectOrDie(&pd,argv[1],argv[2]);
  setCurrentParty(&pd,strcmp(argv[1],"--")==0?1:2);
  execYaoProtocol(&pd,testShuffle,&n);
  cleanupProtocol(&pd);
  printf(n?"Checks passed\n":"Checks failed\n");
  return 0;
}
