#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int main (int argc, char* argv[]) {

  //forces openmp to create the threads beforehand
#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
  }
  
  if (argc < 3) {
    std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int * arr = new int [n];
  int result = 0;
  int nbthreads = atoi(argv[2]);
  int granularity = n/nbthreads;
  omp_set_dynamic(0);
  omp_set_num_threads(nbthreads);
  int * partial_sum = new int [nbthreads];

  generateReduceData (arr, atoi(argv[1]));
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

#pragma omp parallel for
for(int i=0; i<n; i+=granularity){
  int j;
  int begin = i;
  int end = begin+granularity;
  if(end>n){
    end = n;
  }
  #pragma omp task
  {
    for(j=begin;j<end;j++){
      partial_sum[omp_get_thread_num()]+=arr[j];
    }

  }
 } 


for(int thread =0;thread <= nbthreads; thread++){
  result+=partial_sum[thread];

}
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  delete[] arr;

  float elapsed_seconds = (end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec - start.tv_sec);

  // display result
  std::cout<<result<<std::endl;
  std::cerr<<elapsed_seconds<<std::endl;

  return 0;
}
