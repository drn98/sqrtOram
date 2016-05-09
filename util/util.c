#include"util.h"
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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
cmdIsPrefix(const char* prefix,const char* str)
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
  for(i=0;i<n;++i) for(j=0;j<n;++j) if(i!=j) if(cmdIsPrefix(alts[i],alts[j]))
    return false;
  return true;
}

int
cmdFindFirstPrefix(const char* search,const char* alts[],int n)
{
  assert(!checkPrefix(alts,n));
  int i,res=-1;
  for(i=0;i<n;++i) if(cmdIsPrefix(search,alts[i]))
  { if(res==-1) res=i;
    else return -1;
  }
  return i;
}

void
cmdErrExit(const char cmdUsage[])
{
  fprintf(stderr,"%s\n",cmdUsage);
  exit(-1);
}
void
cmdUnknownOptionExit(const char opt[])
{
  fprintf(stderr,"Unknown option %s\n",opt);
  exit(-1);
}

int cmdIndex = 1;
struct CmdNetSpec cmdNetSpec = {};
bool cmdShowResult = true;

bool cmdParseMode(CmdParseState* cargs)
{
  const char* modes[] = {"test","bench"};
  int i = cmdFindFirst(cargs->argv[cargs->cmdIndex],modes,2);
  if(i==-1) return true;
  if(cmdMode!=-1 && cmdMode!=i) 
  { fprintf(stderr,"Choose either 'test' or 'bench', not both\n");
    return false;
  }
  cmdMode=i;
  if(i==cmdModeBench) cmdShowResult=false;
  cargs->cmdIndex++;
  return true;
}
static bool numericString(const char* s)
{ while(*s) if(!isdigit(*(s++))) return false;
  return true;
}
void cmdParseCheckNumeric(const char* s)
{ if(!numericString(s))
  { fprintf(stderr,"Expecting integer, not '%s'.\n",s);
    exit(-1);
  }
}
bool cmdParseNetwork(CmdParseState* cargs)
{
  if(cmdNetDone()) return true;
  const char* a = cargs->argv[cargs->cmdIndex];
  char* p=strchr(a,':');
  if(!p) return true;
  if(!numericString(p+1)) return true;
  cmdNetSpec.port=p+1;
  *p='\0';
  cmdNetSpec.server=a;
  cargs->cmdIndex++;
  return true;
}
bool cmdParseShowResult(CmdParseState* cargs)
{
  const char* a = cargs->argv[cargs->cmdIndex];
  const char* opt[] = {"--showResult","--hideResult","+o","-o"};
  int i = cmdFindFirst(a,opt,sizeof(opt)/sizeof(*opt));
  if(i==-1) return true;
  if(cmdMode!=cmdModeTest) 
  { fprintf(stderr,"Option '%s' can only be used with 'search test'.i\n",a);
    return false;
  }
  cmdShowResult=(i%2==0);
  cargs->cmdIndex++;
  return true;
}

OramType cmdOramType = oramTypeNone;
bool cmdOramTypeDone() { return cmdOramType!=oramTypeNone; }

// Returns NULL on mismatch, pointer to argument on success.
const char*
cmdParseSingleArg(CmdParseState* cargs,char shortopt,const char* longopt)
{
  const char* a = cargs->argv[cargs->cmdIndex];
  if(cmdIsPrefix("--",a)) // --longopt case
  { int n=strlen(longopt);
    if(strncmp(longopt,a+2,n)!=0 || a[n+2]!='=') return 0;
    cargs->cmdIndex++;
    return a+n+3;
  }
  else if(a[0]=='-' && a[1]==shortopt)
  { if(a[2]!='\0') { cargs->cmdIndex++; return a+2; }
    else if(cargs->cmdIndex+1==cargs->argc)
    { fprintf(stderr,"Error: argument missing for -%c\n",shortopt);
      exit(-1);
    }else { cargs->cmdIndex+=2; return cargs->argv[cmdIndex-1]; }
  }
  return 0;
}
bool
cmdParseSingleInt(CmdParseState* cargs,char shortopt,const char* longopt,
                  int* dest,int init)
{
  const char* a = cmdParseSingleArg(cargs,shortopt,longopt);
  if(!a) return true;
  int x;
  if(sscanf(a,"%d",&x)!=1)
  { fprintf(stderr,"Integer expected, not '%s'\n",a);
    exit(-1);
  }
  if(*dest==init || *dest==x) *dest=x;
  else
  { fprintf(stderr,"Conflicting values for --%s\n",longopt);
    exit(-1);
  }
}

bool cmdParseOramType(CmdParseState* cargs)
{
  const char* targ = cmdParseSingleArg(cargs,'t',"oramtype");
  if(!targ) return true;
  OramType t = oramTypeFromString(targ);
  if(t==oramTypeNone)
  { fprintf(stderr,"Invalid oram type %s\n",targ);
    return false;
  }
  if(cmdOramType!=oramTypeNone && cmdOramType!=t)
  { fprintf(stderr,"Conflicting oram types specified\n");
    return false;
  }
  cmdOramType=t;
  return true;
}

static const char* cmdUsage = "";
void cmdSetUsage(const char* c) { cmdUsage=c; }

/*
   Grammar:
   genr := (type|netw)
   cmd  := search genr* (("test"  (genr|testopt)*  inputs) |
                         ("bench" (genr|benchopt)* inputs)))
*/
void 
cmdParseCommon(cargs,appopts,appoptArgs)
  CmdParseState* cargs;
  bool (*appopts)(CmdParseState*,void*);
  void* appoptArgs;
{
  assert(cargs->cmdIndex==1);

  while(cargs->cmdIndex<cargs->argc)
  { int cur=cargs->cmdIndex;
    if(!cmdParseMode(cargs)     ||
       !cmdParseOramType(cargs) ||
       !cmdParseNetwork(cargs)) cmdErrExit(cmdUsage);
    if(cmdMode!=cmdModeNone) break;
    if(cur==cargs->cmdIndex) cmdUnknownOptionExit(cargs->argv[cur]);
  }

  while(cargs->cmdIndex<cargs->argc)
  { // Mode is set at this point
    const char* a=cargs->argv[cargs->cmdIndex];
    if(a[0]!='-' && a[0]!='+' && cmdModeNetDone()) break; // End of options
    int cur=cargs->cmdIndex;
    if(!cmdParseNetwork(cargs)    ||
       !cmdParseOramType(cargs)   ||
       !cmdParseShowResult(cargs) ||
       !appopts(cargs,appopts)) 
      cmdErrExit(cmdUsage);
    if(cur==cargs->cmdIndex) cmdUnknownOptionExit(cargs->argv[cur]);
  }
  if(cmdMode==cmdModeNone) 
  { fprintf(stderr,"Neither 'test' nor 'bench' was specified\n");
    exit(-1);
  }
  if(!cmdNetDone())
  { fprintf(stderr,"Network server/port options missing\n");
    exit(-1);
  }
  if(!cmdOramTypeDone())
  { fprintf(stderr,"Required --oramtype is missing\n");
    exit(-1);
  }
  // Parse non-option params outside function first
}
CmdParseState
cmdParseInit(int argc, char* argv[])
{
  CmdParseState rv = {.argc=argc,.argv=argv,.cmdIndex=1};
  return rv;
}
