#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

void blue(){
	printf("blue\n"); 
	printf("blue\n"); 
	for(int i =0; i<10000000; i++);
	worker_exit(NULL);
}
 
void red(){
	printf("red\n");

 
	for(int i =0; i<10000000; i++);
	worker_exit(NULL);
}





int main(int argc, char **argv) {

	
	/* Implement HERE */
	worker_t wack[10];
	printf("IN MAIN : priorcreate\n");

	
	worker_create(wack,NULL, red , NULL );
	worker_create(wack+6,NULL, red , NULL );
	worker_create(wack+1, NULL, blue, NULL);
	worker_create(wack+2,NULL, red , NULL );
	worker_create(wack+5, NULL, blue, NULL);
	worker_create(wack+3, NULL, blue, NULL);
	worker_create(wack+4,NULL, red , NULL );
	worker_create(wack+7, NULL, blue, NULL);
printf(" IN MAIN : passcreate\n");
	worker_join(wack[4], NULL);
	worker_join(wack[0], NULL);
	worker_join(wack[1], NULL);
	worker_join(wack[2], NULL);
	worker_join(wack[3], NULL);
	worker_join(wack[5], NULL);
	worker_join(wack[6], NULL);
	worker_join(wack[7], NULL); 


	printf("IN MAIN :passjoin\n");


	return 0;
}

