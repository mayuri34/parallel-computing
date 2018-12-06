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

float num_int(int functionid, float a, int range[], float intensity,  float mid)
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

void setArray(int *arr, int value){
  for(int i = 0; i<3; i++){
    arr[i] = value;
    
  }
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
  float local_sum = 0;
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Status status;
  float global_sum = 0;
  int partition = (n/(size - 1))/8;
  auto initiated = check_time::now();  
  

  if(rank == 0){

    int chunk_sent = 0,work_given = 0, range[2], pointer;
    
    for(int i=1; i< size; i++){
     for(int chunk = 0; chunk < 3; chunk++){
      range[0] = work_given * partition;
      range[1] = (work_given + 1)* partition;
      pointer = range[1];
      chunk_sent++;
      work_given++;

      if(range[0] >= n){
        range[0] = -1;
        chunk_sent--;
      }else if(range[1]>n){
        range[1] = n;
      }

      MPI_Send(range, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
    }      
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
  int worker_range[2], index[3], buffer1[2], buffer2[2], buffer3[2], rcount = 3, slot[3], send = 0, recieve_array[3], outcome;
  float sum = 0.0;
  MPI_Status stat[3];
  MPI_Request request[3];

  setArray(slot, 1);
  setArray(recieve_array, 1);
  

  while(true){
    if(slot[0] == 1 && recieve_array[0] == 1){
      MPI_Irecv(buffer1, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &request[0]);
    }
    if(slot[1] == 1 && recieve_array[1] == 1){
      MPI_Irecv(buffer2, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &request[1]);
    }
    if(slot[2] == 1 && recieve_array[2] == 1){
      MPI_Irecv(buffer3, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &request[2]);
    }
    setArray(recieve_array, 0);
    
    while(true){
      MPI_Waitsome(3, request, &outcome, index, stat);
      if(outcome > 0){
        break;
      }
    }

    for(int i=0; i<outcome; i++){
      send = 1;
      if(index[i] == 0){
        
        worker_range[0] = buffer1[0];
        worker_range[1] = buffer1[1];

        if(worker_range[0]==-1){
          slot[0] = 0;
          rcount--;
          send = 0;
        }else{
          recieve_array[0] = 1;
        }
      }if(index[i] == 1){
        
        worker_range[0] = buffer2[0];
        worker_range[1] = buffer2[1];

        if(worker_range[0]==-1){
          slot[1] = 0;
          rcount--;
          send = 0;
        }else{
          recieve_array[1] = 1;
        }
      }if(index[i] == 2){
        
        worker_range[0] = buffer3[0];
        worker_range[1] = buffer3[1];

        if(worker_range[0]==-1){
          slot[2] = 0;
          rcount--;
          send = 0;
        }else{
          recieve_array[2] = 1;
        }
      }

      if(send == 1){
        sum = num_int(functionid, a, worker_range, intensity, mid);
        MPI_Send(&sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
      }

    }
    if(rcount == 0){
      break;
    }
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
