#include"qroram.h"
#include<stdlib.h>
#include<stdio.h> // debugging
#define HERE fprintf(stderr,"%s:%d\n",__FILE__,__LINE__)

struct QroramTable
{ long long *cost;
  int *lastShuff;
  int *shuffleCost;
  int nmax,amax;
};

static inline long long* cost(QroramTable* tbl,int a,int t)
  { return tbl->cost+a*(tbl->nmax+1)+t; }
static inline int* lastShuff(QroramTable* tbl,int a,int t)
  { return tbl->lastShuff+a*(tbl->nmax+1)+t; }

static int maxDepth(int nmax) { return nmax>=2?1+maxDepth((nmax+1)/2):1; }
static long long linearCost(int a,int t)
  { return 1LL*a*t*(t+1)/2; }

QroramTable* qroramTable(int nmax)
{
  int amax = maxDepth(nmax)+1;
  QroramTable *tbl = malloc(sizeof(*tbl));
  tbl->cost=malloc((nmax+1)*(amax+1)*sizeof(long long));
  tbl->lastShuff=malloc((nmax+1)*(amax+1)*sizeof(int));
  tbl->nmax=nmax;
  tbl->amax=amax;
  int d,a,t,t2;

  // Waksman network costs
  int *shuffleCost = malloc((1+nmax)*sizeof(int));
  tbl->shuffleCost=shuffleCost;
  shuffleCost[0]=shuffleCost[1]=0;
  for(t=2;t<=nmax;++t) shuffleCost[t]=t-1+shuffleCost[t/2]+shuffleCost[(t+1)/2];

  // Initialize main table
  for(a=1;a<=amax;++a)
    for(t=0;t*a<=nmax;t++)
    { *cost(tbl,a,t) = linearCost(a,t);
      *lastShuff(tbl,a,t) = -1;
    }
  for(d=1;d<=amax;++d)
    for(a=1;a<=amax-d;++a)
      for(t=1;t*a<=nmax;++t)
      { int lo=0,hi=t;
        if(t>1 && *lastShuff(tbl,a,t-1)>=0)
        { int windowmid = *lastShuff(tbl,a,t-1);
          if(lo<windowmid) lo=windowmid;
          if(hi>windowmid+2) hi=windowmid+2;
        }
        for(t2=lo;t2<hi;++t2) if(t2>0)
        { int tnext=t-t2;
          if(tnext*(a+1)>nmax||t2*a<tnext) continue;
          int c=*cost(tbl,a,t2)+shuffleCost[a*t2]+*cost(tbl,a+1,tnext);
          if(c<*cost(tbl,a,t))
          { *cost(tbl,a,t)=c;
            *lastShuff(tbl,a,t)=t2;
          }
        }
      }
  return tbl;
}

void qroramTableRelease(QroramTable* tbl)
{
  free(tbl->cost);
  free(tbl->lastShuff);
  free(tbl->shuffleCost);
  free(tbl);
}

int qroramBestPeriod(QroramTable* tbl,int n)
{
  double c=n;
  int p=-1,i;
  for(i=1;i<=n;++i)
  { double c2=(tbl->shuffleCost[n]+*cost(tbl,1,i))*1.0/i;
    if(c2<c) { c=c2; p=i; }
  }
  return p;
}
void qroramDebugInfo(QroramTable* tbl)
{
  int a,t;
  for(a=1;a<=tbl->amax;++a)
  { printf("%d:",a);
    for(t=0;t*a<=tbl->nmax;++t)
      printf(" %d",*lastShuff(tbl,a,t));
    printf("\n");
  }
}
typedef struct { int t,a; } pair;
static int cmpByT(const void* va,const void* vb)
  { return ((const pair*)va)->t-((const pair*)vb)->t; }

int qroramShuffTimes_aux(QroramTable* tbl,pair p[],int a,int t)
{
  int rv=0,i;
  while(t>0)
  { int t2=*lastShuff(tbl,a,t);
    if(t2<0) break;
    int nestC = qroramShuffTimes_aux(tbl,p+rv,a+1,t-t2);
    for(i=0;i<nestC;++i) p[rv+i].t+=t2;
    rv+=nestC;
    p[rv++]=(pair){.t=t2,.a=a};
    t=t2;
  }
  return rv;
}

// Output arrays need to be freed
// Return value == size of each array
// Last value is guaranteed to be a full shuffle (depth 0)
// Does not explicitly produce a (0,0) to start with.
int qroramShuffTimes(QroramTable* tbl,int** outT,int** outA,int n)
{
  int period = qroramBestPeriod(tbl,n);
  pair* pr=malloc((period+1)*sizeof(pair));
  int i,size = qroramShuffTimes_aux(tbl,pr,1,period);
  pr[size++]=(pair){.t=period,.a=0};
  qsort(pr,size,sizeof(pair),cmpByT);
  *outT = malloc(size*sizeof(int));
  *outA = malloc(size*sizeof(int));
  for(i=0;i<size;++i)
  { (*outT)[i]=pr[i].t;
    (*outA)[i]=pr[i].a;
  }
  return size;
}

/*
typedef struct Qroram
{
  OcCopy* cpy;
  size_t n;
  int maxdepth,*layer;
} Qroram;
*/

// Plans
// Make random accesses on a shuffled array
// Make with Layer(OcCopy,(index,void*),n,layerID,table), 
//   takes ownership of buffer writes into layer. Write new indices to a 
//   subsidiary ORAM
// Read with readLayer(layer,obliv index,layerID) --> either the correct element
//   or a previously unread new element.

// On access to a layer, we either:
//   1) Access the real element if it is in this layer and unused
//   2) Access a random element of the ones unused
// Reveal the one that is accessed, and publicly mark it as being used.
