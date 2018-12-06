#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include <chrono>
#include <cmath>
using namespace std;
using seconds = chrono::seconds;
using check_time = std::chrono::high_resolution_clock;

#ifdef __cplusplus
extern "C" {
#endif

  float f1(float x, int intensity);
  float f2(float x, int intensity);
  float f3(float x, int intensity);
  float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

float num_int(int functionid, float a, int range[], int intensity,  float mid)
{
  float sum = 0;
  
  for (int i=range[0]; i<range[1]; i++){

    float x = (a + (i + 0.5) * mid);
    
    switch(functionid){      
      case 1:
      sum +=f1(x, intensity);
      break;

      case 2:
      sum +=f2(x, intensity);
      break;

      case 3:
      sum +=f3(x, intensity);
      break;

      case 4:
      sum +=f4(x, intensity);
      break;
    }
    
  }
  return sum;
}

int main (int argc, char* argv[]) {

  MPI_Init(&argc, &argv);
  if (argc < 6) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
  int functionid = stoi(argv[1]);
  float a = stof(argv[2]);
  float b = stof(argv[3]);
  int n = stoi(argv[4]);
  float intensity = stoi(argv[5]);
  float mid = ((b - a) / n );
  float local_sum = 0, global_sum = 0;
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Status status;
  int partition = (n/(size - 1))/8;

  auto initiated = check_time::now();  

  if(rank == 0){

    int chunk_sent = 0,range[2], pointer;
    for(int i=1; i< size; i++){

      range[0] = (i - 1) * partition;
      range[1] = i * partition;
      pointer = range[1];
      chunk_sent++;

      if(range[0] >= n){
        range[0] = -1;
        chunk_sent--;
      }else if(range[1]>n){
        range[1] = n;
      }

      MPI_Send(range, 2, MPI_INT, i, 0, MPI_COMM_WORLD);      
    }
    while(chunk_sent != 0){

      MPI_Recv(&local_sum, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      chunk_sent--;
      global_sum+=local_sum;
      range[0] = pointer;
      range[1] = pointer + partition;
      pointer = range[1];
      chunk_sent++;

      if(range[0] >= n){
        range[0] = -1;
        chunk_sent--;
      }else if(range[1]>n){
        range[1] = n;
      }

      MPI_Send(range, 2, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }
  }else{
      int worker_range[2];
      float sum;

      MPI_Recv(worker_range, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      while(worker_range[0]!=-1){
        sum = num_int(functionid, a, worker_range, intensity, mid);
        MPI_Send(&sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(worker_range, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }

    if(rank == 0){

      cout<<global_sum*mid;
      auto end = check_time::now();
      auto time_elapsed = end - initiated;
      auto secs = std::chrono::duration_cast<std::chrono::duration<float>>(time_elapsed);
      cerr<<secs.count();

    }

    MPI_Finalize();
    return 0;
  }  