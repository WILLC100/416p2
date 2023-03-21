// File:	thread-worker.c

// List all group member's name: William Chen
// username of iLab: whc27
// iLab Server: 4

#include "thread-worker.h"

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
static worker_t THREAD_ID = 0; 
static bool schedulerinit = false; 
static ucontext_t* schedulectx; //context scheduler
static tcb* currentTCB; //initialized to main thread, main thread always has 0 as ID.
static linked_t* runq; 
static threadarr* thrdarr; 
static linked_t* freeli; 

static void schedule();

//Initialization 
void worker_init(){

	//TCB ARRAY SETUP
	thrdarr = malloc(sizeof(threadarr));
	thrdarr->array = malloc(sizeof(tcb*)*THREADCOUNT);
	thrdarr->size = 0; 
	
	thrdarr->freelist = malloc(sizeof(linked_t));
	thrdarr->freelist->head = NULL;
	thrdarr->freelist->tail = NULL; 

	freeli = thrdarr->freelist;


	//Scheduler Setup 
	schedulectx = malloc(sizeof(ucontext_t));
	getcontext(schedulectx);
	schedulectx->uc_stack.ss_sp = malloc(SIGSTKSZ);	
	makecontext(schedulectx, schedule, 0);
	schedulerinit = true; 
			//printf("pass0\n");

	//Main Thread Setup 
			tcb* maintcb = malloc(sizeof(tcb)); 
			ucontext_t* mainctx = malloc(sizeof(ucontext_t)); 
			getcontext(mainctx); 
			maintcb->context = mainctx; 
			maintcb->threadID = THREAD_ID; 
			thrdarr->array[THREAD_ID] = maintcb;

			THREAD_ID++; 
			maintcb->stack = (mainctx->uc_stack.ss_sp); 
			maintcb->priority = 0; 
			maintcb->exitvals = NULL;
			maintcb->status = RUN;  
			maintcb->waiter = -1; 
			currentTCB = maintcb;

			
			
	//List Setup
			runq = malloc(sizeof(linked_t)); 
			//printf("%p\n", runq); 
			runq->head = NULL;
			runq->tail = NULL; 
			insert_list(maintcb, runq);
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
					//printf("pass-1\n");
		if(!schedulerinit){
			//SCHEDULER CONTEXT 
			worker_init(); 
		}

		//printf("PASSED IF\n");
       // - create Thread Control Block (TCB)
	   
       // - create and initialize the context of this worker thread

	   	void* stack = malloc(SIGSTKSZ); 
		ucontext_t* currentctx = malloc(sizeof(ucontext_t));
		currentctx->uc_stack.ss_sp = stack;
		getcontext(currentctx); 
		makecontext(currentctx, function, (int)(long)arg); 
		//printf("pass2\n"); 
       // - allocate space of stack for this thread to run
		
		tcb* control_block  = malloc(sizeof(tcb)); 
		control_block->stack = stack; 
		control_block->context = currentctx; 
		control_block->threadID = THREAD_ID; 

		if(THREAD_ID < THREADCOUNT){ //reminder: must handle case above threadcount and reassign threadID
			thrdarr->array[THREAD_ID] = control_block;
		}else{


		}
		
		THREAD_ID++; 
		control_block->status = READY; 
		control_block->priority = 0; 
		control_block->exitvals = NULL; 
		control_block->waiter = -1;


       // after everything is set, push this thread into run queue and 	
       // - make it ready for the execution.
		insert_list(control_block, runq);
		swapcontext(currentTCB->context, schedulectx); 
       //printf("pass4\n"); 
	   // YOUR CODE HERE
	
    return 0;
};

void insert_list(tcb* thread, linked_t* list){
	//printf("done\n");

	node_t* currentn = malloc(sizeof(node_t));
	currentn->threadnum = thread->threadID; 
	currentn->thread = thread; 
	currentn->next = NULL; 
	
	if(list->head == NULL){
		list->head = currentn; 
		list->tail = currentn; 
	}else{

		list->tail->next = currentn; 
		list->tail = currentn; 
	}

	
}

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	
	currentTCB->status = READY; 
	swapcontext( currentTCB->context, schedulectx); 

	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	// YOUR CODE HERE
	
	return 0;
};


/* terminate a thread */
void worker_exit(void *value_ptr) {

	if(value_ptr != NULL){
		currentTCB->exitvals = &value_ptr;
	}

	currentTCB->status = EXIT; 
	free(currentTCB->stack);
	free(currentTCB->context);

	insert_list(currentTCB, freeli);
	
	//if a thread is being waited on, must indicate that the waitee is exited?
	if(currentTCB->waiter > -1){
		tcb* waiterthr = thrdarr->array[currentTCB->waiter];
		waiterthr->status = READY; 
	}
	
	getcontext(schedulectx);


	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
};

 
/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {

	tcb* waiteethread = thrdarr->array[thread]; 

	if(waiteethread->exitvals != NULL){
		value_ptr = waiteethread->exitvals;
	}

	if(waiteethread->status != EXIT){
		currentTCB->status = WAIT; 
		waiteethread->waiter = currentTCB->threadID; 
		printf("wackjoin + %d\n", currentTCB->status);
		swapcontext(currentTCB->context, schedulectx);
	} 

	currentTCB->status = READY;  
	
	//waitee thread always is in the free list at this point. remember to update the tcb pointer in the free list
	free(waiteethread);
	printf("wackjoin + %d\n", currentTCB->status);
	swapcontext(currentTCB->context, schedulectx);
 
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	// YOUR CODE HERE

	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE
		printf("HELLO WORLD\n");

	 
			swapcontext(schedulectx,thrdarr->array[0]->context);
		 	printf("wackscheduler\n");
		
		exit(0);
// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

// YOUR CODE HERE

 