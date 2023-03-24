// File:	worker_t.h

// List all group member's name: William Chen
// username of iLab: whc27
// iLab Server: ilab4

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1
#define THREADCOUNT 10000
#define QUANTUM 10000 //usec = 10 ms

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>
#include <stdatomic.h>


typedef uint worker_t;

typedef enum statuses{
	
	RUN, READY, ERROR, EXIT, WAIT, FREE, ZOMBIE, MAIN

} stats; 

typedef struct TCB {
	/* add important states in a thread control block */
	// thread Id
		worker_t threadID; 
	// thread status
		stats status; 
	// thread context
		ucontext_t* context; 
	// thread stack
		void* stack; 
	// thread priority
		unsigned int priority; 
	// And more ...
		void* exitvals; 
	//thread waiting on this thread 
		int waiter; 

		unsigned int quantused; 

		struct worker_mutext_t* waitmut; 

	// YOUR CODE HERE
} tcb; 

typedef struct node_t{

	worker_t threadnum; 
	tcb* thread; 
	struct node_t* next; 

}node_t; 

typedef struct linked_t{

	size_t size;
	node_t* head; 
	node_t* tail; 

}linked_t; 


typedef struct staticthreadarr{

	tcb** array; 
	unsigned int size; 
	linked_t* freelist; 

} threadarr; 


/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */

	int thread; 

	linked_t* lockq;

	// YOUR CODE HERE
} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE





/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);


//user functions 




/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
