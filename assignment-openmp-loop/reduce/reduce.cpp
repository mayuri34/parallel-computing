#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include<string.h>

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif
  void generateReduceData (int* arr, size_t n);
#ifdef __cplusplus
}
#endif


int main (int argc, char* argv[]) {

  if (argc < 5) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads> <scheduling> <granularity>"<<std::endl;
    return -1;
  }

    int n = atoi(argv[1]);
    int nbthreads = atoi(argv[2]);
    char* scheduling = argv[3];
    int granularity = atoi(argv[4]);

    omp_set_num_threads(nbthreads);

    if("static" == scheduling){
      omp_set_schedule(omp_sched_static, granularity);

    } else if("dynamic" == scheduling){
      omp_set_schedule(omp_sched_dynamic, granularity);
    } else if("guided" == scheduling){
      omp_set_schedule(omp_sched_guided, granularity);
    }

    int * arr = new int [n];
  int result = 0;

  generateReduceData (arr, n);

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();


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

     #pragma omp for reduction (+ : result) schedule (runtime)    

	for (int i = 0; i < n; ++i) {
	  result = result + arr[i];
	} 
  }

  
  // end time
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapased_seconds = end-start;

  // display result
  std::cout<<result<<std::endl;
  std::cerr<<elapased_seconds.count()<<std::endl;

  
  delete[] arr;

  return 0;
}
