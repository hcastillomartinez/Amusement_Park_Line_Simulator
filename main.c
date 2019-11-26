#include <stdio.h>
#include <stdlib.h>
#include "random437.h"
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#define START 32400
// debugging end (12 minutes)
#define END 33120
// #define END 68400
#define MAX_QUEUE 800
static int current_time = START;
static int incoming_user = 0;
static int total_incoming = 0;
static int total_rejected = 0;
static int time_step = 0;
static int queue = 0;
static int maximum_queue = 0;
static int max_q_time = 0;
static int max_passengers = 7;
static int total_rode = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t car_cond = PTHREAD_COND_INITIALIZER;

/*
Used to generate the # of incoming people based
on what the current_time is.
*/
void incomingP(){
  if(current_time >=32400 && current_time <= 39599){
      incoming_user = 25;
    }
    else if(current_time >=39600 && current_time <= 50399){
      incoming_user = 45;
    }
    else if(current_time >=50400 && current_time <= 57599){
      incoming_user = 35;
    }
    else{
      incoming_user = 25;
    }
}

/*
Just increments time as fast as it can at the moment
does not really map virtual minute to real second.
*/
void *time_passage(){
  sleep(1);
  while(current_time < END){

    pthread_mutex_lock(&lock);
    printf("time enter\n");
    int hours = current_time/3600;
    int min = (current_time%3600)/60;
    int secs = (current_time%3600)%60;
    // printf("%d:%d:%d\n",hours,min,secs);
    current_time+=60;
    time_step++;
  
    pthread_cond_signal(&cond);
    pthread_cond_signal(&car_cond);
    printf("p exit\n");
    pthread_mutex_unlock(&lock);
    sleep(1);
  }
  return NULL;
}

/*
This handles incoming people
*/
void *incomingP_handler(){
  while(current_time < END){
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond,&lock);
    printf("p enter\n");
    int hours = current_time/3600;
    int min = (current_time%3600)/60;
    int secs = (current_time%3600)%60;
    incomingP();
    int incoming = poissonRandom(incoming_user);
    total_incoming += incoming;

    if((queue+incoming) <= MAX_QUEUE){
      queue+= incoming;
      printf("%d arrive %d reject %d wait-line %d at %d:%d:%d\n",time_step, incoming, 0, queue, hours, min, secs);
      if (maximum_queue <= queue){
        max_q_time = current_time;
        maximum_queue = queue;
      }
    }
    else{
      int diff = 800 - queue;
      queue += diff;
      total_rejected += diff;
      printf("%d arrive %d reject %d wait-line %d at %d:%d:%d\n",time_step, incoming, incoming-diff, queue, hours, min, secs);
      if (maximum_queue <= queue){
        max_q_time = current_time;
        maximum_queue = queue;
      }
    }
    printf("p exit\n");
    pthread_cond_signal(&car_cond);
    pthread_mutex_unlock(&lock);
    // sleep(1);
  }
  return NULL;
}


/*
  Handles the things that cars needs to do.
*/
void *carHandler(){

  while(current_time < END){
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&car_cond, &lock);
    printf("enter\n");
    int diff = queue - max_passengers;
    if(diff >= 0){
      queue -= max_passengers;
      total_rode += max_passengers;
    }
    else{
      queue -= queue;
      total_rode += max_passengers;
    }
    printf("exit\n");
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}


int main(void) {
  int n = 2;
  pthread_t tid1;
  pthread_t tid2;
  pthread_t cars[n];
  // pthread_mutex_init(&lock, NULL);
  pthread_create(&tid1,NULL,time_passage,(void *)NULL);
  pthread_create(&tid2,NULL,incomingP_handler,(void *)NULL);
  for(int i = 0; i < n; i ++){
    pthread_create(&cars[i], NULL, carHandler, (void *) carHandler);
  }
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  for(int i = 0 ;i < n;i++){
    pthread_join(cars[i], NULL);
  }
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
  int hours = max_q_time/3600;
  int min = (max_q_time%3600)/60;
  int secs = (max_q_time%3600)%60;
  printf("\nTotal Number of Guests: %d, Total Number of People on Ride: %d, Total Number of People Rejected: %d, Average Waiting Time: %d, Maximum Length of Line for Day and Time of Occurence: %d at %d:%d:%d\n", total_incoming, total_rode, total_rejected, time_step, maximum_queue, hours, min, secs);
  return 0;
}