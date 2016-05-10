#include"util.h"
#include<assert.h>
#include<error.h>
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

void connectOrDieAux(ProtocolDesc* pd,const char* server,const char* port)
{
  if(!server) // I *am* the server
  { if(protocolAcceptTcp2P(pd,port)!=0) error(1,0,"TCP accept failed\n");
    setCurrentParty(pd,1); 
  }else
  { if(protocolConnectTcp2P(pd,server,port)!=0)
      error(1,0,"TCP connect failed\n");
    setCurrentParty(pd,2);
  }
}
void connectOrDie(ProtocolDesc* pd,const char* server,const char* port)
  { connectOrDieAux(pd,strcmp("--",server)==0?0:server,port); }

void cmdConnectOrDie(ProtocolDesc* pd)
{ const char* s = cmdNetSpec.server;
  s=(s&&s[0]?s:NULL);
  connectOrDieAux(pd,s,cmdNetSpec.port); 
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
noPrefix(const char* alts[],int n)
{ int i,j;
  for(i=0;i<n;++i) for(j=0;j<n;++j) if(i!=j && cmdIsPrefix(alts[i],alts[j]))
    return false;
  return true;
}

int
cmdFindFirstPrefix(const char* search,const char* alts[],int n)
{
  assert(noPrefix(alts,n));
  int i,res=-1;
  for(i=0;i<n;++i) if(cmdIsPrefix(search,alts[i]))
  { if(res==-1) res=i;
    else return -1;
  }
  return res;
}

void
cmdUnknownOptionExit(const char opt[])
{
  fprintf(stderr,"Unknown option %s\n",opt);
  exit(-1);
}

struct CmdNetSpec cmdNetSpec = {};
bool cmdShowResult = true;

bool cmdParseMode(CmdParseState* cargs)
{
  const char* modes[] = {"test","bench"};
  int i = cmdFindFirst(cargs->argv[cargs->cmdIndex],modes,2);
  if(i==-1) return false;
  if(cmdMode!=-1 && cmdMode!=i) 
    error(-1,0,"Choose either 'test' or 'bench', not both\n");
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
  if(cmdNetDone()) return false;
  const char* a = cargs->argv[cargs->cmdIndex];
  char* p=strchr(a,':');
  if(!p) return false;
  if(!numericString(p+1)) return false;
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
  if(i==-1) return false;
  if(cmdMode!=cmdModeTest) 
    error(-1,0,"Option '%s' can only be used with 'search test'.i\n",a);
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
      error(-1,0,"Error: argument missing for -%c\n",shortopt);
    else { cargs->cmdIndex+=2; return cargs->argv[cargs->cmdIndex-1]; }
  }
  return 0;
}
bool
cmdParseSingleInt(CmdParseState* cargs,char shortopt,const char* longopt,
                  int* dest,int init)
{
  const char* a = cmdParseSingleArg(cargs,shortopt,longopt);
  if(!a) return false;
  int x;
  if(sscanf(a,"%d",&x)!=1) error(-1,0,"Integer expected, not '%s'\n",a);
  if(*dest==init || *dest==x) *dest=x;
  else error(-1,0,"Conflicting values for --%s\n",longopt);
  return true;
}

bool cmdParseOramType(CmdParseState* cargs)
{
  const char* targ = cmdParseSingleArg(cargs,'t',"oramtype");
  if(!targ) return false;
  OramType t = oramTypeFromString(targ);
  if(t==oramTypeNone) error(-1,0,"Invalid oram type %s\n",targ);
  if(cmdOramType!=oramTypeNone && cmdOramType!=t)
    error(-1,0,"Conflicting oram types specified\n");
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
  if(cargs->argc==1) error(-1,0,"%s\n",cmdUsage);

  while(cargs->cmdIndex<cargs->argc)
  { 
    if(!cmdParseMode(cargs)     &&
       !cmdParseOramType(cargs) &&
       !cmdParseNetwork(cargs)) 
      cmdUnknownOptionExit(cargs->argv[cargs->cmdIndex]);
    if(cmdMode!=cmdModeNone) break;
  }

  while(cargs->cmdIndex<cargs->argc)
  { // Mode is set at this point
    const char* a=cargs->argv[cargs->cmdIndex];
    if(a[0]!='-' && a[0]!='+' && cmdModeNetDone()) break; // End of options
    if(!cmdParseNetwork(cargs)    &&
       !cmdParseOramType(cargs)   &&
       !cmdParseShowResult(cargs) &&
       !appopts(cargs,appopts)) 
      cmdUnknownOptionExit(cargs->argv[cargs->cmdIndex]);
  }
  if(cmdMode==cmdModeNone) 
    error(-1,0,"Neither 'test' nor 'bench' was specified\n");
  if(!cmdNetDone())
    error(-1,0,"Network server/port options missing\n");
  if(!cmdOramTypeDone())
    error(-1,0,"Required --oramtype is missing\n");
}
CmdParseState
cmdParseInit(int argc, char* argv[])
{
  CmdParseState rv = {.argc=argc,.argv=argv,.cmdIndex=1};
  return rv;
}
