#include<stdio.h>
#include"waksman.c"
#include<time.h>

void random_permute(unsigned p[],size_t n)
{
  int i,j;
  for(i=0;i<n;++i) p[i]=i;
  srand(time(0));
  for(i=n-1;i>0;--i) 
  { j = rand()%i;
    unsigned t = p[i];
    p[i]=p[j]; p[j]=t;
  }
}

void printPerm(const unsigned p[],size_t n)
{
  int i;
  for(i=0;i<n;++i) printf("%u ",p[i]);
  printf("\n");
}

bool testWaksman(const unsigned p[],size_t n,
    const unsigned a[],const unsigned b[],const bool c[])
{
  const size_t sc = waksmanSwapCount(n);
  unsigned *vals = malloc(n*sizeof *vals), t;
  int i;
  for(i=0;i<n;++i) vals[i]=i;
  for(i=0;i<sc;++i) if(c[i])
  { if(a[i]>=n || b[i]>=n) return printf("out of bounds\n"),false;
    t = vals[a[i]];
    vals[a[i]] = vals[b[i]];
    vals[b[i]] = t;
  }
  for(i=0;i<n;++i) if(p[i]!=vals[i]) break;
  free(vals);
  return i==n;
}

bool testLevelSizes(const unsigned a[], const unsigned b[],size_t n,
    const size_t level_size[],size_t lc)
{
  int i,j, sc = waksmanSwapCount(n);
  bool *used = calloc(n,sizeof(bool));
  bool rv = true;
  int l=0;
  for(i=j=0;i<sc;++i)
  { if(i-j>=level_size[l])
    { j=i; ++l;
      memset(used,0,sizeof(bool)*n);
    }
    if(used[a[i]]||used[b[i]]) rv=false;
    used[a[i]] = used[b[i]] = true;
  }
  free(used);
  if(l>lc) rv=false;
  return rv;
}

int main(int argc,char* argv[])
{
  int type,i;
  size_t n,sc;
  bool verbose = (argc>1 && strcmp(argv[1],"-v")==0);
  while(1)
  {
    if(verbose) printf(
         "Test 1: strata count\n"
         "Test 2: strata assign\n"
         "Test 3: permute_in_place\n"
         "Test 4: stratify\n"
         "     5: exit\n"
         "\n"
         "Enter test type: \n"
         );
    if(scanf("%d",&type)<1) break;
    if(type<1||type>4) break;
    if(verbose) printf("Network size: \n");
    if(scanf("%zd",&n)<1) break;
    sc = waksmanSwapCount(n);
    switch(type)
    {
      case 1:
        printf("Strata count: %zd\n",waksmanStrataCount(n));
        break;
      case 2:
      {
        unsigned *a = malloc(sc*sizeof(*a));
        unsigned *b = malloc(sc*sizeof(*b));
        unsigned *level = malloc(sc*sizeof(*level));
        waksmanNetwork(a,b,n);
        waksmanStrataAssign(level,a,b,n);
        printf("Levels:");
        for(i=0;i<sc;++i) printf(" %u",level[i]);
        printf("\n");
        free(a); free(b); free(level);
      } break;
      case 3:
      {
        unsigned *a = malloc(sc*sizeof(*a));
        unsigned *p = malloc(sc*sizeof(*p));
        for(i=0;i<sc;++i) a[i]=i;
        random_permute(p,sc);
        char* flags = calloc(sc,1);
        unsigned t;
        permute_in_place((void*)a,sc,sizeof(unsigned),p,flags,&t);
        printPerm(p,sc);
        printPerm(a,sc);
        for(i=0;i<sc;++i) if(p[a[i]]!=i) break;
        printf(i==sc?"Properly permuted\n":"Buggy permutation\n");
        free(flags); free(a); free(p);
      } break;
      case 4:
      {
        unsigned * p = malloc(n*sizeof(*p));
        random_permute(p,n);

        unsigned *a = malloc(sc*sizeof(*a)), *b = malloc(sc*sizeof(*b));
        bool *c = malloc(sc*sizeof(*c));
        unsigned lc = waksmanStrataCount(n);
        size_t *level_size = malloc(lc*sizeof *level_size);
        waksmanNetwork(a,b,n);
        waksmanSwitches(p,n,c);
        if(!testWaksman(p,n,a,b,c)) 
        { printf("Even before stratifying, permutation is failing\n");
          goto stratafail;
        }
        waksmanStratify(a,b,c,n,level_size);
        if(!testWaksman(p,n,a,b,c))
        { printf("Waksman failing after stratify\n");
          goto stratafail;
        }
        if(!testLevelSizes(a,b,n,level_size,lc))
        { printf("Not properly stratified\n");
          goto stratafail;
        }

        printf("All tests passed\n");
stratafail:
        free(level_size);
        free(a); free(b); free(c);
      } break;
    }
  }
  return 0;
}

