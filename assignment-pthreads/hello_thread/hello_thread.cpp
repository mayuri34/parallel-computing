#include <iostream>
#include <stdio.h>
#include <string>

struct thread_details{
	int no_of_threads;
	int current_thread;
};

void* f(void* p){
 thread_details *thread_info = (thread_details*)p;
printf("I am thread %d in %d \n", thread_info->current_thread,thread_info->no_of_threads);
 //std::cout<<"I am thread "<<thread_info->current_thread<<" in "<<thread_info->no_of_threads<<std::endl;
}

int main (int argc, char* argv[]) {

 if (argc < 2) {
   std::cerr<<"usage: "<<argv[0]<<" <nbthreads>"<<std::endl;
   return -1;
 }

 int noThreads = std::stoi(argv[1]);
 struct thread_details thread_info[noThreads];
 pthread_t nbthread[noThreads];

 for(int i=0; i < noThreads; i+=1){
  thread_info[i].no_of_threads = noThreads;
  thread_info[i].current_thread = i; 
  pthread_create(&nbthread[i], NULL, f, (void*) &thread_info[i]);
 }

 for(int i=0; i < noThreads; i+=1){
  pthread_join(nbthread[i], NULL);
 }
  
 return 0;
}
