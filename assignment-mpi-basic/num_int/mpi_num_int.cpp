#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <chrono>

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

  
int main (int argc, char* argv[]) {

  auto initiated = check_time::now();

  MPI_Init(&argc, &argv);
  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
  float result;
  int functionid = stoi(argv[1]);
  float a = stof(argv[2]);
  float b = stof(argv[3]);
  int n = stoi(argv[4]);
  int intensity = stoi(argv[5]);

  float mid = ((b - a) / n );
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    float sum = 0;
    for (int i=rank; i<n; i+=size){
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
  MPI_Reduce(&sum, &result, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

  if(rank == 0)
  {
     cout<<mid*result;
  }
  auto end = check_time::now();
  auto time_elapsed = end - initiated;
  auto secs = std::chrono::duration_cast<std::chrono::duration<float>>(time_elapsed);

  if(rank == 0)
  {
     cerr<<secs.count();
  }
  MPI_Finalize();
  return 0;
}
