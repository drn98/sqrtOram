#pragma once
#include<obliv.h>

// Legacy version: server == "--" means we listen
void connectOrDie(ProtocolDesc* pd,const char* server,const char* port);
// New version: uses whatever is in cmdNetSpec. .server==NULL means we listen
void cmdConnectOrDie(ProtocolDesc* pd);

double wallClock();

bool cmdIsPrefix(const char* prefix,const char* str);
int cmdFindFirst(const char* search,const char* alts[],int n);
int cmdFindFirstPrefix(const char* search,const char* alts[],int n);

typedef enum
{ oramTypeNone=-1,
  oramTypeLin,
  oramTypeSqrt,
  oramTypeCkt
} OramType;

static inline OramType
oramTypeFromString(const char* s)
{ const char* types[] = {"linear","sqrt","ckt"};
  return cmdFindFirstPrefix(s,types,sizeof(types)/sizeof(*types));
}

void cmdShowUsageExit(const char cmdUsage[]);

extern struct CmdNetSpec { const char *server,*port; } cmdNetSpec;
// Call this *after* cmdParseCommon() returns
static inline bool cmdMeServing(void) 
  { return cmdNetSpec.server==NULL || cmdNetSpec.server[0]=='\0'; }
enum CmdMode { cmdModeNone=-1,cmdModeTest,cmdModeBench } cmdMode;
extern bool cmdShowResult;
void cmdParseCheckNumeric(const char* s);


void cmdSetUsage(const char* c);
static inline bool cmdModeNetDone() 
  { return cmdMode!=-1 && cmdNetSpec.port!=0; }
static inline bool cmdNetDone() { return cmdNetSpec.port; }

typedef struct { int argc; char** argv; int cmdIndex; } CmdParseState;
CmdParseState cmdParseInit(int argc, char* argv[]);
extern OramType cmdOramType;

const char* cmdParseSingleArg(CmdParseState* cargs,
    char shortopt,const char* longopt);
bool cmdParseSingleInt(CmdParseState* cargs,char shortopt,const char* longopt,
                  int* dest,int init);
// Parses all remaining arguments as integers.
void cmdParseTermInts(CmdParseState* ps,int** outarray,int* outsize);
void cmdParseEnd(CmdParseState* cargs);

void cmdParseCommon(
  CmdParseState* cargs,
  bool (*appopts)(CmdParseState*,void*),
  void* appoptArgs
  );

// TODO need to standardize error policy. Some functions return false on error,
// while others simply exit. Things to ponder about:
//   Unrecognized error, syntax error, error messages (e.g. "-n 50" vs "-n -x")
//   Combined single options (e.g. "-xk"), conflicting values (compare with
//   -1, check bool etc.).

void matchedBroadcastInt(int* x,bool init,const char* msg);
int* randomIntArray(int max,int n);
