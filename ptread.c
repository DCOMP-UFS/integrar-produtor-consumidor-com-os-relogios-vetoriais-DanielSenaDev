/* File:  
 *    pth_pool.c
 *
 * Purpose:
 *    Implementação de um pool de threads
 *
 *
 * Compile:  gcc -g -Wall -o pool_rvet pool_rvet.c -lpthread -lrt
 * Usage:    ./pool_rvet
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define THREAD_NUM 6    // Tamanho do pool de threads
#define BUFFER_SIZE 6 // Númermo máximo de tarefas enfileiradas


typedef struct Clock { 
   int p[4];
} Clock;

Clock taskQueue[BUFFER_SIZE];
int taskCount = 0;

pthread_mutex_t mutex;

pthread_cond_t condFull;
pthread_cond_t condEmpty;

void Event(int pid, Clock *clock){
   clock->p[pid]++;
  printf("%d %d %d event Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);
}


void Send(int pid, Clock *clock,int pid2){
   clock->p[pid]++;
  MPI_Send(clock->p, 4, MPI_INT, pid2, 0, MPI_COMM_WORLD); 
  printf("%d %d %d send Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);
}

void Receive(int pid, Clock *clock,int pid2){
   clock->p[pid]++;
   int p[4];
   
   MPI_Recv(p, 4, MPI_INT, pid2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   if(clock->p[0]<p[0])
   clock->p[0]=p[0]; 
    if(clock->p[1]<p[1])
   clock->p[1]=p[1]; 
    if(clock->p[2]<p[2])
   clock->p[2]=p[2]; 
  printf("%d %d %d recieve Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);

}
void criarThreads(int n){
   for (int i = 0; i < (THREAD_NUM/3)-1; i++){  
       if(i%2==0)
      if (pthread_create(&thread[i], NULL, &startProducerThread,n) != 0) {
         perror("Failed to create the thread");
      }  
      else
       if (pthread_create(&thread[i], NULL, &startConsumerThread,n) != 0) {
         perror("Failed to create the thread");
      } 
   }
}


// Representa o processo de rank 0
void process0(){
   criarThreads();
   Clock clock = {{0,0,0}};
   Event(0, &clock);
    Send(0, &clock,1);
    Receive(0, &clock,1);
   Send(0, &clock,2);
    Receive(0, &clock,2);
   Send(0, &clock,1);
   Event(0, &clock);
   printf("Process: %d, Clock: (%d, %d, %d)\n", 0, clock.p[0], clock.p[1], clock.p[2]);

   // TO DO

}


// Representa o processo de rank 1
void process1(){
   criarThreads();
   Clock clock = {{0,0,0}};
   Send(1, &clock,0);
   Receive(1, &clock,0);
   Receive(1, &clock,0);
   printf("Process: %d, Clock: (%d, %d, %d)\n", 1, clock.p[0], clock.p[1], clock.p[2]);
   
   // TO DO
}

// Representa o processo de rank 2
void process2(){
   criarThreads();
   Clock clock = {{0,0,0}};
   Event(2, &clock);
   Send(2, &clock,0);
   Receive(2, &clock,0);
   printf("Process: %d, Clock: (%d, %d, %d)\n", 2, clock.p[0], clock.p[1], clock.p[2]);
   
   // TO DO
}


void executeTask(Clock* clock, int id){
  printf("%d %d %d executou\n",clock->p[0],clock->p[1],clock->p[2]);
}

Clock getTask(){
   pthread_mutex_lock(&mutex);
   
   while (taskCount == 0){
        printf("vazio\n");
      pthread_cond_wait(&condEmpty, &mutex);
   }
   
   Clock clock = taskQueue[0];
   int i;
   for (i = 0; i < taskCount - 1; i++){
      taskQueue[i] = taskQueue[i+1];
   }
   taskCount--;
   
   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condFull);
   return clock;
}

void submitTask(Clock clock){
   pthread_mutex_lock(&mutex);

   while (taskCount == BUFFER_SIZE){
      printf("cheio\n");
      pthread_cond_wait(&condFull, &mutex);
   }

   taskQueue[taskCount] = clock;
   taskCount++;

   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condEmpty);
}

void *startProducerThread(void* args);  

void *startConsumerThread(void* args);

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   pthread_mutex_init(&mutex, NULL);
   
   pthread_cond_init(&condEmpty, NULL);
   pthread_cond_init(&condFull, NULL);

   pthread_t thread[THREAD_NUM]; 
   srand(time(NULL));
   long i;
   
   int my_rank;               

   MPI_Init(NULL, NULL); 
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   if (my_rank == 0) { 
      process0();
   } else if (my_rank == 1) {  
      process1();
   } else if (my_rank == 2) {  
      process2();
   }


  
   
   
   
   for (int i = 0; i < THREAD_NUM; i++){  
      if (pthread_join(thread[i], NULL) != 0) {
         perror("Failed to join the thread");
      }  
   }
   
   /* Finaliza MPI */
   MPI_Finalize(); 
   
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&condEmpty);
   pthread_cond_destroy(&condFull);
   return 0;
}  /* main */

/*-------------------------------------------------------------------*/
void *startProducerThread(void* args) {
   long id = (long) args; 
   for(int i=0;i<50;i++){
      Clock clock = {rand()%100,rand()%100,rand()%100};
      printf("produziu %d %d %d \n",clock.p[0],clock.p[1],clock.p[2]);
      submitTask(clock);
      sleep(rand()%5);
   }
   return NULL;
} 
void *startConsumerThread(void* args) {
   mpireceive()
   long id = (long) args; 
   for(int i=0;i<50;i++){
      Clock clock = getTask();
      executeTask(&clock, id);
      sleep(rand()%2);
   }
   return NULL;
} 

void *mainThread(void* args) {
   long id = (long) args; 
   for(int i=0;i<50;i++){
      Clock clock = getTask();
      executeTask(&clock, id);
      sleep(rand()%2);
   }
   return NULL;
} 

