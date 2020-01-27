#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>



/* Numbers are generated in here, the pids. On a linux 32 bit platform max value for pid is 32768 (5 digits = 5 strlen)  
On 64 bit machines pid max value can be any value up to 2^22 (22 digits  = 22 strlen)

For my implementation of the module I passed-in/used a value of 4 for the maxlength of a string. (I guessed the pid would be inbetwee 1-4 digits)
 */

#define MAXLEN 100
int main(int argc, char *argv[])
{
	int fd;
	char numstr[MAXLEN];
	int count = 0;
	int num_to_write;
	if( argc != 1) {
		printf("Usage: %s\n", argv[0]);
		exit(1);
	}

	if ( (fd = open("/dev/numpipe", O_WRONLY)) < 0) {
		perror(""); printf("error opening %s\n", argv[1]);
		exit(1);
	}

	while(1) {
		bzero(numstr, MAXLEN);
		sprintf(numstr, "%d%d\n", getpid(), count++);
		num_to_write = atoi(numstr);
		printf("Writing: %d", num_to_write);
		// write to pipe
		printf("******************\n");
		ssize_t ret = write(fd, &num_to_write, sizeof(int));
		if ( ret <= 0) {
			fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
			perror("");
		} else {
			printf("Bytes written: %ld\n", ret);
		}
		printf("***************\n");
		sleep(1);
	}

	close(fd);

	return 0;
}

