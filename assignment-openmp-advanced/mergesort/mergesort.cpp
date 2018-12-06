#include <omp.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <cstring>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

void merge(int * X, int n, int * tmp) {
   int i = 0;
   int j = n/2;
   int ti = 0;

   while (i<n/2 && j<n) {
      if (X[i] < X[j]) {
         tmp[ti] = X[i];
         ti++; i++;
      } else {
         tmp[ti] = X[j];
         ti++; j++;
      }
   }
   while (i<n/2) {
      tmp[ti] = X[i];
      ti++; i++;
   }
      while (j<n) {
         tmp[ti] = X[j];
         ti++; j++;
   }
   memcpy(X, tmp, n*sizeof(int));

}

void serial_merge(int arr[], int n) {
   
for(int i=0; i<n;i++){

    int first = i%2;

    #pragma omp parallel for default(none) shared(arr, n, first)
      for(int j=first; j<n-1; j+=2){
        if(arr[j]>arr[j+1]){
          swap(arr[j],arr[j+1]);
        }
      }
  }

}

void mergesort(int * X, int n, int * tmp)
{

   if (n < 100) {
   serial_merge(X, n);
   return;
   }

   #pragma omp task firstprivate (X, n, tmp)
   mergesort(X, n/2, tmp);

   #pragma omp task firstprivate (X, n, tmp)
   mergesort(X+(n/2), n-(n/2), tmp+(n/2));
 
   #pragma omp taskwait
   merge(X, n, tmp);
}

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
  
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int nbthreads = atoi(argv[2]);  
  // get arr data
  int * arr = new int [n];
  int * tmp = new int [n];
  omp_set_dynamic(0);
  omp_set_num_threads(nbthreads);
  generateMergeSortData (arr, n);

  // begin timing
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

   #pragma omp parallel
   {
      #pragma omp single
      mergesort(arr, n, tmp);
   }
  
  checkMergeSortResult (arr, n);

  delete[] arr;
  
  // end timing
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = end-start;

  // display time to cerr
  std::cerr<<elpased_seconds.count()<<std::endl;

  return 0;
}
