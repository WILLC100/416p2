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
static tcb* mainTCB; 
static linked_t* runq; 
static threadarr* thrdarr; 
static linked_t* freeli; 
struct sigaction sa; 
static linked_t* zombielist; 
 
 struct itimerval timer;


void insert_list(node_t* node, linked_t* list); 
static void schedule();
static node_t* list_pop(linked_t* list);
static node_t* list_pop_minq(linked_t* list);
static void basicsched();
node_t* create_node(tcb* thread);
void worker_init();
void insert_front_list(node_t* node, linked_t* list);
static void sched_psjf();
static void print_list(linked_t* list);

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
					//printf("pass-1\n");
		if(!schedulerinit){
			//SCHEDULER CONTEXT 
			worker_init(); 
			schedulerinit = true;  
		}

	
       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread

	   	void* stack = malloc(SIGSTKSZ); 
		ucontext_t* currentctx = malloc(sizeof(ucontext_t));
		getcontext(currentctx);
		
       // - allocate space of stack for this thread to run
		
		tcb* control_block  = malloc(sizeof(tcb)); 
		control_block->stack = stack; 
		currentctx->uc_stack.ss_sp = stack;
		sigemptyset(&(currentctx->uc_sigmask));
		currentctx->uc_stack.ss_size = SIGSTKSZ; 
		currentctx->uc_flags = 0;
		control_block->context = currentctx; 
		control_block->threadID = THREAD_ID; 
		*thread = THREAD_ID;
		 
		if(THREAD_ID < THREADCOUNT){ //reminder: must handle case above threadcount and reassign threadID
			thrdarr->array[THREAD_ID] = control_block;
		}else{
		}
		
		THREAD_ID++; 
		control_block->quantused = 0;
		control_block->status = READY; 
		control_block->priority = 0; 
		control_block->exitvals = NULL; 
		control_block->waiter = -1;
		
		//printf("Makecont\n");
		makecontext(currentctx, (void (*) (void)) function, *((int*)arg)); 
	//printf("Makecont\n");
		insert_list(create_node(control_block), runq);
	//	printf("status %d, ThreadID %d\n", mainTCB->status, mainTCB->threadID);
       // after everything is set, push this thread into run queue and 	
       // - make it ready for the execution.

		printf("IN CREATE: current thread %d\n PASSING TO SCHEDULER \n", currentTCB->threadID);
		
	//	printf("status %d, ThreadID %d\n", mainTCB->status, mainTCB->threadID);
		swapcontext(currentTCB->context, schedulectx); 
       //printf("pass4\n"); 
	   // YOUR CODE HERE
	
    return 0;
};

node_t* create_node(tcb* thread){
	node_t* currentn = malloc(sizeof(node_t));
	currentn->threadnum = thread->threadID; 
	currentn->thread = thread; 
	currentn->next = NULL; 

	return currentn;
}


void insert_list(node_t* node, linked_t* list){
	//printf("	INSERT LIST : THREAD %d, Status %d  \n", node->thread->threadID, node->thread->status );
	//printf("List Head : %p\n", list->head);

	if(list->size == 0){
		list->head = node; 
		list->tail = node; 
	}else if (list->size == 1){
		list->head->next = node; 
		list->tail = node; 
	}else{
		list->tail->next = node; 
		list->tail = node; 
		
	}
	node->next = NULL;
	list->size += 1;

	print_list(list);
	printf(" HEAD  %d, TAIL %d SIZE %ld\n", list->head->threadnum, list->tail->threadnum, list->size);
	//printf(" HEAD NEXT %p \n", list->head->next);
	//	printf("	INSERT LIST2 : NULL HEAD THREAD %d, Status %d  \n",list->head->thread->threadID, node->thread->status);
	//	printf("	INSERT LIST3 : NULL TAIL THREAD %d, Status %d  \n", list->tail->thread->threadID, node->thread->status);
	
}

void insert_front_list(node_t* node, linked_t* list){

	node->next = list->head; 
	list->head = node; 
	list->size +=1;
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
	
	//printf("IN EXIT\n");
	if(value_ptr != NULL){
		currentTCB->exitvals = value_ptr;
	}

	

	insert_list(create_node(currentTCB), freeli);
	//printf("Free List H %p\n", freeli->head);
	
	//if a thread is being waited on, must indicate that the waitee is exited?
	if(currentTCB->waiter > -1){
		tcb* waiterthr = thrdarr->array[currentTCB->waiter];
		printf("IN EXIT THREAD ID WAITER : %d\n", waiterthr->threadID);
		waiterthr->status = READY; 
	}
	printf("IN EXIT : RUNQ Head : %p\n", runq->head);
	printf("EXIT : Current Context %d\n EXIT TO SCHEDULE \n", currentTCB->threadID);

	free(currentTCB->stack);
	free(currentTCB->context);
	currentTCB->status = ZOMBIE; 
	setcontext(schedulectx);
	printf("IN Exit: Passed setcontext\n");

	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
};

 
/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	 printf(" JOINING TWO THREADS : CALLED BY %d, WAITING ON %d \n ", currentTCB->threadID, thread);
	
	tcb* waiteethread = thrdarr->array[thread]; 

	if(value_ptr != NULL){
		*value_ptr = waiteethread->exitvals;
	}

	if(waiteethread->status != ZOMBIE){

		printf("WAITING ON THREAD %d \n STATUS %d \n", waiteethread->threadID, waiteethread->status);
		currentTCB->status = WAIT; 
		waiteethread->waiter = currentTCB->threadID; 
		printf("			wackjoin + %d\n waitee : %d\n", waiteethread->waiter, thread);
		
	}else{
		currentTCB->status = READY;  
	} 

	
	
	//waitee thread always is in the free list at this point. remember to update the tcb pointer in the free list
	//free(waiteethread);
	//printf("wackjoin + %d\n", currentTCB->status);
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
	/*
	*/

		mutex = malloc(sizeof(worker_mutex_t));
		mutex->lockq = malloc(sizeof(linked_t));
		mutex->lockq->head = NULL;
		mutex->lockq->tail = NULL; 
		mutex->lockq->size = 0;

		mutex->thread = -1; 


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
	
	
	//basicsched(); // FCFS
	 

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	/* if (SCHED == PSJF)
			sched_psjf();
		else if (SCHED == MLFQ)
	 		sched_mlfq(); */

	// YOUR CODE HERE
		
// - schedule policy
#ifndef MLFQ
	// Choose PSJF
	sched_psjf();
#else 
	// Choose MLFQ

#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)
	while(1){
		printf("\n SCHEDULER \n");
//	printf("RUNQ %p\n", runq->head);
	print_list(runq);
		node_t* currentnode = list_pop_minq(runq); //must perform list search, cannot pop immediately
		
		print_list(runq);
		currentTCB = currentnode->thread; 
		printf("\n	START SCHED NODE  %d\n", currentnode->threadnum);
		if(currentTCB->status == READY || currentTCB->status == MAIN){
			//printf("THREAD %d STATUS %d\n", currentTCB->threadID, currentTCB->status);
			currentTCB->status = RUN; 
			printf(" PRIOR SWAP THREAD %d STATUS %d\n", currentTCB->threadID, currentTCB->status);
			
			
			setitimer(ITIMER_PROF, &timer, NULL);
			
			swapcontext(schedulectx, currentTCB->context);
			
			//timer.it_value.tv_usec = 0;
			//setitimer(ITIMER_PROF, &timer, NULL);

			currentTCB->quantused += 1;
			printf("AFTER SWAP THREAD %d STATUS %d\n", currentTCB->threadID, currentTCB->status);
			
			if(currentTCB == mainTCB){
				insert_list(currentnode, runq);

				printf("MAIN INSERTED\n");
				continue; 
			}
		
		}
		


		//if zombie(exited) or wait(blocked), insert into appropriate list
		if(currentTCB->status == ZOMBIE){
			insert_list(currentnode, zombielist);
			printf("ZOMBIED : %d\n", currentnode->threadnum);
		}
		if( currentTCB->status == WAIT){

			if(runq->head == NULL){
				currentTCB->status = READY;
			}
			
			insert_list(currentnode, runq);
			printf("WAITING\n");
			
			//exit(0);
		}
		//if thread has not terminated, insert into the list inorder location.
		if(currentTCB->status == RUN){
				currentTCB->status = READY;
			 /*	if(runq->head == NULL){
					insert_list(currentnode, runq);
					printf("NULL HEAD EXCHANGED CURRENT HEAD \n");
				} */
				insert_list(currentnode, runq);
				printf("AFTER INSERT HEAD ID %d HEAD Qu %d, Current ID %d Current Qu %d \n ", 
				runq->head->threadnum, runq->head->thread->quantused, 
				runq->tail->threadnum, runq->tail->thread->quantused);

				
		}


	}



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

 node_t* list_pop(linked_t* list){

		node_t* topop = list->head; 
		
	//	printf("	POP STATUS %d THREAD %d\n", list->head->thread->status , list->head->thread->threadID);
		if(list->head != NULL){
			list->head = topop->next;
			topop->next = NULL;
			//printf("Next %p\n", topop->next);
		}
		
	//	printf("POPPED %d\n", topop->threadnum);
		return topop; 

 }

unsigned int find_min_index(linked_t* list){

	unsigned int index = 0;
	unsigned int counter = 0;
	node_t* current = list->head; 

	unsigned int min = __INT_MAX__;

	while(current != NULL){

		if(current->thread->quantused < min){
			min = current->thread->quantused;
			index = counter; 

		}

		counter++;
		current = current->next; 
	}

	return index; 
}

node_t* list_pop_index(linked_t* list, unsigned int index){

	node_t* pop = NULL;
	node_t* current = list->head; 
	//printf("POP POP\n");
	if(index == 0){
		pop = list->head; 
		list->head = list->head->next; 

		return pop;
	}
	
	for(int i =0; i<index-1; i++){
		current = current->next; 
	}

	pop = current->next; 
	//printf("POP POP\n");
	current->next = current->next->next; 
	
	return pop;
}

static void print_list(linked_t* list){

	node_t* current = list->head; 
printf("\n LIST ");
	while(current != NULL){
		
		printf(" >>> %d ", current->threadnum);
		current = current->next; 
	}
	printf(" \n ");

	
	//exit(0);
}

static node_t* list_pop_minq(linked_t* list){

	
	node_t* pop = NULL;

	if(list->size == 0){
		return NULL;
	}
	//printf("POP POP\n");
	unsigned int mindex = find_min_index(list);
//	printf("LIST MININDEX %d\n", mindex);
	pop = list_pop_index(list, mindex);

	list->size -=1;

//	print_list(list);
	printf("POPPED THREAD %d, QU %d LIST SIZE %ld\n", pop->thread->threadID, pop->thread->quantused, list->size);
	pop->next = NULL;
	return pop; 

}



static void basicsched(){
	while(1){
		//printf("status %d, ThreadID %d\n", mainTCB->status, mainTCB->threadID);
		//setitimer(ITIMER_PROF, &timer, NULL);
		printf("IN SCHED : List Head : %p\n", runq->head);
	//	printf("	STATUS %d THREAD %d\n", runq->head->thread->status , runq->head->thread->threadID);
		
		node_t* currentnode = list_pop(runq);
		currentTCB = currentnode->thread; 
		//printf("Next %p\n", currentnode);
		printf("STATUS %d THREAD %d\n", currentTCB->status, currentnode->threadnum);

		if(currentTCB->status == READY || currentTCB->status == RUN){
			
			currentTCB->status = RUN; 
			printf("IN SCHED : List Head : %p\n  Current Context %d\n", runq->head, currentTCB->threadID);
			
		
			swapcontext(schedulectx, currentTCB->context);
			
			

			if(currentTCB == mainTCB ){
				insert_front_list(currentnode, runq);
			} else if(currentTCB->status == RUN || currentTCB->status == READY){
				currentTCB->status = READY; 
				insert_list(currentnode, runq);
			}
		}
		
		if( currentTCB->status == ZOMBIE){
			insert_list(currentnode, zombielist);

		}
		
		if( currentTCB->status == WAIT){
			insert_list(currentnode, runq);
		}


		printf("List Head : %p\n", runq->head);
	}

	printf("IN Scheduler :prior swap to main\n");
	 	//printf("MainTCB %p\n", mainTCB->context);
		//printf("MainTCB %p\n", thrdarr->array[0]->context);
		//printf("Schedulectx %p\n", schedulectx);
	setcontext(mainTCB->context);
	printf("IN Scheduler: After swap to main context\n");
	
}

void ring(int signum){
	//printf("TIMER DEATH\n");
	swapcontext(currentTCB->context, schedulectx);
	
}

//initialization
void worker_init(){

	
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &ring;
	//sa.sa_flags = 0;
	//sigfillset(&(sa.sa_mask));
	sigaction (SIGPROF, &sa, NULL);

	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 0;

	timer.it_value.tv_usec = QUANTUM;
	timer.it_value.tv_sec = 0;

	//setitimer(ITIMER_PROF, &timer, NULL);

	//sleep(1);

	//ZOMBIE LIST
	zombielist = malloc(sizeof(linked_t));
	zombielist->size = 0;
	zombielist->head = NULL;
	zombielist->tail = NULL; 

	//TCB ARRAY SETUP
	thrdarr = malloc(sizeof(threadarr));
	thrdarr->array = malloc(sizeof(tcb*)*THREADCOUNT);
	thrdarr->size = 0; 
	
	thrdarr->freelist = malloc(sizeof(linked_t));
	thrdarr->freelist->size = 0;
	thrdarr->freelist->head = NULL;
	thrdarr->freelist->tail = NULL; 

	freeli = thrdarr->freelist;
			
	//List Setup
			runq = malloc(sizeof(linked_t)); 
			//printf("%p\n", runq); 
			runq->size = 0;
			runq->head = NULL;
			runq->tail = NULL; 

	//Scheduler Setup 
	schedulectx = malloc(sizeof(ucontext_t));
	getcontext(schedulectx);
	schedulectx->uc_stack.ss_sp = malloc(SIGSTKSZ);	
	sigemptyset(&(schedulectx->uc_sigmask));
	makecontext(schedulectx, schedule, 0);

	//Main Thread Setup 
			mainTCB = malloc(sizeof(tcb)); 
			ucontext_t* mainctx = malloc(sizeof(ucontext_t)); 
			getcontext(mainctx); 
			mainTCB->context = mainctx; 
			mainTCB->stack = (mainctx->uc_stack.ss_sp); 
			sigemptyset(&(mainctx->uc_sigmask));
			//printf("Stack %ld ", mainctx->uc_stack.ss_size);
			
			mainTCB->threadID = THREAD_ID; 
			thrdarr->array[THREAD_ID] = mainTCB;
			printf("Thread ID %d\n", THREAD_ID);

			THREAD_ID++; 
			mainTCB->quantused = 0;
			mainTCB->priority = 0; 
			mainTCB->exitvals = NULL;
			mainTCB->status = MAIN;  
		//	printf("status %d, ThreadID %d\n", mainTCB->status, mainTCB->threadID);
			mainTCB->waiter = -1; 
			currentTCB = mainTCB;
		//	printf("status %d, ThreadID %d\n", mainTCB->status, mainTCB->threadID);
			insert_list(create_node(mainTCB), runq);
		//	printf("AFTER LISTstatus %d, ThreadID %d\n", runq->head->thread->status, runq->head->thread->threadID);
	
	
}