#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <cmath>

using namespace std;

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

struct func_details{
  float result, sum;
  int ub, lb,functionid,intensity;
  string sync;
  float a, b, mid;
};
pthread_mutex_t mut;
float total_sum = 0;
void* numerical_integration(void* p){

  func_details *func_info = (func_details*)p;
  if(0 == func_info->sync.compare("iteration")){

  switch(func_info->functionid){
    case 1:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        pthread_mutex_lock(&mut);
        total_sum +=f1(x, func_info->intensity);
	pthread_mutex_unlock(&mut);
      }
      break;

    case 2:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        pthread_mutex_lock(&mut);
        total_sum +=f2(x, func_info->intensity);
	pthread_mutex_unlock(&mut);
      }
      break;

    case 3:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        pthread_mutex_lock(&mut);
        total_sum +=f3(x, func_info->intensity);
	pthread_mutex_unlock(&mut);
      }
      break;

    case 4:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        pthread_mutex_lock(&mut);
        total_sum +=f4(x, func_info->intensity);
	pthread_mutex_unlock(&mut);
      }
      break;
   }
   }else{
  switch(func_info->functionid){
    case 1:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        func_info->sum +=f1(x, func_info->intensity);
      }
      break;

    case 2:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        func_info->sum +=f2(x, func_info->intensity);
      }
      break;

    case 3:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        func_info->sum +=f3(x, func_info->intensity);
      }
      break;

    case 4:
      for (int i=func_info->lb; i<=func_info->ub; i++){
        float x = (func_info->a + (i + 0.5) * func_info->mid);
        func_info->sum +=f4(x, func_info->intensity);
      }
      break;
   }
   }
//total_sum = 2;
  return NULL;
}

int main (int argc, char* argv[]) {

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  if (argc < 7) {
    cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<endl;
    return -1;
  }

  float result;
  int functionid = stoi(argv[1]);
  float a = stof(argv[2]);
  float b = stof(argv[3]);
  int n = stoi(argv[4]);
  int intensity = stoi(argv[5]);
  float mid = ((b - a) / n );
  int nbthreads = stoi(argv[6]);
  string sync = argv[7];
  float sum[nbthreads];
  int computations = n/nbthreads;
  struct func_details func_info[nbthreads];
  pthread_t thread[nbthreads];

  if(0 == sync.compare("iteration")){
    pthread_mutex_init(&mut, NULL);
  }
int j = 0;

for(int i=0;i<nbthreads;i++){
if(j<n){
    func_info[i].a = a;
    func_info[i].b = b;
    func_info[i].functionid = functionid;
    func_info[i].intensity = intensity;
    func_info[i].lb = j;
    func_info[i].mid = mid;
    func_info[i].sum = 0;
    func_info[i].sync = sync;

    if((i+1) >= nbthreads){
      func_info[i].ub = n - 1;
    }else{
     func_info[i].ub = j + (computations-1);
    }  

    pthread_create(&thread[i], NULL, numerical_integration, (void*) &func_info[i]);
  }
j+=computations;
}
  for(int i=0; i < nbthreads; i+=1){
    pthread_join(thread[i], NULL);
  }

  if(0 == sync.compare("iteration")){
    pthread_mutex_destroy(&mut);
  }else{
    for(int i=0;i < nbthreads;i+=1){
      total_sum+= func_info[i].sum;
    }
  }
  cout<<mid * total_sum;
  

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cerr<<elapsed_seconds.count()<<std::endl;
  return 0;
}
