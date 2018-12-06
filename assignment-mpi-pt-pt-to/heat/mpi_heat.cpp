#include <mpi.h>
#include <cmath>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace std;
using seconds = chrono::seconds;
using check_time = std::chrono::high_resolution_clock;

#ifdef __cplusplus
extern "C" {
#endif

  int check2DHeat(double** H, long n, long rank, long P, long k);
//this assumes array of array and grid block decomposition

#ifdef __cplusplus
}
#endif

/***********************************************
 *         NOTES on check2DHeat.
 ***********************************************
 *
 *  First of, I apologize its wonky.
 *
 *  Email me ktibbett@uncc.edu with any issues/concerns with this.
Dr. Saule or the other
 *    TA's are not familiar with how it works.
 *
 * Params:
 *  n - is the same N from the command line, NOT the process's part of N
 *  P - the total amount of processes ie what MPI_Comm_size gives you.
 *  k - assumes n/2 > k-1 , otherwise may return false negatives.
 *
 *
 * Disclaimer:
 ***
 *** Broken for P is 9. Gives false negatives, for me it was always
 ***  ranks 0, 3, 6. I have not found issues with 1, 4, or 16, and these
 ***  are what `make test` will use.
 ***
 *
 * Usage:
 *  When code is WRONG returns TRUE. Short example below
 *  if (check2DHeat(...)) {
 *    // oh no it is (maybe) wrong
 *    std::cout<<"rank: "<<rank<<" is incorrect"<<std::endl;
 *  }
 *
 *
 *
 *  I suggest commenting this out when running the bench
 *
 *
 * - Kyle
 *
 *************/


// Use similarily as the genA, genx from matmult assignment.
double genH0(long row, long col, long n) {
  double val = (double)(col == (n/2));
  return val;
}



int main(int argc, char* argv[]) {

  if (argc < 3) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <N> <K>"<<std::endl;
    return -1;
  }

  long N = atol(argv[1]);
  long K = atol(argv[2]);
  MPI_Init(&argc, &argv);

  int rank, size, count_req = 0, local_count = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int root_size = sqrt(size);
  int granularity = N/root_size;

  int division = rank / root_size;
  int remainder  = fmod(rank, root_size);

  int row_start = (rank - (rank - division)) * granularity;  
  int row_end = row_start + (granularity-1);
  int col_start = (rank - (rank - remainder)) * granularity;
  int col_end = col_start + (granularity-1);
  
  double *H = new double[granularity*granularity];
  double *H_K = new double[granularity*granularity];
  
  auto initiated = check_time::now();  

  for(int i =row_start; i<row_end; i++) {
    for(int j = col_start; j<col_end; j++) {
      H_K[local_count]= genH0(i, j, N);
      local_count++;
    }
  }

  for(long iter = 0; iter < K; iter++) {

    int request_array_size = 0, req_count = 0;
    double *send_buf1  = new double[root_size];
    double *send_buf2  = new double[root_size];
    double *send_buf3  = new double[root_size];
    double *send_buf4  = new double[root_size];

    double *rec_buf1  = new double[root_size];
    double *rec_buf2  = new double[root_size];
    double *rec_buf3  = new double[root_size];
    double *rec_buf4  = new double[root_size];

    MPI_Request req[request_array_size];
    
    //get the size for request array dynamically
    if(row_start != 0){
      request_array_size+=2;
    }
    if(row_end != N-1){
      request_array_size+=2;
    }
    if(col_start != 0){
      request_array_size+=2;
    }    
    if(col_end != N-1){
      request_array_size+=2;
    }
    
    if(row_start != 0){
      int col = 0;
      for(int i =0; i<root_size; i++) {
        send_buf2[i]= H_K[col];
        col++;      
      }
      MPI_Isend(send_buf2, root_size, MPI_DOUBLE, rank - root_size, 0, MPI_COMM_WORLD, &req[req_count]);
      req_count++;
      MPI_Irecv(rec_buf2, root_size, MPI_DOUBLE, rank - root_size, 0, MPI_COMM_WORLD, &req[req_count]);
      count_req+=2;
      req_count++;
    }

    if(row_end != N-1){
      int col = row_end;
      for(int i =0; i<root_size; i++) {
        send_buf4[i]= H_K[col];
        col++;      
      }

      MPI_Isend(send_buf4, root_size, MPI_DOUBLE, rank + root_size, 0, MPI_COMM_WORLD, &req[req_count]);
      req_count++;
      MPI_Irecv(rec_buf4, root_size, MPI_DOUBLE, rank + root_size, 0, MPI_COMM_WORLD, &req[req_count]);
      count_req+=2;
      req_count++;
    }

    if(col_start != 0){
      int row = 0;
      for(int i =0; i<root_size; i++) {
        send_buf1[i]= H_K[row];
        row+=granularity;      
      }
      MPI_Isend(send_buf1, root_size, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count]);
      req_count++;
      MPI_Irecv(rec_buf1, root_size, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_count]);
      count_req+=2;
      req_count++;
    }

    if(col_end != N-1){
      int row = col_end;
      for(int i =0; i<root_size; i++) {
        send_buf3[i]= H_K[row];
        row+=granularity;      
      }

      MPI_Isend(send_buf3, root_size, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count]);
      req_count++;
      MPI_Irecv(rec_buf3, root_size, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_count]);
      count_req+=2;
      req_count++;      
    }

    MPI_Waitall(request_array_size, req, MPI_STATUSES_IGNORE);
    
    int new_local_count = 0;
    for(int i = 0; i< granularity; i++) {
      for(int  j = 0; j<granularity; j++) {

        if(i == 0 && j==0) {
          if(row_start == 0 && col_start ==0){
            H[new_local_count] = (H_K[1] + H_K[(i+1)*granularity+j] +  (3 * H_K[i*granularity+j])) / 5;
          }else{
            H[new_local_count] = (H_K[1] + H_K[(i+1)*granularity+j] +  H_K[i*granularity+j] + rec_buf1[0] + rec_buf2[0]) / 5;
          }          
        }else if(i==granularity-1 && j==granularity-1) {
          if(row_end == N-1 && col_end ==N-1){
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j] + (3 * H_K[i*granularity+j]))/5;
          }else{
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j] + H_K[i*granularity+j] + rec_buf3[i] + rec_buf4[i])/5;
          }
        }else if(i==0 && j!=granularity-1) {
          if(row_start == 0 && col_start !=N-1){
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[i*granularity+(j-1)] + H_K[(i+1)*granularity+j]
             + (2 * H_K[i*granularity+j]))/5;
          }else{
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[i*granularity+(j-1)] + H_K[(i+1)*granularity+j]
             + H_K[i*granularity+j] + rec_buf2[j])/5;
          }
        }else if(i!=0 && j==0) {
          if(row_start != 0 && col_start ==0){
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[(i-1)*granularity+j] + H_K[(i+1)*granularity+j]
             + (2 * H_K[i*granularity+j]))/5;                
          }else{
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[(i-1)*granularity+j] + H_K[(i+1)*granularity+j]
             + H_K[i*granularity+j] + rec_buf1[i])/5;   
          }
        }else if(i!=N-1 && j==N-1) {
          if(row_end != N-1 && col_end ==N-1){
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j] + H_K[(i+1)*granularity+j]
             + (2 * H_K[i*granularity+j]))/5;               
          }else{
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j] + H_K[(i+1)*granularity+j]
             + H_K[i*granularity+j] + rec_buf3[i])/5;  
          }
        }else if(i==N-1 && j!=N-1) {
          if(row_end == N-1 && col_end !=N-1){
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j]
             + (2 * H_K[i*granularity+j]))/5;           
          }else{
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[i*granularity+(j-1)] + H_K[(i-1)*granularity+j]
             + H_K[i*granularity+j] + rec_buf4[j])/5;
          }
        }else if(i==0 && j==N-1) {
          if(row_start == 0 && col_end ==N-1){
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i+1)*granularity+j] + (3 * H_K[i*granularity+j]))/5;
          }else{
            H[new_local_count]= (H_K[i*granularity+(j-1)] + H_K[(i+1)*granularity+j] + H_K[i*granularity+j] + rec_buf2[j] + rec_buf3[i])/5;
          }
        }else if(i==N-1 && j==0) {
          if(row_end == N-1 && col_start ==0){
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[(i-1)*granularity+j] + (3 * H_K[i*granularity+j]))/5;
          }else{
            H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[(i-1)*granularity+j] + H_K[i*granularity+j] + rec_buf1[i] + rec_buf4[j])/5;
          }
        }else{
          H[new_local_count]= (H_K[i*granularity+(j+1)] + H_K[(i-1)*granularity+j] + H_K[i*granularity+j] +
           H_K[(i+1)*granularity+j] + H_K[i*granularity+(j-1)])/5;
        }
        new_local_count++;
      }
    }

/*double **pointer = &H;
if(check2DHeat(pointer, N, rank, size, iter)){
cout<<"wrong"<<endl;
}else{
  cout<<"right"<<endl;
}*/

    H_K = H;
  }
  if(rank == 0){

    auto end = check_time::now();
    auto time_elapsed = end - initiated;
    auto secs = std::chrono::duration_cast<std::chrono::duration<float>>(time_elapsed);
    cerr<<secs.count();
  }
  MPI_Finalize();
  return 0;
}