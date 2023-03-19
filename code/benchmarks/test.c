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

void red(){
	printf("red\n");

}

int main(int argc, char **argv) {

	/* Implement HERE */
	worker_t wack = 0; 
	printf("passmain0\n");
	worker_create(&wack,NULL,red, NULL );
	//insert_list(NULL, NULL);

	return 0;
}

