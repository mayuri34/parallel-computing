#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>

using namespace std;

float genA (int row, int col) {
  if (row > col)
    return 1.;
  else
    return 0.;
}

float genx0 (int i) {
  return 1.;
}


void checkx (int iter, long i, float xval) {
  if (iter == 1) {
    float shouldbe = i;
    if (fabs(xval/shouldbe) > 1.01 || fabs(xval/shouldbe) < .99 )
      std::cout<<"incorrect : x["<<i<<"] at iteration "<<iter<<" should be "<<shouldbe<<" not "<<xval<<std::endl;
  }

  if (iter == 2) {
    float shouldbe =(i-1)*i/2;
    if (fabs(xval/shouldbe) > 1.01 || fabs(xval/shouldbe) < .99)
      std::cout<<"incorrect : x["<<i<<"] at iteration "<<iter<<" should be "<<shouldbe<<" not "<<xval<<std::endl;
  }
}

int main (int argc, char*argv[]) {
  MPI_Init(&argc, &argv);

  if (argc < 3) {
    std::cout<<"usage: "<<argv[0]<<" <n> <iteration>"<<std::endl;
  }

  int wrank, wsize, row_rank, col_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
  MPI_Comm_size(MPI_COMM_WORLD, &wsize);

  bool check = true;
  
  long n = atol(argv[1]);

  long iter = atol(argv[2]);

  MPI_Comm row_comm, col_comm;
  MPI_Comm_split(MPI_COMM_WORLD, (wrank / n), wrank, &col_comm);
  MPI_Comm_split(MPI_COMM_WORLD, (wrank % n), wrank, &row_comm);

  MPI_Comm_rank(row_comm, &row_rank);
  MPI_Comm_rank(col_comm, &col_rank);

  int partition = n/sqrt(wsize);
  int row_start = (wrank - (wrank - (wrank / sqrt(wsize)))) * partition;
  int col_start = (wrank - (wrank - (fmod(wrank, sqrt(wsize))))) * partition;
  float* A = new float[partition*partition];
  float* x = new float[partition];
  float* y = new float[partition];

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  
  for(int i = row_start; i< (row_start + partition); i++){
    for(int j = col_start; j<(col_start + partition); j++){
      A[i*n+j] = genA(i, j);
    }
  }

  for (int i=col_start; i<(col_start + partition); i++){
    x[i] = genx0(i);
  }

  for(int it = 0; it<iter; it++){
    for(int row =row_start; row<(row_start + partition);row++){
      float local_sum = 0.0;
      for(int col = col_start; col<(col_start + partition); col++){
        MPI_Bcast(&(x[col]), 1, MPI_FLOAT, col_rank , col_comm);
	local_sum += A[row*n+col] * x[col];
      }
    MPI_Reduce(&local_sum, &(y[row]), 1, MPI_FLOAT, MPI_SUM,col_rank, row_comm);
  }

  {
    float *t = x;
    x=y;
    y=t;
  }
    
  if (check){
    for (long i = 0; i<n; ++i){
      checkx (it+1, i, x[i]);
    }
  }
}

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;

  std::cerr<<elapsed_seconds.count()<<std::endl;

  
  
  MPI_Comm_free(&row_comm);
  MPI_Comm_free(&col_comm);

  delete[] A;
  delete[] x;
  delete[] y;

  MPI_Finalize();
  return 0;
}