#include "stdio.h"
#include "stdlib.h"
static
int n = 1;

void csub1(int x) {
  if (x < n) {
    x = x + 1;
    printf("x = %d\n", x);
    csub1(x);
  }
}

#include "lwperf_c.h"

#include "mpi.h"

int main(){
  int x = 0, nx=0, m, me=0, size=0;
  double dp = 3.14;

  MPI_Init(0,0);
  MPI_Comm_rank(MPI_COMM_WORLD,&me);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  PERF_INIT;
  PERF_MPI(me,size);
  PERF_OPTS("x5550","gcc","capp.c","capp","tesla.",".log") ;
  char buf[26];

  if (!me) {
    printf("Enter number of repeats\n");
    fgets(buf,sizeof(buf)-2,stdin);
    char *endPtr;
    n = (int)strtol(buf, &endPtr, 10);
    if (endPtr == &(buf[0])) {
      printf("given string, \"%s\" has no initial number\n", buf);
      exit(1);
    }
    printf("n = %d\n", n);
  }
  MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

  PERFLOG(csub1,IP(n)) ;
  PERFLOG(csub2,IP(nx),DP(dp));
  csub1(x);
  PERFSTOP(csub2,IP(nx),DP(dp));
  PERFSTOP(csub1,IP(n));
  PERF_FINAL;
  MPI_Finalize();
  return 0;
}

