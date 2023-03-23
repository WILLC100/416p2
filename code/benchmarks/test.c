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
	sleep(1);
	worker_exit(NULL);
}
 
void red(){
	printf("red\n");

 
	sleep(1);
	worker_exit(NULL);
}

int main(int argc, char **argv) {

	
	/* Implement HERE */
	worker_t wack = 0;
	printf("IN MAIN : priorcreate\n");

	worker_create(wack,NULL, red , NULL );
	worker_create(wack, NULL, blue, NULL);
	worker_create(wack,NULL, red , NULL );
	worker_create(wack, NULL, blue, NULL);
	worker_create(wack,NULL, red , NULL );
	worker_create(wack, NULL, blue, NULL);
	worker_create(wack,NULL, red , NULL );
	worker_create(wack, NULL, blue, NULL);
	
	//worker_create(&wack,NULL,red, NULL );
	printf(" IN MAIN : passcreate\n");
	//worker_join(1,NULL);
	printf("IN MAIN :passjoin\n");

	free(wack);


	//insert_list(NULL, NULL);

	return 0;
}

