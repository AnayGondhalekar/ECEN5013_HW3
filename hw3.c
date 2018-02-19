#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<sys/syscall.h>
#include<sys/time.h>
#include<string.h>
#include<sys/types.h>
#include <unistd.h>
 
pthread_mutex_t lockup;  
/* Link list node */
struct threadParam
{
char *filename;
};

struct node
{
    int data;
    struct node* next;
};


void insert(struct node** head, char value)
{

    struct node* new_node =
            (struct node*) malloc(sizeof(struct node));

    new_node->data  = value;

    new_node->next = (*head);

    (*head)    = new_node;
}

/* Counts the no. of occurences of a node in a linked list */
int count(struct node* start, char item)
{     
    struct node* current = start;
    int count = 0;
    while (current != NULL)
    {
        if ((current->data == item) ||  (current->data == item + 32))
           count++;
        current = current->next;
    }
   
   return count;
}

void* parsing_function(void *arg)
{

  struct threadParam *thread1 = (struct threadParam*) arg; 
  char c;
  int x;
  FILE *fptr;
  time_t curtime;
  time(&curtime);
  fptr = fopen(/*thread1 -> filename*/"Valentinesday.txt","r");
  struct node* start = NULL;
  while (!feof(fptr))
  {  char ins;

   ins = fgetc(fptr);
   insert(&start, ins);
  }
   fclose(fptr);
   fptr = fopen(thread1 -> filename,"w");
   pthread_mutex_lock(&lockup);
   fprintf(fptr,"Thread 1 executing\nCurrent time= %s",ctime(&curtime));
   fprintf(fptr,"Process id:%d ",getppid());
   fprintf(fptr,"Posix Thread id:%d ",getpid());
   fprintf(fptr,"Linux Thread id:%ld\n",syscall(SYS_gettid));
   pthread_mutex_unlock(&lockup);
   for (c = 'A'; c<= 'Z'; c++)
   {
       x = count(start,c);
       if (x==3)
       { 
         pthread_mutex_lock(&lockup);
         fprintf(fptr,"Character occuring thrice is %c. \n", c);
         pthread_mutex_unlock(&lockup);
       }
   }
   pthread_mutex_lock(&lockup);
   fprintf(fptr,"Exiting Thread 1\nCurrent time= %s",ctime(&curtime));
   pthread_mutex_unlock(&lockup);
   fclose(fptr);
   pthread_exit(NULL);
   return fptr;

}

void timer_handler(int var)
{  time_t curtime;
   time(&curtime);
   FILE *fp = fopen(/*thread2 -> filename*/"log.txt","a+"); 
   pthread_mutex_lock(&lockup);
   fprintf(fp,"Thread 2 executing.\n"); 
   fprintf(fp,"Current time= %s",ctime(&curtime));
   pthread_mutex_unlock(&lockup);
   if( var == SIGVTALRM)
 {
   char *util = "cat /proc/stat | head -n 3";
   FILE * ptr = popen(util,"r");
   if(ptr)
    {
    pthread_mutex_lock(&lockup);
    fprintf(fp,"Process id:%d ",getppid());
    fprintf(fp,"Posix Thread id:%d ",getpid());
    fprintf(fp,"Linux Thread id:%ld\n",syscall(SYS_gettid));
    pthread_mutex_unlock(&lockup);
    }
   while(!feof(ptr))
   {
      char vol;
      vol = fgetc(ptr);
      //printf("%c",vol);
      pthread_mutex_lock(&lockup);
      fprintf(fp,"%c",vol);
      pthread_mutex_unlock(&lockup);
   } 
   fclose(ptr);
   fclose(fp);
 }  
   else if (var ==SIGUSR1 || var==SIGUSR2)
  {
   printf("USR got for CPU Utilization Thread.\n");
   pthread_exit(NULL);

  }

}
void* utilization_function(void *arg)
{  
   struct threadParam *thread2 = (struct threadParam*) arg;
   struct sigaction sig;
   struct itimerval timer;
   memset(&sig, 0,sizeof(sig));
   sig.sa_handler = &timer_handler;
   sigaction (SIGVTALRM,&sig,NULL);
   sigaction (SIGUSR2,&sig,NULL);    
   sigaction (SIGUSR1,&sig,NULL);   
   timer.it_value.tv_sec = 0;
   timer.it_value.tv_usec = 100000;
   timer.it_interval.tv_sec = 0;
   timer.it_interval.tv_usec = 100000;
   setitimer (ITIMER_VIRTUAL,&timer,NULL);
   while(1);
   pthread_exit(NULL);
   
}


// Main program
int main()
{
   pthread_t id1; 
   pthread_t id2;
   struct threadParam* thread1 =
            (struct threadParam*) malloc(sizeof(struct threadParam));
   pthread_create(&id1,NULL,parsing_function,(void *)thread1);
   thread1 -> filename = "log.txt";
   struct threadParam* thread2 =
            (struct threadParam*) malloc(sizeof(struct threadParam));
   pthread_create(&id2,NULL,utilization_function,(void *)thread2);
   thread2 -> filename = "log.txt";
   printf("My parent's ID: %d\n", getppid());
   /*
   FILE *fptr ;
   fptr = fopen("Valentinesday.txt","r");
   if(fptr == NULL)
	{ 
	printf("Error");
	}
    */
   
   pthread_join(id1,NULL);
   pthread_join(id2,NULL);
  return 0;
}
