#include <cstdlib>
#include "mpi.h"

#include <iostream>
static int n = 1;
#include "lwperf.h"
#define MAXLINE 256


void cxxsub1(int x) {
  if (x < n) {
    x = x + 1;
    printf("x = %d\n", x);
    cxxsub1(x);
  }
}

int main(int argc, char **argv)
{
  try{
    int me,csize,x;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&me);
    MPI_Comm_size(MPI_COMM_WORLD,&csize);
    std::cout << me << "/" <<csize << std::endl;

    PERFDECL(PERF::init();)
    PERFDECL(PERF::mpiArgs(me,csize);)
    int flag;

    PERFDECL(PERF::stringOptions("x5550","gcc","cxxapp.cpp","cxxapp","tesla.",".log");)

    if (me == 0) {
      printf("Enter number of repeats\n");
      char line[MAXLINE];
      char *dummy = fgets(line,MAXLINE,stdin);
      if (dummy==NULL) { exit(1);}
      sscanf(line,"%d",&flag);
      
    }
    MPI_Bcast(&flag,1,MPI_INT,0,MPI_COMM_WORLD);

    int nx=0; double dp = 0;
    x=0;
    n = flag;
    std::cout << "n = " << n <<std::endl;
    PERFLOG(cxxsub1,IP(n)) ;
    PERFLOG(cxxsub2,IP(nx),DP(dp));
    cxxsub1(x);
    PERFSTOP(cxxsub2,IP(nx),DP(dp));
    PERFSTOP(cxxsub1,IP(n));
    PERFDECL(PERF::finalize();)

    MPI_Finalize();
    exit(0);
  } catch (char *msg) {
     std::cout << msg <<std::endl;
  }
}

