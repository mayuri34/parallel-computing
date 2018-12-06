#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
using namespace std;
#ifdef __cplusplus
extern "C" {
#endif
  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);
#ifdef __cplusplus
}
#endif
/*
*/
int main (int argc, char* argv[]) {
auto start = chrono::steady_clock::now();
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
 
  if (argc < 3) { std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int nbthreads = atoi(argv[2]);
  int * arr = new int [n];
  int * temp_arr = new int [n];
  int i, j, k,thread, left_start, right_end, current_value = 0;
  generateMergeSortData (arr, n);

#pragma omp parallel
{
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }

#pragma omp for reduction(*:current_value) schedule(static, 1)

  for(int thread = 0; thread <nbthreads; thread++){
    while(current_value<n){     
      for (int left=0; left+current_value < n; left += current_value*2 ) {
        left_start = left + current_value;       
        right_end = left_start + current_value;
        if (right_end > n){
          right_end = n;
        }
        k = left;
        i = left;
        j = left_start;
        while (i < left_start && j < right_end) {
          if (arr[i] <= arr[j]) {        
            temp_arr[k] = arr[i]; i++;
          } else {
            temp_arr[k] = arr[j]; j++;
          }
          k++;
        }
        while (i < left_start) {
          temp_arr[k]=arr[i];
          i++; k++;
        }
        while (j < right_end) {
          temp_arr[k]=arr[j];
          j++;
          k++;
        }
        for (k=left; k < right_end; k++) {
          arr[k] = temp_arr[k];
        }
      }
    current_value = current_value * 2;
  }
}
} 
  checkMergeSortResult (arr, n);
 
  delete[] arr;

  // end time
  auto end = chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  cerr<<elapsed.count();
  return 0;
}

