#include <stdio.h>
#include <stdlib.h>
#include "random437.h"
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#define START 32400
// debugging end (12 minutes)
// #define END 33120
// #define END 34000
#define END 68400
#define MAX_QUEUE 800
/*
  Human struct created to keep track of when they spawn and
  when they exit the line.
*/
struct Human{
  int spawn_time;
  int wait_time;
};
/*
  Struct that represents a Queue. Only need to track the head and
  the tail, rest can be found through pointers.
*/
struct Queue{
  struct QueueNode *head;
  struct QueueNode *tail;
};
/*
  Struct used to represent a node in the Queue. Consists of a human
  and a pointer to the next node.
*/
struct QueueNode{
  struct Human human;
  struct QueueNode* next;
};
static struct Queue* actual_line;
static struct Queue* total_line;

static int current_time = START;
static int incoming_user = 0;
static int total_incoming = 0;
static int total_rejected = 0;
static int time_step = 0;
static int queue = 0;
static int maximum_queue = 0;
static int max_q_time = 0;
static int max_passengers = 7;
static int num_cars = 6;
static int total_rode = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t car_cond = PTHREAD_COND_INITIALIZER;
FILE *fp;

// fp = fopen("test.txt", "w+");

/*
  Used to create a node for the Queue
  Sets human to track track time it was created.
*/
struct QueueNode* createNode(){
  struct QueueNode* x = malloc(sizeof(struct QueueNode));
  struct Human h = {current_time, 0};
  x->human = h;
  x->next=NULL;
  return x;
}

/*
  Initializes a Queue
*/
struct Queue* initQueue(){
  struct Queue* line = (struct Queue*)malloc(sizeof(struct Queue));
  line->head = NULL;
  line->tail = NULL;
  return line;
}

/*
  Adds a node to the queue. Does pass anythin as we just use the current time to initialize a node's human variables.
*/
void enqueue(struct Queue* line){
  struct QueueNode* t = createNode();
  if (line->tail == NULL){
    line->head = t;
    line->tail = t;
  }
  else{
    line->tail->next=t;
    line->tail = t;
  }
}
/*
  Used when keeping track of removed people. Only called when we are
  person is getting on a ride (dequeueing)
*/
void enqueue2(struct Queue* line,struct QueueNode* node){
  node->human.wait_time = current_time - node->human.spawn_time;
  if (line->tail == NULL){
    line->head = node;
    line->tail = node;
  }
  else{
    line->tail->next=node;
    line->tail = node;
  }
}
/*
  Removes a node from a queue and sets it in another queue. Latter
  Queue is only used for tracking avg time.
*/
struct QueueNode* deQueue(struct Queue* line,struct Queue* total_line) 
{  
    if (line->head == NULL){
      return NULL; 
    }
    else{ 
      struct QueueNode* temp = line->head;
      enqueue2(total_line,temp); 
      // free(temp); 
      line->head = line->head->next; 
      if (line->head == NULL) 
          line->tail = NULL; 
      return temp;
    } 
} 

/*
  Prints the Queue from head to tail.
*/
void printQueue(struct Queue* line){
  struct QueueNode* q= line->head;
  while(q!=NULL){
    printf("spawn_time = %d wait_time = %d\n", q->human.spawn_time, q->human.wait_time);
    q=q->next;
  }
}
/*
  Computes the average wait time of people in the line. Used when people have already gotten off the ride.
*/
int avgWait(struct Queue* line){
  struct QueueNode* q = line->head;
  int exited = 0;
  int minutes_waited = 0;
  while(q!=NULL){
    if(q->human.wait_time !=0){
      minutes_waited += q->human.wait_time;
      exited ++;
    }
    q = q->next;
  }
  return (minutes_waited/exited)/60;
}

/*
  Useful when we are loading in more than 1 person at a time.
*/
void loadIn(struct Queue* actual, struct Queue* total, int toRemove){
  for(int i =0; i<toRemove;i++){
    deQueue(actual,total);
  }
}

/*
  Returns the number of nodes in the queue.
*/
int queueSize(struct Queue* line){
  struct QueueNode* q= line->head;
  int count = 0;
  while(q!=NULL){
    count++;
    q=q->next;
  }
  return count;
}

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
  usleep(10000);
  while(current_time < END){

    pthread_mutex_lock(&lock);
    // printf("time enter\n");
    int hours = current_time/3600;
    int min = (current_time%3600)/60;
    int secs = (current_time%3600)%60;
    // printf("%d:%d:%d\n",hours,min,secs);
    current_time+=60;
    time_step++;
  
    pthread_cond_signal(&cond);
    pthread_cond_broadcast(&car_cond);
    // printf("p exit\n");
    pthread_mutex_unlock(&lock);
    usleep(10000);
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
    int hours = current_time/3600;
    int min = (current_time%3600)/60;
    int secs = (current_time%3600)%60;
    incomingP();
    int incoming = poissonRandom(incoming_user);
    total_incoming += incoming;
    if((queue+incoming) <= MAX_QUEUE){
      queue+= incoming;
      for(int i =0; i < incoming;i++){
        enqueue(actual_line);
      }
      fprintf(fp, "%d arrive %d reject %d wait-line %d at %d:%d:%d\n",time_step, incoming, 0, queue, hours, min, secs);
      if (maximum_queue <= queue){
        max_q_time = current_time;
        maximum_queue = queue;
      }
    }
    else{

      int diff = MAX_QUEUE - queue;
      queue += diff;
      for(int i = 0; i< diff;i++){
        enqueue(actual_line);
      }
      int current_rej = incoming - diff;
      total_rejected += current_rej;
      fprintf(fp, "%d arrive %d reject %d wait-line %d at %d:%d:%d\n",time_step, incoming, current_rej, queue, hours, min, secs);
      if (maximum_queue <= queue){
        max_q_time = current_time;
        maximum_queue = queue;
      }
    }
    // printf("p exit\n");
    // pthread_cond_signal(&car_cond);
    pthread_mutex_unlock(&lock);
    // usleep(10000);
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
    int diff = queue - max_passengers;
    if(diff >= 0){
      queue -= max_passengers;
      loadIn(actual_line, total_line, max_passengers);
      total_rode += max_passengers;
    }
    else{
      queue -= queue;
      loadIn(actual_line, total_line, queue);
      total_rode += queue;
    }
    pthread_cond_broadcast(&car_cond);
    pthread_mutex_unlock(&lock);
    usleep(1000);
  }

  return NULL;
}


int main(void) {
  actual_line = initQueue();
  total_line = initQueue();
  fp = fopen("test.txt", "w+");
  pthread_t tid1;
  pthread_t tid2;
  pthread_t cars[num_cars];
  pthread_mutex_init(&lock, NULL);
  pthread_create(&tid1,NULL,time_passage,(void *)NULL);
  pthread_create(&tid2,NULL,incomingP_handler,(void *)NULL);
  for(int i = 0; i < num_cars; i ++){
    pthread_create(&cars[i], NULL, carHandler, (void *) carHandler);
  }
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  for(int i = 0 ;i < num_cars;i++){
    pthread_join(cars[i], NULL);
  }
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
  pthread_cond_destroy(&car_cond);
  int hours = max_q_time/3600;
  int min = (max_q_time%3600)/60;
  int secs = (max_q_time%3600)%60;
  fprintf(fp,"\nTotal Number of Guests: %d, Total Number of People on Ride: %d, Total Number of People Rejected: %d, Average Waiting Time: %d, Maximum Length of Line for Day and Time of Occurence: %d at %d:%d:%d\n", total_incoming, total_rode, total_rejected, avgWait(total_line), maximum_queue, hours, min, secs);
  fclose(fp);
  return 0;
}