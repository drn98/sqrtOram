#include<stdlib.h>
#include<stdio.h>
#include<obliv.h>
#include"testPropagate.h"
#include"util.h"

void showUsage()
{
  fprintf(stderr,
    "Usage: ./testPropagate -- <port> (.|-num|num)*\n"
    "       ./testPropagate <remote-addr> <port> <blank-count> <data-count>\n");
  exit(1);
}

int intCmp(const void* a, const void* b) 
  { return *(const int*)a-*(const int*)b; }
void initArrays(int** dataCopy,bool** hasdata,bool** haserase,
    int* data,size_t n)
{
  *dataCopy=malloc(n*sizeof(int));
  *hasdata = calloc(n,sizeof(bool));
  *haserase = calloc(n,sizeof(bool));
  memcpy(*dataCopy,data,n*(sizeof(int)));
  qsort(*dataCopy,n,sizeof(int),intCmp);
}
bool recordArrayCheck(const int dataCopy[],bool hasdata[],bool haserase[],
                      size_t n,int data,bool isdata,bool iserase)
{
  if(!isdata && !iserase) return true;
  size_t i = (const int*)bsearch(&data,dataCopy,n,sizeof(int),intCmp)-dataCopy;
  if(iserase) // can't have duplicates
  { if(haserase[i]) return false;
    else haserase[i]=true;
  }else
  { if(hasdata[i]) return false;
    else hasdata[i]=true;
  }
  return true;
}
bool resultCheckArray(const int dataCopy[],
                      bool hasdata[],bool haserase[],
                      size_t n,int data,bool isdata,bool iserase)
{
  if(!isdata && !iserase) return true;
  size_t i = (const int*)bsearch(&data,dataCopy,n,sizeof(int),intCmp)-dataCopy;
  if(hasdata[i]==haserase[i]) return false; // Should have cancelled out
  if(haserase[i]!=iserase) return false;    // Conflicting with input data
  hasdata[i]=haserase[i]=false;             // Don't want new duplicates
  return true;
}
void cleanupArray(int dataCopy[],bool hasdata[],bool haserase[]) 
  { free(dataCopy); free(hasdata); free(haserase); }

int main(int argc,char* argv[])
{
  PropagateArgs args;
  size_t n,i;
  if(argc<3) showUsage();
  int me = strcmp(argv[1],"--")==0?1:2;
  int *dataCopy;
  bool *hasdata,*haserase;
  if(me==1)
  { args.n=n=argc-3;
    args.data=malloc(n*sizeof(int));
    args.isdata=calloc(n,sizeof(bool));
    args.iserase=calloc(n,sizeof(bool));
    for(i=0;i<n;++i)
    { if(argv[3+i][0]=='.') continue;
      int x;
      if(sscanf(argv[3+i],"%d",&x)!=1) showUsage();
      if(x<0) { x=-x; args.iserase[i]=true; }
      else args.isdata[i]=true;
      args.data[i]=x;
    }
    initArrays(&dataCopy,&hasdata,&haserase,args.data,n);
    for(i=0;i<n;++i) 
      if(!recordArrayCheck(dataCopy,hasdata,haserase,n,
                           args.data[i],args.isdata[i],
                           args.iserase[i]))
      { fprintf(stderr,"Propagation doesn't support duplicates: %d\n",
                args.data[i]);
        exit(1);
      }
  }else
  { if(sscanf(argv[3],"%d",&args.outBlank)!=1 ||
       sscanf(argv[4],"%d",&args.outData) !=1)
      showUsage();
  }
  ProtocolDesc pd;
  connectOrDie(&pd,argv[1],argv[2]);
  setCurrentParty(&pd,me);
  execYaoProtocol(&pd,testPropagate,&args);
  cleanupProtocol(&pd);
  n=args.n;
  if(n<100) for(i=0;i<n;++i)
  { if(!args.isdata[i] && !args.iserase[i]) printf(".");
    else if(args.isdata[i] && args.iserase[i]) printf("!");
    else if(args.isdata[i]) printf("%d",args.data[i]);
    else printf("-%d",args.data[i]);
  }
  printf("\n");
  if(me==1) for(i=0;i<n;++i) 
    if(!resultCheckArray(dataCopy,hasdata,haserase,n,
                         args.data[i],args.isdata[i],
                         args.iserase[i]))
    { printf("Checks failed: results don't make sense\n");
      goto clean;
    }
  for(i=0;i<args.outBlank;++i) if(args.isdata[i]||args.iserase[i]) 
  { printf("Checks failed: too few blanks\n");
    goto clean;
  }
  for(i=args.outBlank;i<args.outData+args.outBlank;++i) 
    if(!args.isdata[i]||args.iserase[i]) 
    { printf("Checks failed: too few data items\n"); 
      goto clean; 
    }
  printf("Checks passed\n");
clean:
  if(me==1) cleanupArray(dataCopy,hasdata,haserase);
  free(args.data);
  free(args.isdata);
  free(args.iserase);
  return 0;
}
