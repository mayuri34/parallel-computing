#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
  void generatePrefixSumData (int* arr, size_t n);
  void checkPrefixSumResult (int* arr, size_t n);
#ifdef __cplusplus
}
#endif


int main (int argc, char* argv[]) {

std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  
  if (argc < 3) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  
  int n = atoi(argv[1]);
  int * arr = new int [n];
  int nbthreads = atoi(argv[2]);
  int * pr = new int [n+1];
  int granularity = n/nbthreads;
  int * t = new int [nbthreads];
  int * k_arr = new int [nbthreads];
  
  generatePrefixSumData (arr, n);
  omp_set_num_threads(nbthreads);
  
  pr[0] = 0;
  t[0] = 0;
  k_arr[0] = 0;

#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }

#pragma omp for schedule(static, 1)

    for(int thread = 0; thread < nbthreads; thread++){
      for(int i= (thread * granularity);i<((thread +1) * granularity); i++){
        if(i== (thread * granularity)){
          pr[i+1] = 0 + arr[i];
        }else{
          pr[i+1] = pr[i] + arr[i];
        }
        t[thread + 1] = pr[i+1];    
      }
        k_arr[thread +1] = ((thread +1) * granularity)+1;
    }
  }

  if(nbthreads * granularity <n){
    for(int k = nbthreads * granularity; k < n; k++){
      pr[k+1] = pr[k] + arr[k];
    }
  }

  int * temp = new int [nbthreads];
  temp[0] = t[0];

  for(int i = 1; i<nbthreads; i++){
    temp[i] = temp[i-1] + t[i];
  }

#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
#pragma omp for schedule(static, 1)

    for(int i =0;i<nbthreads; i++){
      for(int j=k_arr[i]; j < k_arr[i+1]; j++){
      pr[j] = pr[j] + temp[i];
      }
    }
  }
  if(nbthreads * granularity < n){
    for(int k = nbthreads * granularity; k < n; k++){
      pr[k+1] = pr[k+1] + temp[nbthreads-1];
    }
  }
  
  checkPrefixSumResult(pr, n);
  delete[] arr;

  // end time
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapased_seconds = end-start;
  std::cerr<<elapased_seconds.count()<<std::endl;

  return 0;
}